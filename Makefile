all:
	g++ main.cpp -o main -I/usr/include/opencv4 `pkg-config --cflags --libs opencv4`

clean:
	rm -f main

