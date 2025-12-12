build:
	mpicc ranksort.c -lm -o ranksort
run: build
	mpirun --oversubscribe -np 14 ./ranksort
clear:
	rm ranksort
