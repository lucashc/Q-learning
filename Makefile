CC = g++
FLAGS = -Wall -pipe -O2 -g --std=c++17 -lm -pthread
INCLUDES = -I./src

all: run

build/main: src/*.cpp
	mkdir -p build
	$(CC) $(FLAGS) $(INCLUDES) src/main.cpp -o build/main
build: build/main
debug: build
	gdb ./build/main
run: build
	./build/main
run-sanitize:
	$(CC) $(FLAGS) $(INCLUDES) -fsanitize=address -fsanitize=undefined -fsanitize=leak src/main.cpp -o build/main-sanitize
	./build/main-sanitize