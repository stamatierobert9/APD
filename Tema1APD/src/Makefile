.PHONY: build run clean

build:
	javac *.java

run:
	java Tema1 $(filter-out $@,$(MAKECMDGOALS))

clean:
	rm -f *.class