build:
	gcc process_generator.c -o process_generator.o
	gcc clk.c -o clk
	gcc -c scheduler.c
	gcc -c queue.c
	gcc -c LinkedList.c
	gcc -c BuddyBinaryTree.c
	gcc -o scheduler scheduler.o queue.o LinkedList.o BuddyBinaryTree.o
	gcc process.c -o process
	gcc test_generator.c -o test_generator.out

clean:
	rm -f *.out  testcase.txt

all: clean build

run:
	./process_generator.o testcase.txt -sch 2 -mem 3
