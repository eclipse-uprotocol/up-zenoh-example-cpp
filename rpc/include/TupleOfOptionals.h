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

template <typename... Types>
std::ostream& operator<<(std::ostream& os, const std::tuple<Types...>& t) {
	os << "(";
	tuple_of_optionals::print_tuple(os, t, std::index_sequence_for<Types...>{});
	return os << ")";
}
