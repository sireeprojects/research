default:
	gcc read_tap_pcap.c -lpcap -o read_pcap.x
	gcc read_tap.c -o read.x
	gcc write_tap.c -o write.x
clean:
	@rm -f run.log *.out log.txt *.x
