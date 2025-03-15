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
pqsort(std::vector<int> vec, int broadcaster, MPI_Comm bcast_group) {
	int pivot;
	int rank, size;
	MPI_Status status;
	MPI_Comm lower_bcast_group, upper_bcast_group;
	bool first_pass = false;

	while(true) {
		MPI_Comm_rank(bcast_group, &rank);
		MPI_Comm_size(bcast_group, &size);

		if(size == 1) { break; }

		// Splitting Broadcast Group -> Upper & Lower Half
		int midpoint = (size / 2);
		int color = (rank < midpoint) ? 0 : 1; // 1 If In Upper Half
		MPI_Comm_split(bcast_group, !color, rank, &lower_bcast_group);
		MPI_Comm_split(bcast_group, color, rank, &upper_bcast_group);

		// Obtaining Sent Half -> Implies We Got It From The Upper Half
		if(!first_pass) {
			if(vec.empty()) {
				int vec_size;
				MPI_Recv(&vec_size, 1, MPI_INT, 0, 0, bcast_group, &status);

				vec.resize(vec_size);
				MPI_Recv(vec.data(), vec_size, MPI_INT, 0, 0, bcast_group, &status);

				bcast_group = upper_bcast_group;
			} else {
				bcast_group = lower_bcast_group;
			}
		}
				
		MPI_Barrier(bcast_group); 
		if(rank == broadcaster) {
			pivot = find_median_of_three(vec);
			MPI_Bcast(&pivot, 1, MPI_INT, 0, bcast_group);
		}
		MPI_Barrier(bcast_group);

		if(rank == broadcaster) {
			// Splitting Into Rvec & Lvec
			std::vector<int> lvec, rvec;
			for(auto it = vec.begin(); it != vec.end(); it++) {
				if(*it < pivot) { lvec.push_back(*it); }
				else { rvec.push_back(*it); }
			}

			// Send Lvec If In Upper, Rvec If In Lower
			std::vector<int> vec = (broadcaster < size / 2) ? rvec : lvec;
			int vec_size = (broadcaster < size / 2) ? rvec.size() : lvec.size();

			MPI_Request request;
			MPI_Isend(&vec_size, vec_size, MPI_INT, broadcaster, 0, upper_bcast_group, &request);	
			MPI_Isend(vec.data(), vec_size, MPI_INT, broadcaster, 0, upper_bcast_group, &request);	

			if(first_pass) { first_pass = false; };
		}
	} 

	if(size == 1) { // Sending Everything Back To Rank 0
		int vec_size = vec.size();
		std::sort(vec.begin(), vec.end());
		MPI_Request request;
		MPI_Isend(&vec_size, vec.size(), MPI_INT, 0, 0, MPI_COMM_WORLD, &request);	
		MPI_Isend(vec.data(), vec.size(), MPI_INT, 0, 0, MPI_COMM_WORLD, &request);	
	}

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(rank == 0) {
		for(int rank_idx = 0; rank_idx < 0; rank_idx++) {
			int vec_size;
			MPI_Recv(&vec_size, 1, MPI_INT, rank_idx, 0, bcast_group, &status);
			
			std::vector<int> new_vec(vec_size);
			MPI_Recv(vec.data(), vec_size, MPI_INT, rank_idx, 0, bcast_group, &status);
			
			vec.insert(vec.end(), new_vec.begin(), new_vec.end());
		}
	}

	return vec;
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
		std::vector<int> test_vec = create_random_vec(test_sizes[test_idx]);
		gettimeofday(&start, nullptr);
		test_vec = pqsort(test_vec, 0, MPI_COMM_WORLD);
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
