default:
	g++ tap.cpp -o mytap `pkg-config --cflags --libs libdpdk`

back:
	g++ backpressure.cpp -o mybp `pkg-config --cflags --libs libdpdk`

clean:
	rm -f mytap mybp
