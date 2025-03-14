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
		list[list.back()]
	};
	std::sort(vec.begin(), vec.end());
	return vec[1];
}

std::vector<int>
pqsort(std::vector<int> vec, int poffset, int rank) {
	if(poffset == 0) {
		// Base Case -> Sort
		std::sort(vec.begin(), vec.end());
		return vec;
	}
	int pivot = find_median_of_three(vec);
	// Creating Two New Vectors
	std::vector<int> lvec, rvec;
	for(auto it = vec.begin(); it != vec.end(); it++) {
		if(*it < pivot) { lvec.push_back(*it); }
		else { rvec.push_back(*it); }
	}

	int rvec_size = rvec.size();
	// Left Vec Remains In Rank 
	// Right Vec Goes To Rank + Poffset
	MPI_Send(&rank, 1, MPI_INT, rank + poffset, 0, MPI_COMM_WORLD);
	MPI_Send(&poffset, 1, MPI_INT, rank + poffset, 0, MPI_COMM_WORLD);
	MPI_Send(&rvec_size, 1, MPI_INT, rank + poffset, 0, MPI_COMM_WORLD);

	// Sending Right Vector
	MPI_Send(rvec.data(), rvec_size, MPI_INT, rank + poffset, 0, MPI_COMM_WORLD);

	// Sorting Lvec
	lvec = pqsort(lvec, poffset / 2, rank); 

	// Receiving Sorted Rvec
	MPI_Status status;
	MPI_Recv(rvec.data(), rvec_size, MPI_INT, rank + poffset, 0, MPI_COMM_WORLD, &status);

	// Putting The Two Together
	lvec.insert(lvec.end(), rvec.begin(), rvec.end());
	return lvec;
}

int main(int argc, char* argv[]) {
	int signal_finish = 0;
	int rank, size;
	struct timeval start, end, diff;

	int test_sizes[] = {
		100000,
		1000000,
		10000000
	};

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Status status;

	if(rank > 0) { // All Processes > 0 Will Recieve A Right Of Pivot Vector
		// Recieving Source Rank, Vector Size & Offset
		int src_rank, rvec_size, poffset;
		MPI_Recv(&src_rank, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&poffset, 1, MPI_INT, src_rank, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&rvec_size, 1, MPI_INT, src_rank, 0, MPI_COMM_WORLD, &status);
		// Recieving Vector
		std::vector<int> rvec(rvec_size);
		MPI_Recv(rvec.data(), rvec_size, MPI_INT, src_rank, 0, MPI_COMM_WORLD, &status);
		// Calling PQsort
		rvec = pqsort(rvec, poffset / 2, rank);
		// Sending Back The Right Vector
		MPI_Send(rvec.data(), rvec.size(), MPI_INT, src_rank, 0, MPI_COMM_WORLD);
	} else {
		std::vector<int> test_vec = create_random_vec(test_sizes[0]);
		test_vec = pqsort(test_vec, size / 2, rank);

		std::cout << check_sort(test_vec) << "\n"; 
	}


	MPI_Finalize();
	return 0;
}
