all:
	mpic++ main.cpp -o run

test:
	rm -f results
	mpiexec -n 2 ./run	
	mpiexec -n 4 ./run	
