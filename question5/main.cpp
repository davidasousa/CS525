#include <stdlib.h>
#include <sys/time.h>
#include <vector>
#include <iostream>

int main(int argc, char* argv[]) {
	std::vector<int> sizes = {10, 100, 500, 1000, 5000};
	struct timeval start, end, diff;

	for(std::vector<int>::iterator it = sizes.begin(); it != sizes.end(); it++) {
		size_t n = *it;
		int** a = new int*[n];
		int** b = new int*[n];
		int** c = new int*[n];
		int** cTiled = new int*[n];

		// Allocating Arrays
		for(int idx = 0; idx < n; idx++) {
			a[idx] = new int[n];
			b[idx] = new int[n];
			c[idx] = new int[n];
			cTiled[idx] = new int[n];
		}

		// Setting Values
		for(int idx = 0; idx < n; idx++) {
			for(int jdx = 0; jdx < n; jdx++) {
				// A Formula
				if(idx == jdx) { a[idx][jdx] = 1; } 
				else { a[idx][jdx] = 2; }
				// B Formula
				if(idx < jdx) { b[idx][jdx] = 10; } 
				else { b[idx][jdx] = 20; }
				// C Formula	
				c[idx][idx] = 0;	
				cTiled[idx][idx] = 0;	
			}
		}

		// Non Tiled Matrix Computation
		gettimeofday(&start, nullptr);
		for(int idx = 0; idx < n; idx++) {
			for(int jdx = 0; jdx < n; jdx++) {
				for(int kdx = 0; kdx < n; kdx++) {
					c[idx][jdx] += a[idx][kdx] * b[kdx][jdx];
				}
			}
		}
		gettimeofday(&end, nullptr);
		timersub(&end, &start, &diff);

		std::cout << n << " Width Array Took: ";
		std::cout << diff.tv_sec << " Sec + ";
		std::cout << diff.tv_usec << "Usec & There Were ";
		std::cout << n * n * n * 2 << " Operations " << std::endl;

		// Tiled Matrix Computation
		int div = 5; // # Of Divisions
		int tWidth = n / div; 
		gettimeofday(&start, nullptr);

		// Iterating Across Tiles In C
		for(int yt = 0; yt < div; yt++) {
			for(int xt = 0; xt < div; xt++) {
				// Iterating Through The Tile In C
				for(int y = yt * tWidth; y < yt * tWidth + tWidth; y++) {
					for(int x = xt * tWidth; x < xt * tWidth + tWidth; x++) {
						// Iterating Through Tiles In A & B
						for(int tIdx = 0; tIdx < div; tIdx++) {
							// Iterating Through The Single Tile
							for(int tpos = tIdx * tWidth; tpos < (tIdx + 1) * tWidth; tpos++) {
								cTiled[y][x] += a[y][tpos] * b[tpos][x];
							}
						}
					}
				}
			}
		}

		gettimeofday(&end, nullptr);
		timersub(&end, &start, &diff);

		std::cout << n << " Width Array Took: ";
		std::cout << diff.tv_sec << " Sec + ";
		std::cout << diff.tv_usec << "Usec & There Were ";
		std::cout << n * n * n * 2 << " Operations " << std::endl;

		// End Tiled Multiplication

		// Deleting Arrays
		for(int idx = 0; idx < n; idx++) {
			delete[] a[idx];
			delete[] b[idx];
			delete[] c[idx];
			delete[] cTiled[idx];
		}
		delete[] a;
		delete[] b;
		delete[] c;
		delete[] cTiled;
	}
	return EXIT_SUCCESS;
}
