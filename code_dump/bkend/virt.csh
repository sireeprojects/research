/sw/tools/qemu-6.2.0/bin/qemu-system-x86_64   \
    -name centos9-vmdir   \
    -machine pc-i440fx-6.2,accel=kvm,usb=off,dump-guest-core=off   \
    -m 1024   \
    -cpu Skylake-Client-IBRS,ss=on,vmx=on,pdcm=on,hypervisor=on,tsc-adjust=on,clflushopt=on,umip=on,md-clear=on,stibp=on,arch-capabilities=on,ssbd=on,xsaves=on,pdpe1gb=on,ibpb=on,ibrs=on,amd-stibp=on,amd-ssbd=on,rsba=on,skip-l1dfl-vmentry=on,pschange-mc-no=on,hle=off,rtm=off   \
    -object memory-backend-file,id=mem,size=1024M,mem-path=/dev/hugepages,share=on \
    -numa node,memdev=mem -mem-prealloc \
    -smp 8,sockets=8,cores=1,threads=1   \
    -uuid f4952e8d-9719-447b-a7c0-9026aaa56a13   \
    -no-user-config   \
    -nodefaults   \
    -rtc base=utc,driftfix=slew   \
    -global kvm-pit.lost_tick_policy=delay   \
    -no-hpet   \
    -boot strict=on   \
    -device virtio-serial-pci,id=virtio-serial0,bus=pci.0,addr=0x6   \
    -drive file=/sw/vm_images/CentOS9-sparse.qcow2,format=qcow2,if=none,id=drive-virtio-disk0   \
    -device virtio-blk-pci,scsi=off,bus=pci.0,addr=0x7,drive=drive-virtio-disk0,id=virtio-disk0,bootindex=1   \
    -chardev pty,id=charserial0   \
    -device isa-serial,chardev=charserial0,id=serial0   \
    -vnc 127.0.0.1:0 \
    -vga std -global VGA.vgamem_mb=16 \
    -device intel-hda,id=sound0,bus=pci.0,addr=0x4   \
    -device hda-duplex,id=sound0-codec0,bus=sound0.0,cad=0   \
    -device virtio-balloon-pci,id=balloon0,bus=pci.0,addr=0x8   \
    -object rng-random,id=objrng0,filename=/dev/urandom   \
    -device virtio-rng-pci,rng=objrng0,id=rng0,bus=pci.0,addr=0x9   \
    -msg timestamp=on \
    \
    -chardev socket,id=char0,path=/tmp/vm.sock \
    -netdev vhost-user,id=mynet1,chardev=char0,vhostforce=on \
    -device virtio-net-pci,netdev=mynet1,mac=52:54:00:02:d9:0b

    #-vnc 127.0.0.1:0 \

    #-no-shutdown   \




#    /sw/tools/qemu-6.2.0/bin/qemu-system-x86_64   \
#        -name centos9-vmdir   \
#        -machine pc-i440fx-6.2,accel=kvm,usb=off,dump-guest-core=off   \
#        -cpu Skylake-Server-IBRS,-md-clear,+spec-ctrl,+ssbd,+hypervisor,-arat   \
#        -m 1024   \
#        -object memory-backend-file,id=mem,size=1024M,mem-path=/dev/hugepages,share=on \
#        -numa node,memdev=mem -mem-prealloc \
#        -smp 8,sockets=8,cores=1,threads=1   \
#        -uuid f4952e8d-9719-447b-a7c0-9026aaa56a13   \
#        -no-user-config   \
#        -nodefaults   \
#        -rtc base=utc,driftfix=slew   \
#        -global kvm-pit.lost_tick_policy=delay   \
#        -no-hpet   \
#        -global PIIX4_PM.disable_s3=1   \
#        -global PIIX4_PM.disable_s4=1   \
#        -boot strict=on   \
#        -device virtio-serial-pci,id=virtio-serial0,bus=pci.0,addr=0x6   \
#        -drive file=/sw/vm_images/CentOS9-sparse.qcow2,format=qcow2,if=none,id=drive-virtio-disk0   \
#        -device virtio-blk-pci,scsi=off,bus=pci.0,addr=0x7,drive=drive-virtio-disk0,id=virtio-disk0,bootindex=1   \
#        -chardev pty,id=charserial0   \
#        -device isa-serial,chardev=charserial0,id=serial0   \
#        -vnc 127.0.0.1:0 -vga std -global VGA.vgamem_mb=16 \
#        -device intel-hda,id=sound0,bus=pci.0,addr=0x4   \
#        -device hda-duplex,id=sound0-codec0,bus=sound0.0,cad=0   \
#        -device virtio-balloon-pci,id=balloon0,bus=pci.0,addr=0x8   \
#        -object rng-random,id=objrng0,filename=/dev/urandom   \
#        -device virtio-rng-pci,rng=objrng0,id=rng0,bus=pci.0,addr=0x9   \
#        -msg timestamp=on \
#        \
#        -chardev socket,id=char0,path=/tmp/vm.sock \
#        -netdev vhost-user,id=mynet1,chardev=char0 \
#        -device virtio-net-pci,netdev=mynet1,mac=52:54:00:02:d9:0b
#    
#    
#        #-no-shutdown   \
