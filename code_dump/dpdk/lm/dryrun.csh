/sw/tools/qemu-6.2.0/bin/qemu-system-x86_64 -enable-kvm -m 1024 -smp 2 \
    -chardev socket,id=char0,path=/tmp/emulation/sock \
    -netdev vhost-user,id=mynet1,chardev=char0 \
    -device virtio-net-pci,netdev=mynet1,mac=52:54:00:02:d9:0a \
    -object memory-backend-file,id=mem,size=1024M,mem-path=/dev/hugepages,share=on \
    -vnc :1000 -vga std \
    -numa node,memdev=mem -mem-prealloc /sw/vm_images/CentOS9-Minimal-test/CentOS9-Minimal.qcow2\

# /sw/tools/qemu-6.2.0/bin/qemu-system-x86_64 \
#     -name cdn_avip_vlm_0,debug-threads=on \
#     -enable-kvm -m 1024 \
#     -object memory-backend-file,id=mem,size=1024M,mem-path=/dev/hugepages,share=on \
#     -numa node,memdev=mem -mem-prealloc \
#     -smp 2,sockets=1,cores=2,threads=1 -no-user-config \
#     -nodefaults -rtc base=utc -boot strict=on \
#     -device piix3-usb-uhci,id=usb,bus=pci.0,addr=0x1.0x2 \
#     -drive file=/sw/vm_images/CentOS9-Minimal-test/CentOS9-Minimal.qcow2,if=none,id=drive-virtio-disk0,format=qcow2 \
#     -device virtio-blk-pci,scsi=off,bus=pci.0,addr=0x5,drive=drive-virtio-disk0,id=virtio-disk0,bootindex=1 \
#     -chardev pty,id=charserial0 -device isa-serial,chardev=charserial0,id=serial0 \
#     -device virtio-balloon-pci,id=balloon0,bus=pci.0,addr=0x6 -msg timestamp=on -daemonize \
#     \
#     -vnc :1000 -vga std \
#     -chardev socket,id=char1,path=/tmp/emulation/sock \
#     -netdev type=vhost-user,id=hostnet1,chardev=char1,vhostforce=on \
#     -device virtio-net-pci,netdev=hostnet1,id=net1,mac="00:60:2f:00:00:02",bus=pci.0,addr=0x7


#    -object memory-backend-file,id=mem,size=4096M,mem-path=/dev/hugepages,share=on \
#    -numa node,memdev=mem -mem-prealloc \

#    -chardev socket,id=char1,path=/tmp/emulation/sock \
#    -netdev type=vhost-user,id=hostnet1,queues=2,chardev=char1,vhostforce=on \
#    -device virtio-net-pci,netdev=hostnet1,mq=on,mrg_rxbuf=on,vectors=6,id=net1,mac="00:60:2f:00:00:02",bus=pci.0,addr=0x7
#    \
#    -vnc :1000 -vga std \
#    -netdev tap,id=hostnet0,vhost=on,script=/root/research/code_dump/dpdk/lm/qemu-ifup.csh \
#    -device virtio-net-pci,netdev=hostnet0,id=net0,mac="00:60:2f:00:00:01",bus=pci.0,addr=0x3 \
#    -serial telnet:127.0.0.1:9900,server,nowait \
