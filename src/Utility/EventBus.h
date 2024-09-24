#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include <any>
#include <chrono>
#include <functional>
#include <typeindex>

namespace common_dev::event_system {

	class BaseEvent {

	public:

		virtual ~BaseEvent() noexcept = default;

	protected:

		BaseEvent() = default;

		std::chrono::system_clock::time_point const m_timestamp = std::chrono::system_clock::now();

	};

}

namespace common_dev::event_system {

	using CallbackID = std::size_t;

	CallbackID Subscribe(std::type_index const& type_index, std::function<void(std::any const&)> const& func, bool once = false);
	void Unsubscribe(std::type_index const& type_index, CallbackID id);
	void Publish(std::type_index const& type_index, std::any const& arg);
	void ClearSubscriptions();

	template <class T> requires std::derived_from<T, BaseEvent>
	CallbackID Subscribe(std::function<void(T&)> const&& func, bool once = false) {
		return Subscribe(
			typeid(T),
			[func](std::any const& arg) {
				auto _arg = std::any_cast<T>(arg);
				func(_arg);
			},
			once
		);
	}

	template <class T> requires std::derived_from<T, BaseEvent>
	void Unsubscribe(CallbackID id) {
		Unsubscribe(typeid(T), id);
	}

	template <class T> requires std::derived_from<T, BaseEvent>
	void Publish(T&& e) {
		Publish(typeid(e), e);
	}


}

#endif // !EVENT_SYSTEM_H
