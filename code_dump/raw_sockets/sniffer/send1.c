#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

int main()
{
    char* ifname = "tp0";
    unsigned char srcMac[6];
    unsigned char dstMac[6] = {0, 0xDE, 0xAD, 0xBE, 0xEF, 0};
    int fd = socket( PF_PACKET, SOCK_RAW, htons( ETH_P_ALL ) );

     /* bind to interface */
    struct ifreq req;
    memset( &req, 0, sizeof( req ) );
    strcpy( (char*)req.ifr_name, (char*)ifname );

    if ( ioctl( fd, SIOCGIFINDEX, &req ) < 0 )
    {
        perror( "init: ioctl" );
        close( fd );
        return -1;
    }

    struct sockaddr_ll addr;
    memset( &addr, 0, sizeof( addr ) );
    addr.sll_family   = PF_PACKET;
    addr.sll_protocol = 0;
    addr.sll_ifindex  = req.ifr_ifindex;

    if ( bind( fd, (const struct sockaddr*)&addr, sizeof( addr ) ) < 0 )
    {
        perror( "init: bind fails" );
        close( fd );
        return -1;
    }

    if ( ioctl( fd, SIOCGIFHWADDR, &req ) < 0 )
    {
        perror( "init: ioctl SIOCGIFHWADDR" );
        close( fd );
        return -1;
    }

    /* store your mac address somewhere you'll need it! in your packet */
    memcpy( srcMac, (unsigned char*)req.ifr_hwaddr.sa_data, ETH_ALEN );

    int length = 60;
    unsigned char txPkt[ length ];
    unsigned char* pkt = txPkt;
    memset( txPkt, 0, sizeof( txPkt ) );

    //destination mac address
    memcpy( pkt, dstMac, 6 );
    pkt += 6;

    // source mac address
    memcpy( pkt, srcMac, 6 );
    pkt += 6;

    // add more data, like a type field!  etc

    int bytes = write( fd, txPkt, length );
}
