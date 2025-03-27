all:
	g++ main.cpp -o run
	g++ sequential.cpp -o run_seq

test:
	./run
	./run_seq

valcheck:
	valgrind --leak-check=full --track-origins=yes ./run

clean:
	rm run run_seq
