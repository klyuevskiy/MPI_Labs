#include "mpi.h"
#include <vector>
#include <span>
#include <format>
#include <set>
#include <random>
#include "../utils/utils.hpp"
#include <ranges>
#include <chrono>
#include <cassert>

/*
	Задание 1. Вариант 12.
	Каждый процесс генерирует свой вектор размерности m.
	Xi - множество (все значения различные)
	Найти пересечение всех Xi i = 0..k, где k - кол-во процессов.
	Каждый процесс передает свои значение нулевому, который и находит пересечение.

*/

enum MPI_TAGS : int
{
	SEND_VECTOR_SIZE,
	SEND_MIN_NUMBER,
	SEND_MAX_NUMBER,
	SEND_SET,
};

const std::uint32_t MAX_SET_SIZE = 100;
const std::uint32_t MAX_NUMBER = 100;

std::vector<std::uint32_t> generate_set(int rank, int set_size, std::uint32_t min_number, std::uint32_t max_number)
{
	std::set<std::uint32_t> set;
	std::mt19937 generator(rank * time(0));
	std::uniform_int_distribution<std::uint32_t> distibution(min_number, max_number + 1);

	while (set.size() < set_size)
	{
		auto num = distibution(generator);
		set.insert(num);
	}

	return std::vector(set.begin(), set.end());
}

void print_set(std::vector<uint32_t> const& set, int rank)
{
	std::cout << std::format("Process {} set: ", rank);
	std::ranges::copy(set, std::ostream_iterator<std::uint32_t>(std::cout, ", "));
	std::cout << std::endl;
}

std::vector<uint32_t> intersect_sets(auto master_vector, auto const& slave_vector)
{
	std::vector<uint32_t> result;
	std::ranges::set_intersection(master_vector, slave_vector, std::back_inserter(result));
	return result;
}

void master(int rank, int process_number)
{
	/* get m, min number, max number from user input */
	std::cout << "input set size (m)" << std::endl;
	auto set_size = input_in_range<std::uint32_t>(1, MAX_SET_SIZE);
	std::cout << "input min number" << std::endl;
	auto min_number = input_in_range<std::uint32_t>(0, MAX_NUMBER);
	std::cout << "input max number" << std::endl;
	/* add set size for non-empty set */
	auto max_number = input_in_range<std::uint32_t>(min_number + set_size, MAX_NUMBER + set_size);
	
	/* send information to all slaves */
	for (int i = 1; i < process_number; i++)
	{
		MPI_Send(& set_size, 1,
			MPI_UINT32_T, i, SEND_VECTOR_SIZE, MPI_COMM_WORLD);
		MPI_Send(&min_number, 1,
			MPI_UINT32_T, i, SEND_MIN_NUMBER, MPI_COMM_WORLD);
		MPI_Send(&max_number, 1,
			MPI_UINT32_T, i, SEND_MAX_NUMBER, MPI_COMM_WORLD);
	}

	auto set = generate_set(rank, set_size, min_number, max_number);
	print_set(set, rank);

	std::vector<std::uint32_t> slave_set(set_size);

	for (int i = 1; i < process_number; i++)
	{
		MPI_Status recv_status;
		MPI_Recv(slave_set.data(), set_size, MPI_UINT32_T, i, SEND_SET, MPI_COMM_WORLD, &recv_status);
		int recv_count;
		MPI_Get_count(&recv_status, MPI_UINT32_T, &recv_count);
		assert(recv_count == set_size);

		set = intersect_sets(std::move(set), slave_set);
	}

	std::cout << "Set intersection:" << std::endl;
	print_set(set, rank);
}

void slave(int rank, int process_number)
{
	std::uint32_t set_size, min_number, max_number;
	MPI_Status recv_status;
	MPI_Recv(&set_size, 1, MPI_UINT32_T,
		0, SEND_VECTOR_SIZE, MPI_COMM_WORLD, &recv_status);
	MPI_Recv(&min_number, 1, MPI_UINT32_T,
		0, SEND_MIN_NUMBER, MPI_COMM_WORLD, &recv_status);
	MPI_Recv(&max_number, 1, MPI_UINT32_T,
		0, SEND_MAX_NUMBER, MPI_COMM_WORLD, &recv_status);
	
	auto set = generate_set(rank, set_size, min_number, max_number);
	print_set(set, rank);

	MPI_Send(set.data(), set.size(), MPI_UINT32_T, 0, SEND_SET, MPI_COMM_WORLD);
}

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);

	int rank, process_number;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &process_number);

	if (rank == 0)
		master(rank, process_number);
	else
		slave(rank, process_number);
	
	MPI_Finalize();
}