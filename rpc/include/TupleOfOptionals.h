// SPDX-FileCopyrightText: 2024 Contributors to the Eclipse Foundation
//
// See the NOTICE file(s) distributed with this work for additional
// information regarding copyright ownership.
//
// This program and the accompanying materials are made available under the
// terms of the Apache License Version 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <iostream>
#include <optional>
#include <tuple>
#include <type_traits>

namespace tuple_of_optionals {
using namespace std;

//
// Top part of namespace contains functions for hashing.
//

template <typename T, typename = std::void_t<>>
struct is_std_hashable : std::false_type {};

template <typename T>
struct is_std_hashable<
    T, std::void_t<decltype(std::declval<std::hash<T>>()(std::declval<T>()))>>
    : std::true_type {};

template <typename T>
constexpr bool is_std_hashable_v = is_std_hashable<T>::value;

template <typename T>
struct hash;

template <typename T>
struct hash<optional<T>> {
	size_t operator()(const optional<T>& opt) const {
		if (opt.has_value()) {
			return hash<T>{}(*opt);
		} else {
			return typeid(T).hash_code();
		}
	}
};

template <typename... Types>
struct hash<tuple<Types...>> {
	size_t operator()(const tuple<Types...>& t) const { return hash_value(t); }

private:
	static void hash_combine(size_t& seed, size_t value) {
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	template <typename Arg>
	static size_t hash_compute(const Arg& obj) {
		if constexpr (is_std_hashable_v<Arg>)
			return std::hash<Arg>()(obj);
		else
			return hash<Arg>()(obj);
	}

	template <typename Tuple, size_t Index = tuple_size<Tuple>::value - 1>
	struct TupleHash {
		static size_t apply(const Tuple& t) {
			size_t seed = TupleHash<Tuple, Index - 1>::apply(t);
			hash_combine(seed, hash_compute(get<Index>(t)));
			return seed;
		}
	};

	template <typename Tuple>
	struct TupleHash<Tuple, 0> {
		static size_t apply(const Tuple& t) { return hash_compute(get<0>(t)); }
	};

	template <typename Tuple>
	static size_t hash_value(const Tuple& t) {
		return TupleHash<Tuple>::apply(t);
	}
};

//
// These two assist in generating the optionals expansion, but need no outside
// exposure.
//
template <typename T>
void assign_if(T& field, size_t& cnt, size_t i, size_t bits) {}

template <typename T>
void assign_if(optional<T>& field, size_t& cnt, size_t i, size_t bits) {
	if (((1 << i) & bits) && (field != nullopt)) {
		field = nullopt;
		cnt++;
	}
}

//
// Bottom part of namespace contains functions for printing.
//
template <typename T>
ostream& operator<<(ostream& os, const optional<T>& t) {
	if (t)
		os << *t;
	else
		os << "nullopt";
	return os;
}

template <typename Tuple, size_t Index>
ostream& print_tuple_element(ostream& os, const Tuple& t) {
	if constexpr (Index < tuple_size<Tuple>::value - 1) {
		return os << get<Index>(t) << ", ";
	} else {
		return os << get<Index>(t);
	}
}

template <typename Tuple, size_t... Indices>
ostream& print_tuple(ostream& os, const Tuple& t, index_sequence<Indices...>) {
	((print_tuple_element<Tuple, Indices>(os, t)), ...);
	return os;
}

}  // namespace tuple_of_optionals

template <auto Start, auto End, auto Inc, class F>
constexpr void constexpr_for(F&& f) {
	if constexpr (Start < End) {
		f(std::integral_constant<decltype(Start), Start>());
		constexpr_for<Start + Inc, End, Inc>(f);
	}
}

//
// This function is going to take a tuple key, and return a vector of
// alternatives with wildcard substitutions.
//
template <typename T>
std::vector<T> generateOptionals(const T& key) {
	const auto len = std::tuple_size_v<T>;
	std::vector<T> ret;
	ret.push_back(key);
	constexpr_for<1, (1 << len), 1>([&](const auto bits) {
		auto out = key;
		size_t cnt = 0;
		constexpr_for<0, len, 1>([&](const auto i) {
			tuple_of_optionals::assign_if(std::get<i>(out), cnt, i, bits);
		});
		if (cnt > 0) {
			ret.push_back(out);
		}
	});
	return ret;
}

template <typename... input_t>
using tuple_cat_t = decltype(std::tuple_cat(std::declval<input_t>()...));

template <typename... Types>
std::ostream& operator<<(std::ostream& os, const std::tuple<Types...>& t) {
	os << "(";
	tuple_of_optionals::print_tuple(os, t, std::index_sequence_for<Types...>{});
	return os << ")";
}

template <typename... Types>
std::string to_string(const std::tuple<Types...>& t)
{
	std::stringstream ss;
	ss << "(";
	tuple_of_optionals::print_tuple(ss, t, std::index_sequence_for<Types...>{});
	ss << ")";
	return ss.str();
}