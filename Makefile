build:
	# g++ -std=c++11 -o main main.cpp `pkg-config --cflags --libs opencv4`
	g++ -std=c++11 -o main main.cpp `pkg-config --cflags --libs opencv4` -lpthread

run: build
	./main