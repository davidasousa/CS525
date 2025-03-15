#include <iostream>
#include <mpi.h>
#include <sys/time.h>
#include <random>
#include <algorithm>
#include <vector>

// Slide 60 Of Sorting
// Message Passing Quicksort / Parallelizing Quicksort

bool
check_sort(std::vector<int> vec) {
	for(int idx = 0; idx < vec.size() - 1; idx++) {
		if(vec[idx] > vec[idx + 1]) { return false; }
	}
	return true;
}

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

int 
find_median_of_three(std::vector<int> list) {
	std::vector<int> vec = {
		list.front(), 
		list[list.size() / 2], 
		list.back()
	};
	std::sort(vec.begin(), vec.end());
	return vec.at(1);
}

std::vector<int>
pqsort(std::vector<int> vec) {
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Status status;

	int pivot; 
	if(rank == 0) {
		pivot = find_median_of_three(vec);
		MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_COMM_WORLD);
	}
	MPI_Barrier(MPI_COMM_WORLD);

	int part_size = vec.size() / size;
	int start = part_size * rank;
	int end = (part_size + 1) * rank;

	// Adding Remainder Numbers To Final Vec
	std::vector<int> subvec(vec.begin() + start, vec.begin() + end);
	if(rank == (size - 1) && end != vec.size()) {
		subvec.insert(subvec.end(), vec.begin() + end, vec.end());
	}

	// Partitioning Each Subvec
	std::vector<int> lvec, rvec;
	for(auto it = subvec.begin(); it != subvec.end(); it++) {
		if(*it < pivot) { lvec.push_back(*it); }
		else { rvec.push_back(*it); }
	}
	
	if(rank < size / 2) {
			// Sending The Rvec & Recieving Lvec
			int rvec_size = rvec.size();
			MPI_Send(&rvec_size, 1, MPI_INT, rank + size / 2, 0, MPI_COMM_WORLD);
			MPI_Send(rvec.data(), rvec_size, MPI_INT, rank + size / 2, 0, MPI_COMM_WORLD);

			int vec_size;
			MPI_Recv(&vec_size, 1, MPI_INT, rank + size / 2, 0, MPI_COMM_WORLD, &status);
			std::vector<int> nvec(vec_size);
			MPI_Recv(nvec.data(), vec_size, MPI_INT, rank + size / 2, 0, MPI_COMM_WORLD, &status);

			lvec.insert(lvec.end(), nvec.begin(), nvec.end());
			subvec = lvec;
	} else {
			// Sending Lvec & Recieving Rvec
			int lvec_size = lvec.size();
			MPI_Send(&lvec_size, 1, MPI_INT, rank - size / 2, 0, MPI_COMM_WORLD);
			MPI_Send(lvec.data(), lvec_size, MPI_INT, rank - size / 2, 0, MPI_COMM_WORLD);

			int vec_size;
			MPI_Recv(&vec_size, 1, MPI_INT, rank - size / 2, 0, MPI_COMM_WORLD, &status);
			std::vector<int> nvec(vec_size);
			MPI_Recv(nvec.data(), vec_size, MPI_INT, rank - size / 2, 0, MPI_COMM_WORLD, &status);

			lvec.insert(lvec.end(), nvec.begin(), nvec.end());
			subvec = lvec;
	}

	// Sorting The Individual Lists
	std::sort(subvec.begin(), subvec.end());

	int subvec_size = subvec.size();
	if(rank > 0) {
		MPI_Send(&subvec_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		MPI_Send(subvec.data(), subvec_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
	} else {
		for(int rank_idx = 1; rank_idx < size; rank_idx++) {
			MPI_Recv(&subvec_size, 1, MPI_INT, rank_idx, 0, MPI_COMM_WORLD, &status);
			std::vector<int> nvec(subvec_size);
			MPI_Recv(nvec.data(), subvec_size, MPI_INT, rank_idx, 0, MPI_COMM_WORLD, &status);	

			subvec.insert(subvec.end(), nvec.begin(), nvec.end());
		}
	}
		
	return subvec;
}

int main(int argc, char* argv[]) {
	FILE* fp = nullptr;
	struct timeval start, end, diff;

	int test_sizes[] = {
		100000,
		1000000,
		10000000
	};
	
	int rank, size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Status status;
	
	if(rank == 0) { fp = fopen("results", "a"); }

	for(int test_idx = 0; test_idx < 3; test_idx++) {
		std::vector<int> test_vec;
		test_vec = create_random_vec(test_sizes[test_idx]);

		gettimeofday(&start, nullptr);
		test_vec = pqsort(test_vec);
		gettimeofday(&end, nullptr);
		timersub(&end, &start, &diff);

		if(rank == 0) {
			fprintf(
				fp,
				"%d Processes, %d Elements, %ld Total MicroSeconds, Successfully Sorted: %d\n", 
				size, test_sizes[test_idx], diff.tv_sec * 100000 + diff.tv_usec, (bool)check_sort(test_vec)
			);
		}
	}

	MPI_Finalize();
	if(rank == 0) { fclose(fp); }
	return 0;
}
