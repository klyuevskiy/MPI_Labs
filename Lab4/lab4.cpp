#include "mpi.h"
#include "../utils/utils.hpp"
#include <cmath>
#include <format>
#include <vector>
#include <ranges>
#include <sstream>
#include <algorithm>
#include <random>
#include <assert.h>

/*
* Задание 4. Вариант 14.
* { * * a ... a a }
* { * * a ... a a }
* { ...			  }
* { a a a ... * * }
* { a a a ... * * }
*/

constexpr int MIN_DIMENSION_SIZE = 4;
constexpr int MAX_DIMENSION_SIZE = 10;

/* aux class for matrix type */
class matrix_type
{
	MPI_Datatype first2_records, last2_records, matrix;

public:
	matrix_type(int rows_count, int columns_count)
	{
		/* type with 2 first elements in row */
		MPI_Type_vector(1, 2, columns_count - 2, MPI_INT, &first2_records);
		MPI_Type_commit(&first2_records);

		/* type with 2 last elements in row */
		int last2_block_lens[] = { 2 };
		MPI_Aint last2_displacements[] = {sizeof(int) * (columns_count - 2)}; /* offset by (m - 2) first elements */
		MPI_Datatype last2_types[] = {MPI_INT};
		MPI_Type_create_struct(1, last2_block_lens,
			last2_displacements, last2_types, &last2_records);
		MPI_Type_commit(&last2_records);
;
		int m_block_len[4] = { 1, 1, 1, 1 };
		MPI_Aint m_disp[4] =
		{
			0,
			columns_count * sizeof(int), /* offset by first row */
			(rows_count - 2) * columns_count * sizeof(int), /* offset by first n - 2 rows */
			(rows_count - 1) * columns_count * sizeof(int) /* offset by first n - 1 rows */
		};
		MPI_Datatype m_type[4] = { first2_records, first2_records, last2_records, last2_records };
		MPI_Type_create_struct(4, m_block_len,
			m_disp, m_type, &matrix);
		MPI_Type_commit(&matrix);
	}

	~matrix_type()
	{
		MPI_Type_free(&matrix);
		MPI_Type_free(&first2_records);
		MPI_Type_free(&last2_records);
	}

	operator MPI_Datatype() const
	{
		return matrix;
	}
};

void print_matrix(std::vector<int> const& matrix, int rows_number, int columns_number)
{
	for (int i = 0; i < rows_number; i++)
	{
		auto row_begin = matrix.begin() + i * columns_number;
		std::copy(row_begin, row_begin + columns_number, std::ostream_iterator<int>(std::cout, " "));
		std::cout << std::endl;
	}
}

int main(int argc, char** argv)
{
	MPI_env mpi_env(&argc, &argv);
	int rank = mpi_env.get_rank();

	if (rank != 0)
		return 0;

	/* get dimensions size from user */
	std::cout << "Input rows number" << std::endl;
	int rows_number = input_in_segment<int>(MIN_DIMENSION_SIZE, MAX_DIMENSION_SIZE);

	std::cout << "Input columns number" << std::endl;
	int columns_number = input_in_segment<int>(MIN_DIMENSION_SIZE, MAX_DIMENSION_SIZE);

	/* generate matrix by 1, 2, ..., n * m */
	std::vector<int> matrix;
	matrix.reserve(rows_number * columns_number);
	std::ranges::generate_n(
		std::back_inserter(matrix),
		rows_number * columns_number, [i = 1]() mutable { return i++; }
	);

	/* print generated matrix */
	std::cout << "Initial matrix:" << std::endl;
	print_matrix(matrix, rows_number, columns_number);

	/* define matrix type */
	matrix_type matrix_t(rows_number, columns_number);

	/* send matrix by matrix type */
	std::vector<int> result_matrix(rows_number * columns_number, 0);
	MPI_Status status;
	MPI_Sendrecv(matrix.data(), 1, matrix_t,rank, 0,
		result_matrix.data(), 1, matrix_t, rank, 0, MPI_COMM_WORLD, &status);

	std::cout << "Result matrix:" << std::endl;
	print_matrix(result_matrix, rows_number, columns_number);
}