#include <type_traits>
#include <cstdint>
#include <iostream>
#include <limits.h>
#include <concepts>
#include <format>

template <typename T>
T input()
	requires std::is_default_constructible_v<T>
{
	T value;
	std::uint16_t attempt_number = 0;

	do
	{
		std::cout << "Input..." << std::endl;
		std::cin >> value;
		if (std::cin.good())
			break;
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cout << "Incorrect input, try again" << std::endl;
	} while (true);
	
	return value;
}

template <typename T, template <typename> typename compare_policy>
T __input_in_range(T left_bound, T right_bound)
	requires requires()
{
	requires std::is_default_constructible_v<T>;
	requires std::is_same_v<bool, decltype(compare_policy<T>::compare_left(left_bound, right_bound))>;
	requires std::is_same_v<bool, decltype(compare_policy<T>::compare_right(left_bound, right_bound))>;
	requires std::is_same_v<const char, decltype(compare_policy<T>::left_bracket)>;
	requires std::is_same_v<const char, decltype(compare_policy<T>::right_bracket)>;
}
{
	T value;
	std::cout << std::format("Input in range {}{}; {}{}",
		compare_policy<T>::left_bracket,
		left_bound, right_bound,
		compare_policy<T>::right_bracket)
		<< std::endl;
	do
	{
		value = input<T>();
		if (compare_policy<T>::compare_left(left_bound, value) && compare_policy<T>::compare_right(value, right_bound))
			break;
		std::cout << std::format("Value out of in range {}{}; {}{}",
			compare_policy<T>::left_bracket,
			left_bound, right_bound,
			compare_policy<T>::right_bracket)
			<< std::endl;
	} while (true);
	return value;
}

template <typename T>
struct __segment_policy
{
	static bool compare_left(T left_bound, T value)
	{
		return left_bound <= value;
	}

	static bool compare_right(T value, T right_bound)
	{
		return value <= right_bound;
	}

	constexpr static char left_bracket = '[';
	constexpr static char right_bracket = ']';
};

template <typename T>
struct __left_range_policy : public __segment_policy<T>
{
	static bool compare_right(T value, T right_bound)
	{
		return value < right_bound;
	}

	constexpr static char right_bracket = ')';
};

template <typename T>
struct __right_range_policy : public __segment_policy<T>
{
	static bool compare_left(T left_bound, T value)
	{
		return left_bound < value;
	}

	constexpr static char left_bracket = '(';
};

template <typename T>
struct __range_policy : public __right_range_policy<T>
{
	static bool compare_right(T value, T right_bound)
	{
		return value < right_bound;
	}

	constexpr static char right_bracket = ')';
};

/* input value in [left_bound; right_bound] */
template <typename T>
T input_in_segment(T left_bound, T right_bound)
	requires std::totally_ordered<T>&& std::is_default_constructible_v<T>
{
	return __input_in_range<T, __segment_policy>(left_bound, right_bound);
}

/* input value in [left_bound; right_bound) */
template <typename T>
T input_in_left_range(T left_bound, T right_bound)
	requires std::totally_ordered<T>&& std::is_default_constructible_v<T>
{
	return __input_in_range<T, __left_range_policy>(left_bound, right_bound);
}

/* input value in (left_bound; right_bound] */
template <typename T>
T input_in_right_range(T left_bound, T right_bound)
	requires std::totally_ordered<T>&& std::is_default_constructible_v<T>
{
	return __input_in_range<T, __right_range_policy>(left_bound, right_bound);
}

/* input value in (left_bound; right_bound) */
template <typename T>
T input_in_range(T left_bound, T right_bound)
	requires std::totally_ordered<T>&& std::is_default_constructible_v<T>
{
	return __input_in_range<T, __range_policy>(left_bound, right_bound);
}
