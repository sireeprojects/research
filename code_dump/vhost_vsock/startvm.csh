/sw/tools/qemu-6.2.0/bin/qemu-system-x86_64 \
    -name Centos9 \
    -machine pc-i440fx-6.2,accel=kvm,usb=off,dump-guest-core=off \
    -m 1024 \
    -cpu Skylake-Client-IBRS,ss=on,vmx=on,pdcm=on,hypervisor=on,tsc-adjust=on,clflushopt=on,umip=on,md-clear=on,stibp=on,arch-capabilities=on,ssbd=on,xsaves=on,pdpe1gb=on,ibpb=on,ibrs=on,amd-stibp=on,amd-ssbd=on,rsba=on,skip-l1dfl-vmentry=on,pschange-mc-no=on,hle=off,rtm=off \
    -object memory-backend-file,id=mem,size=1024M,mem-path=/dev/hugepages,share=on \
    -numa node,memdev=mem -mem-prealloc \
    -smp 4,sockets=4,cores=1,threads=1 \
    -uuid f4952e8d-9719-447b-a7c0-9026aaa56a13 \
    -no-user-config \
    -nodefaults \
    -rtc base=utc,driftfix=slew \
    -global kvm-pit.lost_tick_policy=delay \
    -no-hpet \
    -boot strict=on \
    -device virtio-serial-pci,id=virtio-serial0,bus=pci.0,addr=0x6 \
    -drive file=/sw/vm_images/CentOS9-vsock.qcow2,format=qcow2,if=none,id=drive-virtio-disk0 \
    -device virtio-blk-pci,scsi=off,bus=pci.0,addr=0x7,drive=drive-virtio-disk0,id=virtio-disk0,bootindex=1 \
    -chardev pty,id=charserial0 \
    -device isa-serial,chardev=charserial0,id=serial0 \
    -vnc 127.0.0.1:0 \
    -vga std -global VGA.vgamem_mb=16 \
    -device virtio-balloon-pci,id=balloon0,bus=pci.0,addr=0x8 \
    -msg timestamp=on \
    \
    -netdev user,id=user0 \
    -device e1000,netdev=user0,mac="52:54:00:02:d9:00" \
    \
    -device vhost-vsock-pci,id=vhost-vsock-pci0,guest-cid=3 \
    &
