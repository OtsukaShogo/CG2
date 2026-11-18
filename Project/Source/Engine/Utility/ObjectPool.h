#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <utility>
#include <vector>

namespace Engine {

/// <summary>
/// プール内のオブジェクトを指す、安全な参照。
/// 生ポインタと違い、指していたオブジェクトが返却（再利用）されると、無効になったことを検出できる。
/// 「敵が追いかけている相手」のように、相手が先に消える可能性がある参照に使う
/// </summary>
struct PoolHandle {
	uint32_t index = (std::numeric_limits<uint32_t>::max)();
	uint32_t generation = 0;

	[[nodiscard]] bool operator==(const PoolHandle& other) const { return index == other.index && generation == other.generation; }
	[[nodiscard]] bool operator!=(const PoolHandle& other) const { return !(*this == other); }
};

/// <summary>
/// 固定容量のオブジェクトプール。弾や敵のように、頻繁に出たり消えたりするオブジェクトを、
/// 生成・破棄せずに使い回すためのもの。
///
/// - 容量ぶんのオブジェクトを、最初に1度だけ作る。Acquire/Release では、生成も破棄も行わない（メモリ確保なし）。
///   モデルなど、作るのに時間がかかるものを持たせても、出現のたびに作り直さずに済む。
/// - オブジェクトのアドレスは、プールが生きている間、変わらない。
/// - Acquire で渡されるオブジェクトは、前回の使用時の状態が残っている。呼び出し側で、必ず初期化すること。
/// - スレッドセーフではない。
///
/// 使い方:
///   ObjectPool<Bullet> bullets(256);
///   if (Bullet* b = bullets.Acquire()) { b->position = ...; }    // 満杯なら nullptr
///   for (Bullet& b : bullets) { ...; if (b.life <= 0) bullets.Release(&b); } // 走査中に返却してよい
/// </summary>
template <typename T>
class ObjectPool {
public:
	/// <summary>
	/// 容量ぶんのオブジェクトを、既定のコンストラクタで作る（T は既定のコンストラクタで作れること）
	/// </summary>
	explicit ObjectPool(size_t capacity) : items_(capacity) {
		InitializeSlots();
	}

	/// <summary>
	/// 容量ぶんのオブジェクトを、factory(スロット番号) の戻り値で作る。
	/// 既定のコンストラクタがない型や、モデルなどを持たせる型に使う（T はムーブできること）
	/// </summary>
	ObjectPool(size_t capacity, const std::function<T(size_t)>& factory) {
		items_.reserve(capacity);
		for (size_t i = 0; i < capacity; ++i) {
			items_.push_back(factory(i));
		}
		InitializeSlots();
	}

	ObjectPool(const ObjectPool&) = delete;
	ObjectPool& operator=(const ObjectPool&) = delete;

	/// <summary>
	/// 空いているオブジェクトを1つ取り出す。満杯なら nullptr。
	/// 前回の使用時の状態が残っているので、呼び出し側で初期化すること
	/// </summary>
	[[nodiscard]] T* Acquire() {
		if (freeSlots_.empty()) {
			return nullptr;
		}
		const uint32_t slot = freeSlots_.back();
		freeSlots_.pop_back();
		alive_[slot] = 1;
		serial_[slot] = ++nextSerial_;
		++activeCount_;
		if (slot >= scanEnd_) {
			scanEnd_ = slot + 1;
		}
		return &items_[slot];
	}

	/// <summary>
	/// 空いているオブジェクトを1つ取り出す。満杯の場合は、最も古く取り出されたものを返却して、再利用する。
	/// 弾が多すぎるときに、古い弾から消したい場合に使う（容量が0のときだけ nullptr）
	/// </summary>
	[[nodiscard]] T* AcquireOrRecycleOldest() {
		if (items_.empty()) {
			return nullptr;
		}
		if (freeSlots_.empty()) {
			uint32_t oldest = 0;
			uint64_t oldestSerial = (std::numeric_limits<uint64_t>::max)();
			for (uint32_t slot = 0; slot < scanEnd_; ++slot) {
				if (alive_[slot] && serial_[slot] < oldestSerial) {
					oldestSerial = serial_[slot];
					oldest = slot;
				}
			}
			ReleaseSlot(oldest);
		}
		return Acquire();
	}

