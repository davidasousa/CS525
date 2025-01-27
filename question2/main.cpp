#include <sys/time.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char* argv[]) {
	struct timeval start, end, diff;
	size_t length = 10000000;
	FILE* fp = fopen("output.txt", "w");

	for(size_t skip = 1; skip <= 100; skip++) {
		long sum = 0;
		long* a = (long*) malloc(sizeof(long) * skip * length);
		for(int idx = 0; idx < skip * length; idx++) {
			a[idx] = 2;
		}

		gettimeofday(&start, nullptr);
		for(int i = 0; i < skip * length; i += skip) {
			sum += a[i];
		}
		gettimeofday(&end, nullptr);
		timersub(&end, &start, &diff);
		delete a;

		fprintf(fp, "Skip: %ld, Total Time: %ld\n", skip, diff.tv_usec);
	}
	fclose(fp);
}
