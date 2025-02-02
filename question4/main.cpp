#include <sys/time.h>
#include <iostream>

int main(int argc, char* argv[]) {
  size_t n = 1000;
  int** a = new int*[n];
  for(int idx = 0; idx < n; idx++) {
    a[idx] = new int[n];
  }

  int* b = new int[n];
  int* res = new int[n];
  struct timeval start, end, diff;
  
  // Setting Array Values
  for(int idx = 0; idx < n; idx++) {
    for(int jdx = 0; jdx < n; jdx++) {
      a[idx][jdx] = 2;
    }
    b[idx] = 3;
    res[idx] = 0;
  }

  // Row Major Looping 
  gettimeofday(&start, nullptr);
  for(int idx = 0; idx < n; idx++) {
    for(int jdx = 0; jdx < n; jdx++) {
      res[idx] += a[idx][jdx] * b[idx];
    }
  }
  gettimeofday(&end, nullptr);
  timersub(&end, &start, &diff);
	std::cout << "Time For The Row Major Looping: "; 
	std::cout << diff.tv_sec << "s" << " & ";
	std::cout << diff.tv_usec << "us" << std::endl;

  // Column Major Looping
  gettimeofday(&start, nullptr);
  for(int jdx = 0; jdx < n; jdx++) {
    for(int idx = 0; idx < n; idx++) {
      res[idx] += a[idx][jdx] * b[idx];
    }
  }
  gettimeofday(&end, nullptr);
  timersub(&end, &start, &diff);
	std::cout << "Time For The Column Major Looping: "; 
	std::cout << diff.tv_sec << "s" << " & ";
	std::cout << diff.tv_usec << "us" << std::endl;

  for(int idx = 0; idx < n; idx++) {
    delete[] a[idx];
  }
  delete[] a;
  delete[] b;
  delete[] res;

  return EXIT_SUCCESS;
}
