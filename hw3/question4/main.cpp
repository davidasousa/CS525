#include <iostream>
#include <mpi.h>
#include <sys/time.h>
#include <random>
#include <algorithm>
#include <vector>
#include <cmath>

std::vector<int> 
create_random_vec(int list_size) {
	std::random_device seed;
	std::mt19937 gen(seed());
	std::uniform_int_distribution<int> uniform_dist(0, list_size);
	std::vector<int> vec;

	for(int idx = 0; idx < list_size; idx++) {
		vec.push_back(uniform_dist(gen));
	}
	return vec;
}

void
vecMatMul1D(
	std::vector<std::vector<int>> mat, 
	std::vector<int> vec, 
	std::vector<int> ans
	) {
	// Setting Up MPI
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	int rows = floor((float) (mat.size()) / (float) size);

	for(int rdx = 0; rdx < rows; rdx++) {
		for(int cdx = 0; cdx < mat.size(); cdx++) {
			ans[rank * rows + rdx] += mat[rank * rows + rdx][cdx] * vec[rank * rows + rdx];
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	return;
}

void
vecMatMul2D(
	std::vector<std::vector<int>> mat, 
	std::vector<int> vec, 
	std::vector<int> ans
	) {
	// Setting Up MPI
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	int subdiv = floor((float) (mat.size()) / (float) sqrt(size));
	int row = rank / (int) sqrt(size);
	int col = rank % (int) sqrt(size);

	for(int rdx = 0; rdx < subdiv; rdx++) {
		for(int cdx = 0; cdx < subdiv; cdx++) {
			int val = mat[row * subdiv + rdx][col * subdiv + cdx] * vec[row * subdiv + rdx];
			ans[row * subdiv + rdx] += val;
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	return;
}

int main(int argc, char* argv[]) {
	struct timeval start, end, diff;
	// Creating Vector & Matrix
	int dim = 2048;
	std::vector<int> vec = create_random_vec(dim);

	std::vector<std::vector<int>> mat;
	for(int idx = 0; idx < dim; idx++) {
		mat.push_back(create_random_vec(dim));
	}
	
	MPI_Init(&argc, &argv);
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	FILE* fp1d = nullptr;
	FILE* fp2d = nullptr;
	if(rank == 0) { fp1d = fopen("1dresults", "a"); }
	if(rank == 0) { fp2d = fopen("2dresults", "a"); }
	
	// 1D Test
	std::vector<int> ans1d(dim);
	MPI_Barrier(MPI_COMM_WORLD);
	gettimeofday(&start, nullptr);
	vecMatMul1D(mat, vec, ans1d);
	gettimeofday(&end, nullptr);
	MPI_Barrier(MPI_COMM_WORLD);
	timersub(&end, &start, &diff);

	if(rank == 0) {
		fprintf(
			fp1d,
			"%d Processes, %d x %d, %ld Total MicroSeconds\n", 
			size, dim, dim, diff.tv_sec * 1000000 + diff.tv_usec
		);
	}

	// 2D Test
	std::vector<int> ans2d(dim);
	MPI_Barrier(MPI_COMM_WORLD);
	gettimeofday(&start, nullptr);
	vecMatMul2D(mat, vec, ans2d);
	gettimeofday(&end, nullptr);
	MPI_Barrier(MPI_COMM_WORLD);
	timersub(&end, &start, &diff);

	if(rank == 0) {
		fprintf(
			fp2d,
			"%d Processes, %d x %d, %ld Total MicroSeconds\n", 
			size, dim, dim, diff.tv_sec * 1000000 + diff.tv_usec
		);
	}
	//
	
	MPI_Finalize();
	if(rank == 0) { fclose(fp1d); fclose(fp2d); }
	return 0;
}
