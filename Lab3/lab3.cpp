#include "mpi.h"
#include "../utils/utils.hpp"
#include <cmath>
#include <format>
#include <vector>
#include <ranges>
#include <sstream>
#include <algorithm>
#include <random>

/*
* Задание 3. Вариант 12.
* Найти максимальный отрицательный (0 если нет отрицательных)
*/

constexpr int MIN_VALUE = -100;
constexpr int MAX_VALUE = 100;

std::vector<int> generate_numbers(int rank, int numbers_count, int min_value, int max_value)
{
	std::vector<int> result;
	std::mt19937 generator(rank * time(0));
	std::uniform_int_distribution<int> distribution(min_value, max_value);
	std::generate_n(std::back_inserter(result), numbers_count, [&]() {
		return distribution(generator);
		});
	return result;
}

void find_max_negative(void *in_vec, void *inout_vec, int *len, MPI_Datatype *dtype)
{
	int *in = reinterpret_cast<int*>(in_vec);
	int *inout = reinterpret_cast<int*>(inout_vec);
	for (int i = 0; i < *len; i++)
	{
		int a = inout[i];
		int b = in[i];
		if (a < 0 && b < 0)
			inout[i] = std::max(a, b);
		else
			inout[i] = std::min(a, b);
	}
}

int main(int argc, char** argv)
{
	MPI_env env(&argc, &argv);

	int rank = env.get_rank(), process_number = env.get_process_number();

	int number_count = 0, min_number = 0, max_number = 0;

	/* in master input parameters */
	if (rank == 0)
	{
		std::cout << "input elements count in vector (n)" << std::endl;
		number_count = input_in_range<int>(1, 100);
		
		std::cout << "input min number for generation" << std::endl;
		min_number = input_in_segment<int>(MIN_VALUE, MAX_VALUE);

		std::cout << "input max number for generation" << std::endl;
		max_number = input_in_right_range<int>(min_number, MAX_VALUE + min_number);
	}

	/* send parameters to other processes */
	MPI_Bcast(&number_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&min_number, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&max_number, 1, MPI_INT, 0, MPI_COMM_WORLD);

	auto vec = generate_numbers(rank, number_count, min_number, max_number);
	
	/* print number for everyone process */
	std::ostringstream ostream;
	ostream << std::format("process {}: ", rank);
	std::ranges::copy(vec, std::ostream_iterator<int>(ostream, " "));
	std::cout << ostream.str() << std::endl;
	
	std::vector<int> result;
	if (rank == 0) /* need to result in master only */
		result = std::vector<int>(number_count);

	/* define MPI user operation */
	MPI_Op find_max_negative_op;
	MPI_Op_create(find_max_negative, true, &find_max_negative_op);

	MPI_Reduce(vec.data(), result.data(), number_count,
		MPI_INT, find_max_negative_op, 0, MPI_COMM_WORLD);

	MPI_Op_free(&find_max_negative_op);

	/* check for positive number in result */
	std::ranges::for_each(result, [](auto& v) { v = std::min(v, 0); });

	if (rank == 0)
	{
		std::cout << "result: " << std::endl;
		std::ranges::copy(result, std::ostream_iterator<int>(std::cout, " "));
		std::cout << std::endl;
	}
}