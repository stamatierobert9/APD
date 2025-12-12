build:
	mpicc scan.c -o scan
run: build
	mpirun --oversubscribe -np 10 ./scan
clear:
	rm scan