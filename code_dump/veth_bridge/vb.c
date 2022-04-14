// This is a program which is going to make the bridge between two interfaces

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <stdbool.h>
#include <errno.h>

int _sock[2];

int _CtrlCHandler()
{
	printf(" terminating...\n");
	close(_sock[0]);
	close(_sock[1]);
	printf("all sockets closed successfully.\n");
}

void CtrlCHandler(int dummy)
{
	_CtrlCHandler();
}

int OpenSocket(const char *ifname)
{
	// getting socket
	int socket_fd = socket(PF_PACKET, SOCK_RAW/*|SOCK_NONBLOCK*/, htons(ETH_P_ALL));
	if (socket_fd != -1) 
        printf("Socket creation: %s\n", strerror(errno));

	// init interface options struct with the interface name
	struct ifreq if_options;
	memset(&if_options, 0, sizeof(struct ifreq));
	strncpy(if_options.ifr_name, ifname, sizeof(if_options.ifr_name) - 1);
	if_options.ifr_name[sizeof(if_options.ifr_name) - 1] = 0;

	// enable promiscuous mode
	if(ioctl(socket_fd, SIOCGIFFLAGS, &if_options) != -1);
        printf("Set promiscuous mode: %s\n", strerror(errno));

	if_options.ifr_flags |= IFF_PROMISC;
	if(ioctl(socket_fd, SIOCSIFFLAGS, &if_options) != -1);
        printf("Enable promiscuous mode: %s\n", strerror(errno));

	// get interface index
	if(ioctl(socket_fd, SIOCGIFINDEX, &if_options) != -1);
        printf("Get interface index: %s\n", strerror(errno));

	// bind socket to the interface
	struct sockaddr_ll my_addr;
	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sll_family = PF_PACKET;
	my_addr.sll_ifindex = if_options.ifr_ifindex;
	if(bind(socket_fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) != -1)
        printf("Bind: %s\n", strerror(errno));

	// socket is ready
	return socket_fd;
}

int MakeBridge(const char *if1, const char *if2)
{
	_sock[0] = OpenSocket(if1);
	_sock[1] = OpenSocket(if2);

	printf("Sockets %d and %d opened successfully\n", _sock[0], _sock[1]);

	fd_set readfds, orig;
	int activity;
	char buf[1<<16];
	signal(SIGINT, CtrlCHandler);
	int packetNumber = 0;
	// const int numSocks = _countof(_sock);
	const int numSocks = 2;
	int nfds = _sock[0];
	for (int i = 1; i < numSocks; i++)
		if (_sock[i] > nfds)
			nfds = _sock[i];
	nfds++;
	FD_ZERO(&orig);
	for (int i = 0; i < numSocks; i++)
		FD_SET(_sock[i], &orig);
	while (true)
	{
		readfds = orig;
		activity = select(nfds, &readfds, NULL, NULL, NULL);
		if (activity == -1)  // sockets closed
			break;
		// CHECK(activity > 0, "");
		for (int i = 0; i < numSocks; i++)
			if (FD_ISSET(_sock[i], &readfds))
			{
				int len = recvfrom(_sock[i], buf, sizeof(buf), MSG_TRUNC, NULL, NULL);
				// printf("[%d -> %d] %10d %d %d\n", _sock[i], _sock[!i], ++packetNumber, i, len);
				sendto(_sock[!i], buf, len, 0, NULL, 0);
			}
	}

	return 0;
}


int Usage(int argc, const char *argv[])
{
	printf("Usage: %s <if1> <if2>\n", argv[0]);
	printf("Bridges two interfaces with the names specified.\n");
	return 0;
}

int main(int argc, const char *argv[])
{
	if (argc != 3)
		return Usage(argc, argv);
	MakeBridge(argv[1], argv[2]);
	return 0;
}
