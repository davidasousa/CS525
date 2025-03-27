all:
	g++ main.cpp -o run

test:
	./run

valcheck:
	valgrind --leak-check=full --track-origins=yes ./run

clean:
	rm run
