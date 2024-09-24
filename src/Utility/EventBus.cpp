
#include "EventBus.h"

#include <mutex>

using namespace common_dev::event_system;

static std::size_t s_next_id = 0;

struct Callback {
	std::function<void(std::any const&)> func;
	std::size_t const id = s_next_id++;
	bool once;
};

static std::unordered_map<std::type_index, std::list<Callback>> s_subscriptions;
static std::mutex s_mutex;

CallbackID common_dev::event_system::Subscribe(std::type_index const& type_index, std::function<void(std::any const&)> const& func, bool once) {

	std::lock_guard<std::mutex> lock(s_mutex);
	if (s_subscriptions.find(type_index) == s_subscriptions.end()) {
		s_subscriptions.emplace(type_index, std::list<Callback>());
	}
	Callback callback = { .func = func,.once = once };
	s_subscriptions[type_index].emplace_back(callback);
	return callback.id;

}

void common_dev::event_system::Unsubscribe(std::type_index const& type_index, CallbackID id) {

	std::lock_guard<std::mutex> lock(s_mutex);
	if (s_subscriptions.find(type_index) != s_subscriptions.end()) {

		auto& callbacks = s_subscriptions[type_index];
		callbacks.remove_if([&id](Callback const& callback) {return callback.id == id; });
		if (callbacks.empty()) {
			s_subscriptions.erase(type_index);
		}
	}

}

void common_dev::event_system::Publish(std::type_index const& type_index, std::any const& arg) {

	std::lock_guard<std::mutex> lock(s_mutex);
	if (s_subscriptions.find(type_index) != s_subscriptions.end()) {
		auto& callbacks = s_subscriptions[type_index];
		auto iter = callbacks.begin();
		while (iter != callbacks.end()) {
			iter->func(arg);
			if (iter->once) {
				iter = callbacks.erase(iter);
			}
			else {
				iter++;
			}
		}
		if (callbacks.empty()) {
			s_subscriptions.erase(type_index);
		}
	}

}

void common_dev::event_system::ClearSubscriptions() {

	std::lock_guard<std::mutex> lock(s_mutex);
	s_subscriptions.clear();

}
