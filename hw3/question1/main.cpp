#include <iostream>
#include <sys/time.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);
	struct timeval start_time, end_time, diff;
	FILE* fp = fopen("results", "-w");

	MPI_Comm comm1, comm2;
	int rank1, rank2;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank1);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank2);
	MPI_Status status;


	for(int idx = 1; idx <= 100; idx++) {
		int size = idx * 1000;
		char* message = new char[size];
		char* recvBuf = new char[size];

		gettimeofday(&start_time, nullptr);
		for(int jdx = 0; jdx < 3; jdx++) {
			// Send 1 -> 2
			MPI_Send(
				(void*) message, 
				size,
				MPI_BYTE,
				rank2,
				0,
				MPI_COMM_WORLD
			);
			// Recv 1 -> 2
			MPI_Recv(
				(void*) recvBuf,
				size,
				MPI_BYTE,
				rank1,
				0,
				MPI_COMM_WORLD,
				&status
			);
			// Send 2 -> 1
			MPI_Send(
				(void*) recvBuf, 
				size,
				MPI_BYTE,
				rank1,
				0,
				MPI_COMM_WORLD
			);
			// Recv 2 -> 1
			MPI_Recv(
				(void*) message,
				size,
				MPI_BYTE,
				rank2,
				0,
				MPI_COMM_WORLD,
				&status
			);
		}
		
		gettimeofday(&end_time, nullptr);
		timersub(&end_time, &start_time, &diff);

		fprintf(fp, "%d : %ld %ld\n", size, diff.tv_sec, diff.tv_usec);
	}

	fclose(fp);
	MPI_Finalize();
	return 0;
}
