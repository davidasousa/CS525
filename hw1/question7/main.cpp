#include <sys/time.h>
#include <iostream>

int main(int argc, char* argv[]) {
	float* a = new float[10000000];
	for(int idx = 0; idx < 10000000; idx++) { a[idx] = 1; }
	struct timeval start, end, diff;
	
	// Code Segment 1
	gettimeofday(&start, nullptr);
	float sum = 0.0;
	for(int idx = 0; idx < 10000000; idx++) {
		sum += a[idx];
	}
	gettimeofday(&end, nullptr);
	timersub(&end, &start, &diff);

	std::cout << "Total Time: ";
	std::cout << diff.tv_sec << " s & ";
	std::cout << diff.tv_usec << " us" << std::endl;

	sum = 0.0;
	// Code Segment 2
	int s = 0;
	int t = 1;
	int u = 2;
	int v = 3;
	float sum1 = 0.0;
	float sum2 = 0.0;
	float sum3 = 0.0;
	float sum4 = 0.0;

	gettimeofday(&start, nullptr);
	for(int idx = 0; idx < 10000000; idx+=4) {
		sum1 = a[s];
		sum2 = a[t]; 
		sum3 = a[u];
		sum4 = a[v];
		s += 4;
		t += 4;
		u += 4;
		v += 4;
		
		sum += (sum1 + sum2 + sum3 + sum4);
	}
	gettimeofday(&end, nullptr);
	timersub(&end, &start, &diff);

	std::cout << "Total Time: ";
	std::cout << diff.tv_sec << " s & ";
	std::cout << diff.tv_usec << " us" << std::endl;

	delete[] a;
	return EXIT_SUCCESS;	
}
