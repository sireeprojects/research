/sw/tools/qemu-6.2.0/bin/qemu-system-x86_64   \
    -name guest=CentOS9,debug-threads=on   \
    -object '{"qom-type":"secret","id":"masterKey0","format":"raw","file":"/var/lib/libvirt/qemu/domain--1-CentOS9/master-key.aes"}'   \
    -machine pc-q35-rhel9.0.0,usb=off,dump-guest-core=off,memory-backend=pc.ram   \
    -accel kvm   \
    -cpu Skylake-Client-IBRS,ss=on,vmx=on,pdcm=on,hypervisor=on,tsc-adjust=on,clflushopt=on,umip=on,md-clear=on,stibp=on,arch-capabilities=on,ssbd=on,xsaves=on,pdpe1gb=on,ibpb=on,ibrs=on,amd-stibp=on,amd-ssbd=on,rsba=on,skip-l1dfl-vmentry=on,pschange-mc-no=on,hle=off,rtm=off   \
    -m 4096   \
    -object '{"qom-type":"memory-backend-ram","id":"pc.ram","size":4294967296}'   \
    -overcommit mem-lock=off   \
    -smp 4,sockets=4,cores=1,threads=1   \
    -uuid e00f0de1-349c-4b30-a61b-6389cadbb274   \
    -no-user-config   \
    -nodefaults   \
    -chardev socket,id=charmonitor,path=/var/lib/libvirt/qemu/domain--1-CentOS9/monitor.sock,server=on,wait=off   \
    -mon chardev=charmonitor,id=monitor,mode=control   \
    -rtc base=utc,driftfix=slew   \
    -global kvm-pit.lost_tick_policy=delay   \
    -no-hpet   \
    -no-shutdown   \
    -global ICH9-LPC.disable_s3=1   \
    -global ICH9-LPC.disable_s4=1   \
    -boot strict=on   \
    -device pcie-root-port,port=16,chassis=1,id=pci.1,bus=pcie.0,multifunction=on,addr=0x2   \
    -device pcie-root-port,port=17,chassis=2,id=pci.2,bus=pcie.0,addr=0x2.0x1   \
    -device pcie-root-port,port=18,chassis=3,id=pci.3,bus=pcie.0,addr=0x2.0x2   \
    -device pcie-root-port,port=19,chassis=4,id=pci.4,bus=pcie.0,addr=0x2.0x3   \
    -device pcie-root-port,port=20,chassis=5,id=pci.5,bus=pcie.0,addr=0x2.0x4   \
    -device pcie-root-port,port=21,chassis=6,id=pci.6,bus=pcie.0,addr=0x2.0x5   \
    -device pcie-root-port,port=22,chassis=7,id=pci.7,bus=pcie.0,addr=0x2.0x6   \
    -device pcie-root-port,port=23,chassis=8,id=pci.8,bus=pcie.0,addr=0x2.0x7   \
    -device pcie-pci-bridge,id=pci.9,bus=pci.7,addr=0x0   \
    -device qemu-xhci,p2=15,p3=15,id=usb,bus=pci.2,addr=0x0   \
    -device virtio-serial-pci,id=virtio-serial0,bus=pci.3,addr=0x0   \
    -blockdev '{"driver":"file","filename":"/sw/vm_images/CentOS9/CentOS9.qcow2","node-name":"libvirt-2-storage","auto-read-only":true,"discard":"unmap"}'   \
    -blockdev '{"node-name":"libvirt-2-format","read-only":false,"driver":"qcow2","file":"libvirt-2-storage"}'   \
    -device virtio-blk-pci,bus=pci.4,addr=0x0,drive=libvirt-2-format,id=virtio-disk0,bootindex=1   \
    -device ide-cd,bus=ide.0,id=sata0-0-0   \
    -netdev tap,fd=26,id=hostnet0   \
    -device virtio-net-pci,netdev=hostnet0,id=net0,mac=52:54:00:6a:3d:82,bus=pci.1,addr=0x0   \
    -netdev tap,fd=27,id=hostnet1   \
    -device virtio-net-pci,netdev=hostnet1,id=net1,mac=52:54:00:2e:db:b2,bus=pci.8,addr=0x0   \
    -chardev pty,id=charserial0   \
    -device isa-serial,chardev=charserial0,id=serial0   \
    -chardev socket,id=charchannel0,path=/var/lib/libvirt/qemu/channel/target/domain--1-CentOS9/org.qemu.guest_agent.0,server=on,wait=off   \
    -device virtserialport,bus=virtio-serial0.0,nr=1,chardev=charchannel0,id=channel0,name=org.qemu.guest_agent.0   \
    -audiodev '{"id":"audio1","driver":"none"}'   \
    -vnc 127.0.0.1:0,audiodev=audio1   \
    -device VGA,id=video0,vgamem_mb=16,bus=pcie.0,addr=0x1   \
    -device virtio-balloon-pci,id=balloon0,bus=pci.5,addr=0x0   \
    -object '{"qom-type":"rng-random","id":"objrng0","filename":"/dev/urandom"}'   \
    -device virtio-rng-pci,rng=objrng0,id=rng0,bus=pci.6,addr=0x0   \
    -sandbox on,obsolete=deny,elevateprivileges=deny,spawn=deny,resourcecontrol=deny   \
    -msg timestamp=on 
