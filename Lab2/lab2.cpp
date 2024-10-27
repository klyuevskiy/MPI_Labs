#include "mpi.h"
#include "../utils/utils.hpp"
#include <cmath>
#include <format>
#include <vector>
#include <ranges>
#include <sstream>
#include <algorithm>

/*
* Задание 2. Вариант 14.
* ln(1 + x) = x / 1 - x^2 / 2 + x^3 / 3 - x^4 / 4 = (-1)^(n+1)x^n / n
* for x in (-1, 1)
*/

double calculate_function(double x, double eps)
{
	double res = 0;
	double cur = x;
	int i = 1;

	while (abs(cur) >= eps)
	{
		res += cur / i;
		cur *= -x;
		i++;
	}

	return res;
}

/* in-place function calculate */
template <typename InpIt>
void calculate_function_by_points(InpIt begin, InpIt end, double eps)
{
	std::for_each(begin, end, [eps](double& x) { x = calculate_function(x, eps); });
}

void master(int process_number)
{
	std::cout << "input start value (A)" << std::endl;
	auto start_value = input_in_range<double>(-1., 1.);

	std::cout << "input end value (B)" << std::endl;
	auto end_value = input_in_range<double>(start_value, 1.);

	std::cout << "input points number (n)" << std::endl;
	auto points_number = input_in_segment<std::uint32_t>(2, 100);

	std::cout << "input epsilon" << std::endl;
	auto eps = input_in_segment<double>(1e-15, 1e-1);

	/* generate points */
	std::vector<double> points(points_number);
	for (std::uint32_t i = 0; i < points_number; i++)
		points[i] = (end_value - start_value) / (points_number - 1) * i + start_value;

	std::cout << "generated points: ";
	std::ranges::copy(points, std::ostream_iterator<double>(std::cout, " "));
	std::cout << std::endl;

	std::uint32_t points_per_process = points_number / process_number;
	std::vector<double> master_points(points_per_process); /* buffer for master points */

	/* send parameters to slaves */
	MPI_Bcast(&eps, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(&points_per_process, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);

	/* send points to slaves */
	MPI_Scatter(points.data(), points_per_process, MPI_DOUBLE,
		master_points.data(), points_per_process, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	/* calculate master points */
	calculate_function_by_points(master_points.begin(), master_points.end(), eps);
	
	/* calculate tail */
	std::vector<double> function_values(points);
	calculate_function_by_points(function_values.begin() + points_per_process * process_number, function_values.end(), eps);

	/* receive values from slaves */
	MPI_Gather(master_points.data(), points_per_process, MPI_DOUBLE,
		function_values.data(), points_per_process, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	/* print table */
	std::cout << "point | calculated value | real value | error" << std::endl;
	for (std::uint32_t i = 0; i < points_number; i++)
	{
		double calculated = function_values[i];
		double real_value = log(1 + points[i]);
		std::cout << std::format("{} | {} | {} | {}",
			points[i],
			calculated,
			real_value,
			abs(calculated - real_value))
			<< std::endl;
	}
}

void slave()
{
	double eps = 0;
	std::uint32_t points_per_process = 0;

	/* receive parameters from master */
	MPI_Bcast(&eps, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(&points_per_process, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);

	/* receive points from master */
	std::vector<double> points(points_per_process);
	MPI_Scatter(NULL, 0, MPI_DOUBLE,
		points.data(), points_per_process, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	/* calculate slave values */
	calculate_function_by_points(points.begin(), points.end(), eps);

	/* send values to master */
	MPI_Gather(points.data(), points_per_process,
		MPI_DOUBLE, NULL, 0, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

int main(int argc, char** argv)
{
	MPI_env env(&argc, &argv);

	if (env.get_rank() == 0)
		master(env.get_process_number());
	else
		slave();
}