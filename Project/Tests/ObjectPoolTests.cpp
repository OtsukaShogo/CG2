#include "TestFramework.h"
#include "Source/Engine/Utility/ObjectPool.h"

#include <cstdint>
#include <memory>
#include <set>
#include <vector>

using namespace Engine;

namespace {

struct Bullet {
	int id = 0;
	float life = 0.0f;
};

// 作成・破棄の回数を数える型（プールが、取り出し・返却のたびに作り直していないことの確認用）
struct Counted {
	static inline int constructed = 0;
	static inline int destroyed = 0;
	std::unique_ptr<int> payload; // ムーブしかできない型
	int uses = 0;

	explicit Counted(int value) : payload(std::make_unique<int>(value)) { ++constructed; }
	Counted(Counted&& other) noexcept : payload(std::move(other.payload)), uses(other.uses) {}
	~Counted() { if (payload) ++destroyed; } // 中身を持つ本体の破棄だけを数える（ムーブ元は数えない）
};

} // namespace

TEST_CASE(ObjectPool_AcquireUntilFull) {
	ObjectPool<Bullet> pool(3);
	CHECK(pool.GetCapacity() == 3);
	CHECK(pool.GetActiveCount() == 0);
	CHECK(!pool.IsFull());

	Bullet* a = pool.Acquire();
	Bullet* b = pool.Acquire();
	Bullet* c = pool.Acquire();
	CHECK(a != nullptr);
	CHECK(b != nullptr);
	CHECK(c != nullptr);
	CHECK(a != b);
	CHECK(b != c);
	CHECK(a != c);
	CHECK(pool.GetActiveCount() == 3);
	CHECK(pool.IsFull());
	CHECK(pool.Acquire() == nullptr); // 満杯
	CHECK(pool.GetActiveCount() == 3);
}

TEST_CASE(ObjectPool_ReleaseAndReuse) {
	ObjectPool<Bullet> pool(2);
	Bullet* a = pool.Acquire();
	Bullet* b = pool.Acquire();
	a->id = 11;
	b->id = 22;

	CHECK(pool.Release(a));
	CHECK(pool.GetActiveCount() == 1);
	CHECK(!pool.IsFull());
	CHECK(!pool.IsActive(a));
	CHECK(pool.IsActive(b));

	// 返却したものが再利用され、アドレスは変わらない。前回の状態が残っている（初期化は呼び出し側の責任）
	Bullet* again = pool.Acquire();
	CHECK(again == a);
	CHECK(again->id == 11);
	CHECK(pool.IsFull());
}

TEST_CASE(ObjectPool_ReleaseIsSafeAgainstMistakes) {
	ObjectPool<Bullet> pool(2);
	ObjectPool<Bullet> other(2);
	Bullet* a = pool.Acquire();
	Bullet* foreign = other.Acquire();

	CHECK(pool.Release(a));
	CHECK(!pool.Release(a));                 // 二重返却
	CHECK(pool.GetActiveCount() == 0);
	CHECK(!pool.Release(foreign));           // 別のプールのもの
	CHECK(other.IsActive(foreign));          // 別のプールには影響しない
	CHECK(!pool.Release(static_cast<const Bullet*>(nullptr)));
	CHECK(!pool.Release(reinterpret_cast<Bullet*>(reinterpret_cast<char*>(a) + 1))); // 要素の途中を指している
	Bullet onStack;
	CHECK(!pool.Release(&onStack));          // プールの外のオブジェクト
	CHECK(!pool.IsActive(&onStack));
	CHECK(pool.GetActiveCount() == 0);

	// 返却の失敗が、空きの数を壊していない（満杯までちょうど2つ取り出せる）
	CHECK(pool.Acquire() != nullptr);
	CHECK(pool.Acquire() != nullptr);
	CHECK(pool.Acquire() == nullptr);
}

TEST_CASE(ObjectPool_Handles) {
	ObjectPool<Bullet> pool(2);
	Bullet* a = pool.Acquire();
	a->id = 5;
	PoolHandle handle = pool.GetHandle(a);
	CHECK(pool.Get(handle) == a);

	// 返却すると、ハンドルは無効になる
	CHECK(pool.Release(handle));
	CHECK(pool.Get(handle) == nullptr);
	CHECK(!pool.Release(handle));
	CHECK(pool.GetHandle(a) == PoolHandle{}); // 使用中でないものは、無効なハンドル

	// 同じスロットが再利用されても、古いハンドルは、新しいオブジェクトを指さない
	Bullet* reused = pool.Acquire();
	CHECK(reused == a);
	CHECK(pool.Get(handle) == nullptr);
	PoolHandle fresh = pool.GetHandle(reused);
	CHECK(pool.Get(fresh) == reused);
	CHECK(fresh != handle);

	// 無効なハンドル・範囲外のハンドル
	CHECK(pool.Get(PoolHandle{}) == nullptr);
	CHECK(pool.Get(PoolHandle{ 99, 0 }) == nullptr);

	// 全返却で、すべてのハンドルが無効になる
	const ObjectPool<Bullet>& constPool = pool;
	pool.ReleaseAll();
	CHECK(pool.Get(fresh) == nullptr);
	CHECK(constPool.Get(fresh) == nullptr);
	CHECK(pool.GetActiveCount() == 0);
}

