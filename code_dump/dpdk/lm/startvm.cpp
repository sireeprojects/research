#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <unistd.h> 
#include <sys/types.h>


#define MAC_ADDR_BYTES  6
#define IP_ADDR_BYTES   4
#define MAX_U8_VAL      0xFF


using namespace std;
extern int opterr;
void print_line(int num);
void incr_ip_mac_addr(uint8_t *pAddr, uint32_t *i, uint32_t *p, uint32_t max_val);
void assign_port_indexing(int num_lm, int ports_per_lm, string ports_lm_index, bool index_priority);
void pregenerate_mac_addresses();
vector<string> macaddr;
vector<int>ports_index;

void display_usage_and_exit() {
    char help[] =
        "\nScript: Ixia Virtual Load Module Bringup \n\n"
        "Usage: \n"
        "  startvm [options=<value>] \n"
        "\n"
        "Options: \n"
        "  -num_lm          Number of load modules to be started\n"
        "  -ports_per_lm    Number of ports per load module\n"
	"  -ports_lm_index  Number of ports in LM as per index where each index represent 1 LM, seperated by comma\n"
        "  -ixia_ctrl_path  Domain socket path to connect adaptor with Ixia load modules\n"
        "  -qemu_path       Path of the patched qemu binay\n"
        "  -disk_path       Path to the Ixia load module image(s)\n"
        "  -disk_template   Template of the load module qcow2 image files\n"
        "                   For example, if vlm_ is specified, then vlm_0.qcow2, vlm_1.qcow2 will be used from the disk_path parameter \n"
        "  -ifup_path       Path to the qemu-ifup script\n"
        "  -vnc_model       Default is 'std'\n"
        "  -display         The Load module VNC starts from 5900+display, 5901+display, etc. The default display is 1000\n"
        "  -telnet_port     The Load module telnet port starts from 8900+telnet_port, 8901+telnet_port, etc. The default telnet_port is 1000\n"
        "  -vcpu_pin_start  Start pinning the VLM threads from this CPU id\n"
        "  -lm_vcpu         CPU id that will be used by misc VLM threads. Can be specified in range like 2-3\n"
        "                   valid only if -vcpu_pin_start is also specfied\n"
        "                   -vcpu_pin_start and -lm_vcpu should be specified together\n"
        "  -dry             Prints the script. Does not start the load modules\n"
        "  -help            Display this help \n"
        "  -bridge          Enable bridge mode to connect to backplane instead of tap interface \n"
        "                   in this mode, -bplane and -qhelper options are also required to be specified \n"
        "  -bplane          Specifies the bridge that will be used to bridge the VLM and Chassis. Required if -bridge is specified.\n"
        "  -pfc_enable	    Specifies the PFC is enabled.\n"
        "  -qhelper         Required if -bridge is specified.\n"
        ;
    printf ("%s", help);
    exit (EXIT_FAILURE);
}

void assign_port_indexing(int num_lm, int ports_per_lm, string ports_lm_index, bool index_priority)
{
	if (index_priority == false) {
		for(int i = 0; i< num_lm; i++)
			ports_index.push_back(ports_per_lm);
		return;
	}
	/*parse the ports_lm_index */
	stringstream check_str(ports_lm_index);
	// Vector of string to save tokens 
	string intermediate; 

	// Tokenizing w.r.t. space ' ' 
	while(getline(check_str, intermediate, ',')) 
	{ 
		ports_index.push_back(atoi(intermediate.c_str()));
	} 

}

