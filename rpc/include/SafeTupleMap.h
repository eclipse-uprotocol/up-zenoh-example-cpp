#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "TupleOfOptionals.h"

template <typename KEY, typename VALUE>
class SafeTupleMap {
	std::unordered_map<KEY, std::shared_ptr<VALUE>,
	                   tuple_of_optionals::hash<KEY>>
	    map_;
	std::mutex mtx;

public:
	using Key = KEY;

	SafeTupleMap() = default;

	std::shared_ptr<VALUE> find(const KEY& key, bool create = false) {
		std::unique_lock<std::mutex> lock(mtx);
		auto it = map_.find(key);
		if (!create) {
			return (it != map_.end()) ? it->second : nullptr;
		} else {
			if (it != map_.end())
				return it->second;
			auto ptr = std::make_shared<VALUE>();
			map_.emplace(key, ptr);
			return ptr;
		}
	}

	void erase(std::function<void(std::shared_ptr<VALUE>)> fn) {
		std::unique_lock<std::mutex> lock(mtx);
		for (auto [key, ptr] : map_) {
			fn(ptr);
		}
	}
};