TEST_CASE(ObjectPool_RangeForVisitsOnlyActive) {
	ObjectPool<Bullet> pool(8);
	std::vector<Bullet*> acquired;
	for (int i = 0; i < 6; ++i) {
		Bullet* b = pool.Acquire();
		b->id = i;
		acquired.push_back(b);
	}
	pool.Release(acquired[1]);
	pool.Release(acquired[4]);

	std::set<int> seen;
	int visits = 0;
	for (Bullet& b : pool) {
		seen.insert(b.id);
		++visits;
	}
	CHECK(visits == 4);
	CHECK(seen == std::set<int>({ 0, 2, 3, 5 }));

	// const の走査
	const ObjectPool<Bullet>& constPool = pool;
	int constVisits = 0;
	for (const Bullet& b : constPool) {
		(void)b;
		++constVisits;
	}
	CHECK(constVisits == 4);

	// 何も使っていなければ、空
	pool.ReleaseAll();
	int none = 0;
	for (Bullet& b : pool) { (void)b; ++none; }
	CHECK(none == 0);
}

TEST_CASE(ObjectPool_ReleaseDuringIteration) {
	ObjectPool<Bullet> pool(10);
	for (int i = 0; i < 10; ++i) {
		pool.Acquire()->id = i;
	}

	// 走査中に、自分自身を返却（寿命の尽きた弾）
	int visits = 0;
	for (Bullet& b : pool) {
		++visits;
		if (b.id % 2 == 0) {
			CHECK(pool.Release(&b));
		}
	}
	CHECK(visits == 10);               // 全員を、ちょうど1回ずつ訪れる
	CHECK(pool.GetActiveCount() == 5);

	// 走査中に、まだ訪れていない他のオブジェクトを返却（命中した敵を消す）。消されたものは訪れない
	std::vector<int> order;
	for (Bullet& b : pool) {
		order.push_back(b.id);
		if (b.id == 1) {
			for (Bullet& other : pool) {
				if (other.id == 5) pool.Release(&other);
			}
		}
	}
	CHECK(order == std::vector<int>({ 1, 3, 7, 9 }));

	// 最後の要素まで返却しても、終端に届く（無限ループやはみ出しをしない）
	int count = 0;
	for (Bullet& b : pool) {
		++count;
		pool.Release(&b);
	}
	CHECK(count == 4);
	CHECK(pool.GetActiveCount() == 0);

	// 走査中の取り出し（弾が弾を生む）。この走査で訪れるかは規定しないが、壊れず、全員が使用中として数えられる
	for (int i = 0; i < 3; ++i) pool.Acquire()->id = 100 + i;
	int spawned = 0;
	for (Bullet& b : pool) {
		if (b.id >= 100 && spawned < 3) {
			if (Bullet* child = pool.Acquire()) { child->id = 200 + spawned; ++spawned; }
		}
	}
	CHECK(pool.GetActiveCount() == 6);
}

TEST_CASE(ObjectPool_ForEachAndReleaseIf) {
	ObjectPool<Bullet> pool(6);
	for (int i = 0; i < 6; ++i) {
		Bullet* b = pool.Acquire();
		b->id = i;
		b->life = static_cast<float>(i);
	}
	int sum = 0;
	pool.ForEach([&](Bullet& b) { sum += b.id; });
	CHECK(sum == 15);

	// 寿命が2以下のものを返却
	size_t released = pool.ReleaseIf([](const Bullet& b) { return b.life <= 2.0f; });
	CHECK(released == 3);
	CHECK(pool.GetActiveCount() == 3);
	int remaining = 0;
	pool.ForEach([&](Bullet& b) { remaining += b.id; });
	CHECK(remaining == 3 + 4 + 5);

	// ForEach の中で返却してもよい
	pool.ForEach([&](Bullet& b) { if (b.id == 4) pool.Release(&b); });
	CHECK(pool.GetActiveCount() == 2);
	CHECK(pool.ReleaseIf([](const Bullet&) { return false; }) == 0);
}