int main(int argc, char *argv[]) {
    opterr                     = 0;
    int       opt              = 0;
    int       num_lm           = -1;
    string    ports_lm_index   = "";
    int       ports_per_lm     = -1;
    int       display          = 1000;
    int       q_per_port       = 2;
    int       telnet_port      = 1000;
    bool      dry              = false;
    string    vnc_model        = "std";
    string    ixia_ctrl_path   = "";
    string    qemu_path        = "";
    string    disk_path        = "";
    string    ifup_path        = "";
    string    disk_template    = "";
    // uint32_t  mac_incr_idx     = MAC_ADDR_BYTES-1;
    // uint32_t  mac_parent_idx   = MAC_ADDR_BYTES-2 ;
    // uint8_t   init_mac_addr[]  = {0x00,0x60,0x2f,0x00,0x00,0x00} ;
    int 	  vcpu_pin_start   = -1;
    string 	  pin_opts         = "";
    string    bplane           = "";
    string    qhelper          = "";
    string    lm_vcpu          = "";
    int       bridge           = -1;
    string    misc_args        = "";
    // uint8_t   mac_addr[MAC_ADDR_BYTES];
    char      mac[256];
    bool index_priority = false;

    uint32_t idx_lm = 0;
    uint32_t idx_port = 0;

    if (argc==1) {
        display_usage_and_exit();
    }

    ostringstream command;
    ostringstream string1;
    ostringstream string2;
    ostringstream string3;
    ostringstream string4;

    // memcpy(&mac_addr[0], &init_mac_addr, sizeof(uint8_t)*MAC_ADDR_BYTES);

    enum {
        opt_dry            = 1000,
        opt_help           ,
        opt_num_lm         ,
        opt_ports_per_lm   ,
	opt_ports_lm_index ,
        opt_ixia_ctrl_path ,
        opt_qemu_path      ,
        opt_disk_path      ,
        opt_disk_template  ,
        opt_ifup_path      ,
        opt_vnc_model      ,
        opt_display        ,
        opt_telnet_port    ,
        opt_vcpu_pin_start ,
        opt_bridge         ,
        opt_bplane         ,
        opt_qhelper        ,
        opt_lm_vcpu        ,
        opt_q_per_port	   ,
        opt_misc_args
    };

    static struct option longOptions[] = {
        {"dry"            , no_argument,       0, opt_dry            },
        {"help"           , no_argument,       0, opt_help           },
        {"num_lm"         , required_argument, 0, opt_num_lm         },
        {"ports_per_lm"   , required_argument, 0, opt_ports_per_lm   },
        {"ports_lm_index" , required_argument, 0, opt_ports_lm_index },
        {"ixia_ctrl_path" , required_argument, 0, opt_ixia_ctrl_path },
        {"qemu_path"      , required_argument, 0, opt_qemu_path      },
        {"disk_path"      , required_argument, 0, opt_disk_path      },
        {"disk_template"  , required_argument, 0, opt_disk_template  },
        {"ifup_path"      , required_argument, 0, opt_ifup_path      },
        {"vnc_model"      , required_argument, 0, opt_vnc_model      },
        {"display"        , required_argument, 0, opt_display        },
        {"telnet_port"    , required_argument, 0, opt_telnet_port    },
        {"vcpu_pin_start" , required_argument, 0, opt_vcpu_pin_start },
        {"bridge"         , no_argument,       0, opt_bridge         },
        {"bplane"         , required_argument, 0, opt_bplane         },
        {"pfc_enable"	  , no_argument,       0, opt_q_per_port     },
        {"qhelper"        , required_argument, 0, opt_qhelper        },
        {"lm_vcpu"        , required_argument, 0, opt_lm_vcpu        },
        {"misc_args"      , required_argument, 0, opt_misc_args      },
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long_only(argc, argv, "", longOptions, NULL)) != -1) {
        switch (opt) {
            case opt_dry            :{ dry            = true;         break; }
            case opt_num_lm         :{
                                        num_lm = atoi(optarg);
                                        idx_port = num_lm;
                                        break;
                                     }
            case opt_ports_per_lm   :{ ports_per_lm   = atoi(optarg); break; }
            case opt_ports_lm_index :{ ports_lm_index = optarg;	      break; }
            case opt_ixia_ctrl_path :{ ixia_ctrl_path = optarg;       break; }
            case opt_qemu_path      :{ qemu_path      = optarg;       break; }
            case opt_disk_path      :{ disk_path      = optarg;       break; }
            case opt_disk_template  :{ disk_template  = optarg;       break; }
            case opt_ifup_path      :{ ifup_path      = optarg;       break; }
            case opt_vnc_model      :{ vnc_model      = optarg;       break; }
            case opt_display        :{ display        = atoi(optarg); break; }
            case opt_telnet_port    :{ telnet_port    = atoi(optarg); break; }
            case opt_vcpu_pin_start :{ vcpu_pin_start = atoi(optarg); break; }
            case opt_bridge         :{ bridge         = 1;            break; }
            case opt_bplane         :{ bplane         = optarg;       break; }
            case opt_qhelper        :{ qhelper        = optarg;       break; }
            case opt_lm_vcpu        :{ lm_vcpu        = optarg;       break; }
            case opt_misc_args      :{ misc_args      = optarg;       break; }
            case opt_q_per_port     :{ q_per_port     = 10; break; }
            case opt_help           :{ display_usage_and_exit();      break; }
            case '?'                :{ display_usage_and_exit();      break; }
        }
    }

    pregenerate_mac_addresses();

    if (bridge==1) {
        if (bplane=="" || qhelper=="") {
            printf("In Bridge mode, both -bplane and -qhelper options needs to be specified\n");
            exit(EXIT_FAILURE);
        }
    }

    if (dry==false && geteuid() != 0) {
        printf ("*** Error: This application requires root permission.\n");
        exit(EXIT_FAILURE);
    }

    if (vcpu_pin_start != -1 && lm_vcpu=="") {
        printf("Both vcpu_pin_start and lm_vcpu should be specified together\n");
        exit(EXIT_FAILURE);
    }
    
    print_line(80);
    printf("Cadence Accelerated VIP\n");
    printf("(c) Cadence Design Systems, Inc. All rights reserved\n");
    print_line (80);
    printf("Number of load modules       : %d\n", num_lm);
    printf("Number of ports/load modules : %d\n", ports_per_lm);
    if(strcmp(ports_lm_index.c_str(), "") ) {
    	printf("ports_lm_indexing	     : %s\n", ports_lm_index.c_str());
	index_priority = true;
    }
    printf("Ixia Socket path             : %s\n", ixia_ctrl_path.c_str());
    printf("Ixia Qemu path               : %s\n", qemu_path.c_str());
    printf("VLM Disk path                : %s\n", disk_path.c_str());
    printf("VLM Disk template            : %s\n", disk_template.c_str());
    printf("VNC Displays start from      : %d\n", (5900+display));
    printf("Telnet port start from       : %d\n", (8900+telnet_port));
    printf("PFC enable			 : %s\n", (q_per_port == 10)? "TRUE": "FALSE");

    if (bridge==1) {
        printf("Bridge mode                  : Enabled\n");
        printf("Backplane                    : %s\n", bplane.c_str());
        printf("Qemu Helper                  : %s\n", qhelper.c_str());
    }
    if (misc_args!="") {
        printf("Other qemu arguments         : %s\n", misc_args.c_str());
    }
    print_line(80);

	if (vcpu_pin_start != -1) {
        pin_opts = "-vcpu_pin_start ";
	}

    /* assign the vector port_index as per the index_priority.
     * We will use port_index for assigning port numbers to each
     * LM, later.
     */
    assign_port_indexing(num_lm, ports_per_lm, ports_lm_index, index_priority);
