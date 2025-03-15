all:
	mpic++ main.cpp -o run

test:
	rm -f results
	mpiexec -n 2 ./run	
	mpiexec -n 4 ./run	
	mpiexec -n 8 ./run	
	mpiexec -n 16 ./run	
	mpiexec -n 32 ./run	