TEST_CASE(ObjectPool_AcquireOrRecycleOldest) {
	ObjectPool<Bullet> pool(3);
	Bullet* a = pool.Acquire();
	Bullet* b = pool.Acquire();
	Bullet* c = pool.Acquire();
	a->id = 1; b->id = 2; c->id = 3;

	// aを返却して取り直す: スロットの番号は最も小さいが、取り出した順では最も新しい
	CHECK(pool.Release(a));
	Bullet* a2 = pool.Acquire();
	CHECK(a2 == a);
	a2->id = 4;
	PoolHandle oldestHandle = pool.GetHandle(b);

	// 満杯: 最も古く取り出された b が再利用される
	Bullet* recycled = pool.AcquireOrRecycleOldest();
	CHECK(recycled == b);
	CHECK(pool.GetActiveCount() == 3);
	CHECK(pool.Get(oldestHandle) == nullptr); // 古いハンドルは無効
	recycled->id = 5;
	// 次に古いのは c
	CHECK(pool.AcquireOrRecycleOldest() == c);

	// 空きがあるときは、通常の取り出しと同じ（何も返却しない）
	ObjectPool<Bullet> roomy(3);
	Bullet* only = roomy.Acquire();
	Bullet* second = roomy.AcquireOrRecycleOldest();
	CHECK(second != only);
	CHECK(roomy.GetActiveCount() == 2);

	// 容量0
	ObjectPool<Bullet> empty(0);
	CHECK(empty.AcquireOrRecycleOldest() == nullptr);
	CHECK(empty.Acquire() == nullptr);
	CHECK(empty.IsFull());
}

TEST_CASE(ObjectPool_FactoryAndNoRecreation) {
	Counted::constructed = 0;
	Counted::destroyed = 0;
	{
		std::vector<size_t> indices;
		ObjectPool<Counted> pool(4, [&](size_t i) {
			indices.push_back(i);
			return Counted(static_cast<int>(i) * 10);
		});
		CHECK(indices == std::vector<size_t>({ 0, 1, 2, 3 })); // 容量ぶん、番号の順に作る
		CHECK(Counted::constructed == 4);

		// 取り出し・返却を何度繰り返しても、作成も破棄も起きない（モデルなどを持たせても、出現のたびに作り直さない）
		for (int round = 0; round < 100; ++round) {
			Counted* a = pool.Acquire();
			Counted* b = pool.Acquire();
			a->uses++;
			b->uses++;
			pool.Release(a);
			pool.Release(b);
		}
		CHECK(Counted::constructed == 4);
		CHECK(Counted::destroyed == 0);

		// 持たせたもの（payload）は、返却しても保持される
		Counted* c = pool.Acquire();
		CHECK(c->payload != nullptr);
		CHECK(c->uses > 0);
	}
	CHECK(Counted::destroyed == 4); // プールが消えるときに、すべて破棄される
}

TEST_CASE(ObjectPool_AddressesAreStable) {
	ObjectPool<Bullet> pool(100);
	std::vector<Bullet*> pointers;
	for (int i = 0; i < 100; ++i) {
		pointers.push_back(pool.Acquire());
	}
	// 取り出しや返却を繰り返しても、アドレスは変わらない（使用中に別の場所へ動くことはない）
	std::set<Bullet*> all(pointers.begin(), pointers.end());
	CHECK(all.size() == 100);
	for (int round = 0; round < 10; ++round) {
		for (Bullet* b : pointers) pool.Release(b);
		for (int i = 0; i < 100; ++i) {
			Bullet* b = pool.Acquire();
			CHECK(all.count(b) == 1);
		}
	}
}

TEST_CASE(ObjectPool_RandomOperationsMatchModel) {
	// 乱数による取り出し・返却を、単純なモデル（使用中の集合）と照合する
	ObjectPool<Bullet> pool(64);
	std::set<Bullet*> model;
	uint32_t state = 12345;
	auto next = [&]() { state = state * 1664525u + 1013904223u; return (state >> 8) & 0xFFFF; };

	bool ok = true;
	for (int step = 0; step < 20000; ++step) {
		uint32_t action = next() % 4;
		if (action <= 1) {
			Bullet* b = pool.Acquire();
			if (model.size() == 64) {
				ok = ok && b == nullptr;
			} else {
				ok = ok && b != nullptr && model.count(b) == 0; // 使用中のものを、重ねて渡さない
				if (b) model.insert(b);
			}
		} else if (action == 2 && !model.empty()) {
			auto it = model.begin();
			std::advance(it, next() % model.size());
			ok = ok && pool.Release(*it);
			model.erase(it);
		} else if (action == 3 && next() % 50 == 0) {
			pool.ReleaseAll();
			model.clear();
		}
		ok = ok && pool.GetActiveCount() == model.size();
		if (step % 500 == 0) {
			// 走査が、使用中の集合とちょうど一致する
			std::set<Bullet*> seen;
			for (Bullet& b : pool) seen.insert(&b);
			ok = ok && seen == model;
		}
	}
	CHECK(ok);
}
