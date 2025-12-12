build:
	mpicc broadcast.c -o broadcast
run: build
	mpirun -np 4 ./broadcast
clear:
	rm broadcast