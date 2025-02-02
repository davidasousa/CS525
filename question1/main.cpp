#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h> // Get Time Of Day

int main(int argc, char* argv[]) {
	size_t length = 10000000;
	long sum = 0;
	long* a = (long*)malloc(length * sizeof(long));
	for(size_t i = 0; i < length; i++) {
		a[i];
	}
  // Part One
	struct timeval start_time, end_time, diff;
	gettimeofday(&start_time, nullptr);
	for(int i = 0; i < 1000; i++) {
		sum += a[i];
	}
	gettimeofday(&end_time, nullptr);
	timersub(&end_time, &start_time, &diff);
	printf("The Total Time: %lu Iterations Is %ldus\n", length, diff.tv_usec);
  // End Part 1
  
  sum = 0;
  length = 10000000;

	// Part 2
	gettimeofday(&start_time, nullptr);
	for(int i = 0; i < length; i++) {
		sum += a[i];
	}
	gettimeofday(&end_time, nullptr);
	timersub(&end_time, &start_time, &diff);

	printf("The Total Time: %lu Iterations Is %ldus\n", length, diff.tv_usec);
	// End User Code

	free(a);
	return 0;
}
