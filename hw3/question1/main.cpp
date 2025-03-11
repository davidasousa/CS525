#include <iostream>
#include <sys/time.h>
#include <mpi.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);
	struct timeval start_time, end_time, diff;

	int rank;
	int size;
	MPI_Status status;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if(rank == 0) {
		// Sender
		FILE* fp = fopen("results", "w");
		for(int idx = 1; idx < 100; idx++) {
			int msg_size = idx * 1000;
			char* msg = new char[msg_size];
			gettimeofday(&start_time, nullptr);
			
			for(int jdx = 0; jdx < 10000; jdx++) {
				MPI_Send((void*) msg, msg_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
				MPI_Recv((void*) msg, msg_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD, &status);
			}

			gettimeofday(&end_time, nullptr);
			timersub(&end_time, &start_time, &diff);

			fprintf(fp, "%ld\n", diff.tv_sec * 1000000 + diff.tv_usec);
		}
		fclose(fp);
	} else {
		// Reciever
		for(int idx = 1; idx < 100; idx++) {
			int msg_size = idx * 1000;
			char* msg_buf = new char[msg_size];

			for(int jdx = 0; jdx < 10000; jdx++) {
				MPI_Recv((void*) msg_buf, msg_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);
				MPI_Send((void*) msg_buf, msg_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
			}
		}
	}

	MPI_Finalize();
	return 0;
}