#if 0
    assert (ports_index.size() != num_lm);
#else
    if (ports_index.size() != num_lm){
	printf("port_index size (%d) is not equal to num_lm(%d), please check input parameters!\n", ports_index.size(), num_lm);
	exit(-1);
    }
#endif
    for (int i=0; i<num_lm; i++) {
        if (vcpu_pin_start == -1) {
            string1 << qemu_path << " \\\n\
            "<< misc_args << " \\\n\
            -name cdn_avip_vlm_" << i << ",debug-threads=on \\\n\
            -enable-kvm -m 4096 \\\n\
            -object memory-backend-file,id=mem,size=4096M,mem-path=/dev/hugepages,share=on \\\n\
            -numa node,memdev=mem -mem-prealloc \\\n\
            -smp " << (ports_index[i]*2) << ",sockets=1,cores="<< (ports_index[i]*2) << ",threads=1 -no-user-config \\\n\
            -nodefaults -rtc base=utc -boot strict=on \\\n\
            -device piix3-usb-uhci,id=usb,bus=pci.0,addr=0x1.0x2 \\\n\
            -drive file=" << disk_path << "/" << disk_template << i <<".qcow2" << ",if=none,id=drive-virtio-disk0,format=qcow2 \\\n\
            -device virtio-blk-pci,scsi=off,bus=pci.0,addr=0x5,drive=drive-virtio-disk0,id=virtio-disk0,bootindex=1 \\\n\
            -chardev pty,id=charserial0 -device isa-serial,chardev=charserial0,id=serial0 \\\n\
            -device virtio-balloon-pci,id=balloon0,bus=pci.0,addr=0x6 -msg timestamp=on -daemonize \\\n\
            ";
        } else {
            string1 << "taskset -c " << lm_vcpu << " " << 
            qemu_path << " " << pin_opts <<  vcpu_pin_start << " \\\n\
            -name cdn_avip_vlm_" << i << ",debug-threads=on \\\n\
            -enable-kvm -m 4096 \\\n\
            -object memory-backend-file,id=mem,size=4096M,mem-path=/dev/hugepages,share=on \\\n\
            -numa node,memdev=mem -mem-prealloc \\\n\
            -smp " << (ports_index[i]*2) << ",sockets=1,cores="<< (ports_index[i]*2) << ",threads=1 -no-user-config \\\n\
            -nodefaults -rtc base=utc -boot strict=on \\\n\
            -device piix3-usb-uhci,id=usb,bus=pci.0,addr=0x1.0x2 \\\n\
            -drive file=" << disk_path << "/" << disk_template << i <<".qcow2" << ",if=none,id=drive-virtio-disk0,format=qcow2 \\\n\
            -device virtio-blk-pci,scsi=off,bus=pci.0,addr=0x5,drive=drive-virtio-disk0,id=virtio-disk0,bootindex=1 \\\n\
            -chardev pty,id=charserial0 -device isa-serial,chardev=charserial0,id=serial0 \\\n\
            -device virtio-balloon-pci,id=balloon0,bus=pci.0,addr=0x6 -msg timestamp=on -daemonize \\\n\
            ";
        }

        // incr_ip_mac_addr( mac_addr, &mac_incr_idx, &mac_parent_idx, MAX_U8_VAL);
        // sprintf(mac, "\"%02x:%02x:%02x:%02x:%02x:%02x\"", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

        if (bridge==-1) {
            string2 <<
            "\\\n\
            -vnc :" << display << " -vga " << vnc_model << " \\\n\
            -netdev tap,id=hostnet0,vhost=on,script=" << ifup_path << " \\\n\
            -device virtio-net-pci,netdev=hostnet0,id=net0,mac="<<macaddr[idx_lm]<<",bus=pci.0,addr=0x3 \\\n\
            -serial telnet:127.0.0.1:"<< (8900+telnet_port) <<",server,nowait \\\n\
            \\\n\
            ";
            idx_lm++;
        } else {
            string2 <<
            "\\\n\
            -vnc :" << display << " -vga " << vnc_model << " \\\n\
            -netdev bridge,id=hostnet0,br=" << bplane << ",helper=" << qhelper << " \\\n\
            -device virtio-net-pci,netdev=hostnet0,id=net0,mac="<<macaddr[idx_lm]<<",bus=pci.0,addr=0x3 \\\n\
            -serial telnet:127.0.0.1:"<< (8900+telnet_port) <<",server,nowait \\\n\
            \\\n\
            ";
            idx_lm++;
        }

        for (int j=0; j<ports_index[i]; j++) {
            string3 <<
            "-chardev socket,id=char"<<dec<<j+1<<",path=" << ixia_ctrl_path <<" \\\n\
            ";
            if (vcpu_pin_start != -1) {
                vcpu_pin_start += 2; // 2 vcpu per port
            }
        }

    	ostringstream string_q_1;
    	ostringstream string_q_2;
	string_q_1 << ",queues="<<(q_per_port); 
	string_q_2 << ",mq=on,mrg_rxbuf=on"<<",vectors="<< (q_per_port*2 +2);


        for (int j=0; j<ports_index[i]; j++) {
            // incr_ip_mac_addr( mac_addr, &mac_incr_idx, &mac_parent_idx, MAX_U8_VAL);
            // sprintf(mac, "\"%02x:%02x:%02x:%02x:%02x:%02x\"", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
            string4 <<
            "-netdev type=vhost-user,id=hostnet"<<dec<<j+1<<string_q_1.str()<<",chardev=char"<<dec<<j+1<<",vhostforce \\\n\
            -device virtio-net-pci,netdev=hostnet"<<dec<<j+1<<string_q_2.str()<<",id=net"<<dec<<j+1<<",mac="<<macaddr[idx_port]<<",bus=pci.0,addr=0x"<<hex<<j+7<<",emulator=30 \\\n\
            ";
            idx_port++;
        }

        command << string1.str() << string2.str() << string3.str() << string4.str() << endl;

        if (dry) {
            cout << command.str();
        } else {
            system(command.str().c_str());
        }

        command.str("");
        string1.str(""); 
        string2.str(""); 
        string3.str(""); 
        string4.str(""); 
        display++;
        telnet_port++;
    }
    return (0);
}


