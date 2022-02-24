#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdlib.h>

int main() {

    struct ifreq ifreq;
    int sockfd=socket(PF_INET, SOCK_DGRAM, 0);
  if(sockfd<0)
  {
    printf("create socket");
    fprintf(stderr, "%s\n",strerror(errno));
    exit(EXIT_FAILURE);
  }
  sprintf( ifreq.ifr_name, "%s", "tp0");
  if(ioctl(sockfd, SIOCGIFINDEX, &ifreq)<0) 
  {
    fprintf(stderr, "%s\n",strerror(errno));
    exit(EXIT_FAILURE);
  }

    printf("id: %d\n", ifreq.ifr_ifindex);

    return 0;
}