	/// <summary>
	/// オブジェクトを返却する。このプールのもので、使用中なら true。
	/// すでに返却済み・このプールのものではない・nullptr の場合は、何もせず false（二重返却しても安全）。
	/// 走査（範囲for・ForEach）の最中に呼んでもよい
	/// </summary>
	bool Release(const T* object) {
		const uint32_t slot = FindSlot(object);
		if (slot == kInvalidSlot || !alive_[slot]) {
			return false;
		}
		ReleaseSlot(slot);
		return true;
	}

	/// <summary>
	/// ハンドルが指すオブジェクトを返却する。ハンドルが無効（すでに返却された）なら false
	/// </summary>
	bool Release(const PoolHandle& handle) {
		T* object = Get(handle);
		return object != nullptr && Release(object);
	}

	/// <summary>
	/// 使用中のオブジェクトをすべて返却する（シーンの切り替え時など）。以前のハンドルは、すべて無効になる
	/// </summary>
	void ReleaseAll() {
		for (uint32_t slot = 0; slot < scanEnd_; ++slot) {
			if (alive_[slot]) {
				ReleaseSlot(slot);
			}
		}
	}

	/// <summary>
	/// 条件を満たす使用中のオブジェクトを、すべて返却する。返却した数を返す。
	/// 例: bullets.ReleaseIf([](const Bullet& b) { return b.life <= 0.0f; });
	/// </summary>
	template <typename Predicate>
	size_t ReleaseIf(Predicate&& shouldRelease) {
		size_t released = 0;
		for (uint32_t slot = 0; slot < scanEnd_; ++slot) {
			if (alive_[slot] && shouldRelease(items_[slot])) {
				ReleaseSlot(slot);
				++released;
			}
		}
		return released;
	}

	/// <summary>
	/// オブジェクトを指すハンドルを作る（使用中でなければ、無効なハンドル）
	/// </summary>
	[[nodiscard]] PoolHandle GetHandle(const T* object) const {
		const uint32_t slot = FindSlot(object);
		if (slot == kInvalidSlot || !alive_[slot]) {
			return PoolHandle{};
		}
		return PoolHandle{ slot, generation_[slot] };
	}

	/// <summary>
	/// ハンドルが指すオブジェクトを返す。すでに返却された（再利用された）ものや、無効なハンドルなら nullptr
	/// </summary>
	[[nodiscard]] T* Get(const PoolHandle& handle) {
		if (handle.index >= items_.size() || !alive_[handle.index] || generation_[handle.index] != handle.generation) {
			return nullptr;
		}
		return &items_[handle.index];
	}
	[[nodiscard]] const T* Get(const PoolHandle& handle) const {
		return const_cast<ObjectPool*>(this)->Get(handle);
	}

	/// <summary>
	/// オブジェクトが、このプールで使用中か
	/// </summary>
	[[nodiscard]] bool IsActive(const T* object) const {
		const uint32_t slot = FindSlot(object);
		return slot != kInvalidSlot && alive_[slot] != 0;
	}

	/// <summary>
	/// 使用中のオブジェクトの数
	/// </summary>
	[[nodiscard]] size_t GetActiveCount() const { return activeCount_; }

	/// <summary>
	/// 容量（同時に使える最大数）
	/// </summary>
	[[nodiscard]] size_t GetCapacity() const { return items_.size(); }

	/// <summary>
	/// 満杯か（Acquire が nullptr を返す状態か）
	/// </summary>
	[[nodiscard]] bool IsFull() const { return freeSlots_.empty(); }

	/// <summary>
	/// 使用中のオブジェクトを、順に呼び出す（スロット番号の順。取り出した順とは限らない）。
	/// 呼び出し中に、そのオブジェクトや他のオブジェクトを返却してもよい。
	/// 呼び出し中に取り出したオブジェクトは、番号によっては、同じ走査で呼ばれることがある
	/// </summary>
	template <typename Function>
	void ForEach(Function&& function) {
		for (uint32_t slot = 0; slot < scanEnd_; ++slot) {
			if (alive_[slot]) {
				function(items_[slot]);
			}
		}
	}

