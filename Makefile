CC = g++
FLAGS = -Wall -pipe --std=c++17 -lm -pthread
DEBUG = -O0 -g
RELEASE = -O3
INCLUDES = -I./src

all: run

build/main: src/*.cpp
	mkdir -p build
	$(CC) $(FLAGS) $(DEBUG) $(INCLUDES) src/main.cpp -o build/main
build: build/main
release: src/*.cpp
	mkdir -p release
	$(CC) $(FLAGS) $(RELEASE) $(INCLUDES) src/main.cpp -o release/main
debug: build
	gdb ./build/main
run: release
	./release/main
run-sanitize:
	$(CC) $(FLAGS) $(INCLUDES) -fsanitize=address -fsanitize=undefined -fsanitize=leak src/main.cpp -o build/main-sanitize
	./build/main-sanitize
