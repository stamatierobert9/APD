build:
	mpicc reduce.c -o reduce
run: build
	mpirun --oversubscribe -np 10 ./reduce
clear:
	rm reduce