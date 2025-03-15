all:
	mpic++ main.cpp -o run

test:
	rm -f 1dresults 2dresults
	mpiexec -n 1 ./run	
	mpiexec -n 4 ./run	