	/// <summary>
	/// 使用中のオブジェクトだけを走査する範囲for用のイテレータ
	/// </summary>
	template <typename Pool, typename Value>
	class IteratorBase {
	public:
		IteratorBase(Pool* pool, uint32_t slot) : pool_(pool), slot_(slot) { SkipInactive(); }
		Value& operator*() const { return pool_->items_[slot_]; }
		Value* operator->() const { return &pool_->items_[slot_]; }
		IteratorBase& operator++() { ++slot_; SkipInactive(); return *this; }
		bool operator==(const IteratorBase& other) const { return slot_ == other.slot_; }
		bool operator!=(const IteratorBase& other) const { return slot_ != other.slot_; }

	private:
		void SkipInactive() {
			// 使用中のスロットまで進む。末尾まで進んだら「終わり」の印にする。
			// 走査の最中に返却されて scanEnd_ が縮んでも、終わりの印どうしは等しいままなので、終端に届く
			while (slot_ < pool_->scanEnd_ && !pool_->alive_[slot_]) {
				++slot_;
			}
			if (slot_ >= pool_->scanEnd_) {
				slot_ = kInvalidSlot;
			}
		}

		Pool* pool_;
		uint32_t slot_;
	};
	using Iterator = IteratorBase<ObjectPool, T>;
	using ConstIterator = IteratorBase<const ObjectPool, const T>;

	[[nodiscard]] Iterator begin() { return Iterator(this, 0); }
	[[nodiscard]] Iterator end() { return Iterator(this, kInvalidSlot); }
	[[nodiscard]] ConstIterator begin() const { return ConstIterator(this, 0); }
	[[nodiscard]] ConstIterator end() const { return ConstIterator(this, kInvalidSlot); }

private:
	static constexpr uint32_t kInvalidSlot = (std::numeric_limits<uint32_t>::max)();

	void InitializeSlots() {
		const size_t capacity = items_.size();
		assert(capacity < kInvalidSlot);
		alive_.assign(capacity, 0);
		generation_.assign(capacity, 0);
		serial_.assign(capacity, 0);
		// 空きスロットは、番号の小さいものから使われるよう、逆順に積む（走査の範囲を小さく保つため）
		freeSlots_.reserve(capacity);
		for (size_t i = capacity; i > 0; --i) {
			freeSlots_.push_back(static_cast<uint32_t>(i - 1));
		}
	}

	// ポインタが指すスロット番号を求める（このプールのものでなければ kInvalidSlot）
	[[nodiscard]] uint32_t FindSlot(const T* object) const {
		if (object == nullptr || items_.empty()) {
			return kInvalidSlot;
		}
		const T* first = items_.data();
		const T* last = first + items_.size();
		// 別の配列とのポインタ比較は未規定なので、整数にして比べる
		const uintptr_t address = reinterpret_cast<uintptr_t>(object);
		if (address < reinterpret_cast<uintptr_t>(first) || address >= reinterpret_cast<uintptr_t>(last)) {
			return kInvalidSlot;
		}
		if ((address - reinterpret_cast<uintptr_t>(first)) % sizeof(T) != 0) {
			return kInvalidSlot; // 要素の途中を指している
		}
		return static_cast<uint32_t>(object - first);
	}

	void ReleaseSlot(uint32_t slot) {
		alive_[slot] = 0;
		++generation_[slot]; // この世代のハンドルは、すべて無効になる
		freeSlots_.push_back(slot);
		--activeCount_;
		// 走査範囲の末尾が空いたら、使用中の最後の番号まで縮める
		while (scanEnd_ > 0 && !alive_[scanEnd_ - 1]) {
			--scanEnd_;
		}
	}

private:
	std::vector<T> items_;               // 容量ぶん。作成後は、大きさもアドレスも変えない
	std::vector<uint8_t> alive_;         // 使用中か
	std::vector<uint32_t> generation_;   // スロットごとの世代（返却のたびに増える）
	std::vector<uint64_t> serial_;       // 取り出した順番（古いものを探すため）
	std::vector<uint32_t> freeSlots_;    // 空きスロットの積み重ね（後ろから取り出す）
	size_t activeCount_ = 0;
	uint32_t scanEnd_ = 0;               // 使用中のスロットの最大番号+1。走査の範囲
	uint64_t nextSerial_ = 0;
};

} // namespace Engine
