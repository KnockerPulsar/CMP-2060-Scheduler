build:
	gcc src/process_generator.c -o bin/process_generator
	gcc src/clk.c -o bin/clk
	gcc -c src/scheduler.c
	gcc -c src/queue.c
	gcc -o bin/scheduler scheduler.o queue.o
	gcc src/process.c -o process
	gcc src/test_generator.c -o test_generator.out

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./bin/process_generator src/processes.txt 1
