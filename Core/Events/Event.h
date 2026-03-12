#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>

template <typename T>
class Event {
protected:
	T type;
	std::string name;
	bool isHandled = false;

public:
	Event() = default;
	Event(T type, const std::string& name = "") : type(type), name(name) {}
	virtual ~Event() {}
	inline const T getType() const { return type; }

	template <typename EventType>
	inline EventType toType() const {
		return static_cast<const EventType&>(*this);
	}

	inline const std::string& getName() const { return name; }
	virtual bool handled() const { return isHandled; }
};

template <typename T>
class EventDispatcher {
	using Func = std::function<void(const Event<T>&)>;
	std::map<T, std::vector<Func>> listeners;
public:
	void addListener(T type, const Func& func) {
		listeners[type].push_back(func);
	}

	void sendEvent(const Event<T>& event) {
		if (listeners.find(event.getType()) == listeners.end()) {
			return;
		}

		for (auto&& listener : listeners.at(event.getType())) {
			if (!event.handled()) {
				listener(event);
			}
		}
	}
};