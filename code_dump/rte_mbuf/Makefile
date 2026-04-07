default:
	g++ -O3 -march=native main.cpp -o app `pkg-config --cflags --libs libdpdk` -lrte_bus_vdev

run:
	@./app -l 3-4 \
      --file-prefix=virtio_app \
      --no-telemetry \
		--huge-unlink

clean:
	@rm -f app
