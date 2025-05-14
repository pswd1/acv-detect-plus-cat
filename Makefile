all:
	g++ findcat.cpp -o findcat -I/usr/include/opencv4 `pkg-config --cflags --libs opencv4`

clean:
	rm -f findcat

