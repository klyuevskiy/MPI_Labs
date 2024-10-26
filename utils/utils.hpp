#include <type_traits>
#include <cstdint>
#include <iostream>
#include <limits.h>
#include <concepts>

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

template <typename T>
T input_in_range(T left_bound, T right_bound)
	requires std::totally_ordered<T> && std::is_default_constructible_v<T>
{
	T value;
	std::cout << std::format("Input in range [{}; {}]", left_bound, right_bound) << std::endl;
	do
	{
		value = input<T>();
		if (left_bound <= value && value <= right_bound)
			break;
		std::cout << std::format("Value out of range [{}; {}]", left_bound, right_bound) << std::endl;
	} while (true);
	return value;
}