void print_line(int num) {
    char symbol = '-';
    int y;
    for (y = 1; y <= num; y++)
    printf("%c", symbol);
    printf("\n");
}


void incr_ip_mac_addr(uint8_t *pAddr, uint32_t *i, uint32_t *p, uint32_t max_val) {
    uint32_t incr_index;
    uint32_t parent_index;
    
    incr_index = *i ;
    parent_index = *p ;
    
    if (pAddr[incr_index] == max_val) {
        if (pAddr[parent_index] == max_val) {
            pAddr[parent_index] = 0 ;
            parent_index--;
        }
        pAddr[parent_index] +=1 ;
        pAddr[incr_index] = 0 ;
    }
    else {
        pAddr[incr_index]++ ;
    }
    *i = incr_index ;
    *p = parent_index ;
}

void pregenerate_mac_addresses() {
    macaddr.resize(0);
    char mac[256];
    uint32_t  mac_incr_idx     = MAC_ADDR_BYTES-1;
    uint32_t  mac_parent_idx   = MAC_ADDR_BYTES-2 ;
    uint8_t   mac_addr[MAC_ADDR_BYTES];
    uint8_t   init_mac_addr[]  = {0x00,0x60,0x2f,0x00,0x00,0x00} ;
    memcpy(&mac_addr[0], &init_mac_addr, sizeof(uint8_t)*MAC_ADDR_BYTES);

    for(uint32_t i=0; i<1024; i++) {
        incr_ip_mac_addr( mac_addr, &mac_incr_idx, &mac_parent_idx, MAX_U8_VAL);
        sprintf(mac, "\"%02x:%02x:%02x:%02x:%02x:%02x\"", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        macaddr.push_back(mac);
    }

}
