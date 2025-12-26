qemu-system-x86_64 \
    -name cdn_avip_vlm_0,debug-threads=on \
    -machine pc-i440fx-10.0,accel=kvm \
    -cpu host \
    -enable-kvm -m 4G \
    -object memory-backend-file,id=mem,size=4G,mem-path=/dev/hugepages,share=on \
    -numa node,memdev=mem -mem-prealloc \
    -smp 4,sockets=1,cores=2,threads=2 -no-user-config \
    -nodefaults -rtc base=utc -boot strict=on \
    -drive file=/grid/iso/ub24.qcow2,if=none,id=drive-virtio-disk0,format=qcow2 \
    -device virtio-blk-pci,bus=pci.0,addr=0x5,drive=drive-virtio-disk0,id=virtio-disk0,bootindex=1 \
    -device virtio-vga,xres=1920,yres=1080 \
    -vnc :1000 \
    \
    -chardev socket,id=char1,path=/tmp/vhost-user0 \
    -netdev type=vhost-user,id=hostnet1,chardev=char1,vhostforce=on \
    -device virtio-net-pci,netdev=hostnet1,id=net1,mac="00:60:2f:00:00:01" \
    \
    -chardev socket,id=char2,path=/tmp/vhost-user0 \
    -netdev type=vhost-user,id=hostnet2,chardev=char2,vhostforce=on \
    -device virtio-net-pci,netdev=hostnet2,id=net2,mac="00:60:2f:00:00:02" \
    -daemonize


