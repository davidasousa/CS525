#include <iostream>
#include <sys/time.h>

int main(int argc, char* argv[]) {
	int nums[1000000];
	for(int idx = 0; idx < 1000000; idx++) {
		nums[idx] = idx + 1;
	}

	struct timeval start, end, diff;
	int sum = 0;

	// Non Unrolled Loop
	gettimeofday(&start, nullptr);
	for(int idx = 0; idx < 1000000; idx++) {
		sum += nums[idx];
	}
	gettimeofday(&end, nullptr);

	timersub(&end, &start, &diff);
	std::cout << "Time For The Non Unrolled Loop: "; 
	std::cout << diff.tv_sec << "s" << " & ";
	std::cout << diff.tv_usec << "us" << std::endl;

	// Unrolled Loop
	gettimeofday(&start, nullptr);
	for(int idx = 0; idx < 1000000; idx+=4) {
		sum += nums[idx];
		sum += nums[idx + 1];
		sum += nums[idx + 2];
		sum += nums[idx + 3];
	}
	gettimeofday(&end, nullptr);

	timersub(&end, &start, &diff);
	std::cout << "Time For The Unrolled Loop: "; 
	std::cout << diff.tv_sec << "s" << " & ";
	std::cout << diff.tv_usec << "ms" << std::endl;

	return 0;
}
