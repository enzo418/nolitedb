#pragma once
#include <concepts>
#include <string>
#include <type_traits>

template <class T>
concept StringLike = std::is_convertible_v<T, std::string_view>;