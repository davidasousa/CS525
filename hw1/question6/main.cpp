#include <sys/time.h>
#include <iostream>

int main(int argc, char* argv[]) {
	struct timeval start, end, diff;

	// Code Segment 1
	float* a = new float[1000000];
	for(int idx = 0; idx < 1000000; idx++) {
		a[idx] = 2.0;
	}

	gettimeofday(&start, nullptr);
	for(int idx = 0; idx < 1000000; idx++) { 
		a[idx] = a[idx] / 2.0; 
	}
	gettimeofday(&end, nullptr);
	timersub(&end, &start, &diff);

	std::cout << "Code Segment Took " << diff.tv_sec;
	std::cout << " s & " << diff.tv_usec << " us" << std::endl;

	// Code Segment 2
	float* b = new float[1000000];
	for(int idx = 0; idx < 1000000; idx++) {
		b[idx] = 2.0;
	}

	float one_over_two = 1.0 / 2.0;
	gettimeofday(&start, nullptr);
	for(int idx = 0; idx < 1000000; idx++) { 
		b[idx] = b[idx] * one_over_two; 
	}
	gettimeofday(&end, nullptr);
	timersub(&end, &start, &diff);

	std::cout << "Code Segment Took " << diff.tv_sec;
	std::cout << " s & " << diff.tv_usec << " us" << std::endl;
	
	// Freeing Memory
	delete[] a;
	delete[] b;
	return EXIT_SUCCESS;
}
