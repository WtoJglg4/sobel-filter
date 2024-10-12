build:
	g++ -std=c++11 -o main main.cpp `pkg-config --cflags --libs opencv4`

run: build
	./main