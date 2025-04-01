all:
	g++ main_1.cpp -o run1
	g++ main_2.cpp -o run2
	g++ main_3.cpp -o run3

part1:
	bash part1.sh

part2:
	./run2

part3:
	./run3

valcheck:
	valgrind ./run1 7

clean:
	rm -f run* *.txt
