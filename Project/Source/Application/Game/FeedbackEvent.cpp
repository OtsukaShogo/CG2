#include "FeedbackEvent.h"

namespace Shooter {

bool FeedbackQueue::Push(FeedbackType type, const Engine::Vector3& position, const Engine::Vector3& direction) {
	if (events_.size() >= kMaxEventsPerFrame) {
		return false;
	}
	FeedbackEvent event;
	event.type = type;
	event.position = position;
	event.direction = direction;
	events_.push_back(event);
	return true;
}

} // namespace Shooter
