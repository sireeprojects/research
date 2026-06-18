**Collected Notes**

[Network Bridges in RHEL 8.6 2](#_Toc231870054)

[Creating Management Bridge 3](#_Toc231870055)

[Qemu Bridge Helper 5](#_Toc231870056)

[Useful Virsh Commands 6](#_Toc231870057)

[Access Hugepages without sudo with user directory 7](#_Toc231870058)

[Useful p4 commands 8](#_Toc231870059)

[How to rename interface names to eth0 9](#_Toc231870060)

[Qemu Guest VM Silent Network Interface 10](#_Toc231870061)

[Qemu Guest VM Silent ALL Network Interface 11](#_Toc231870062)

[DPDK and VM without Hugepages 12](#_Toc231870063)

[Sudo and Library Path issue 13](#_Toc231870064)

[Embedding library path inside 14](#_Toc231870065)

[Lstopo Commands 15](#_Toc231870066)

[VSCode 16](#_Toc231870067)

[Adding Hugepages in GRUB 17](#_Toc231870068)

[FFmpeg Commands 19](#_Toc231870069)

[Create Patch between two folders 20](#_Toc231870070)

# Network Bridges in RHEL 8.6

**Creating empty bridges**

(Requires sudo permissions)

**Create the network config file under bp\_arunp**

/etc/sysconfig/network-scripts

**bp\_arunp content should be like**

BOOTPROTO="none"

DEVICE="bp\_arunp"

NAME="bp\_arunp"

ONBOOT="yes"

TYPE="Bridge"

**Execute the following commands**

sudo nmcli connection reload

sudo nmcli connection up bp\_arunp

**Check if the interfaces are up and running**

Check status:

ip addr show bp\_arunp

Check bridge details:

bridge link show

nmcli device status

nmcli connection show

# Creating Management Bridge

(Requires sudo permissions)

**Update the existing active NIC config File**

/etc/sysconfig/network-scripts/ifcfg-eno33np0

**The contents of the original NIC config file will be something like this**

DEVICE=eno33np0

BOOTPROTO=static

HWADDR=5c:6f:69:86:15:f0

IPADDR=10.206.216.2

NETMASK=255.255.255.0

ONBOOT=yes

**Update the file to**

TYPE=Ethernet

USERCTL=yes

DEVICE=eno33np0

NAME=eno33np0

BOOTPROTO=none

ONBOOT=yes

BRIDGE=management

Create the new management interface config file ifcfg-management

TYPE=Bridge

DEVICE=management

NAME=management

BOOTPROTO=static

ONBOOT=yes

IPADDR=10.206.216.3

NETMASK=255.255.255.0

GATEWAY=10.206.216.254

1. IPADDR and NETMASK should be taken from original ifcfg-eno33np0
2. GATEWAY can be obtained from the following command

ip route show | grep default

(arunp@hsv-avip16 network-scripts) ip route show | grep default

default via 10.206.216.254 dev management proto static metric 431

**Apply the changes**

# Reload files from disk

sudo nmcli connection reload

# Bring the bridge up

sudo nmcli connection up management

# Ensure the physical port is attached

sudo nmcli connection up eno33np0

**Check bridge details:**

bridge link show

nmcli device status

nmcli connection show

# Qemu Bridge Helper

**If there is an error like not allowed from ‘acl’**

Then udpate the following file

sudo vi /etc/qemu-kvm/bridge.conf

add

allow <bridge\_name>

# Useful Virsh Commands

**Get IP addresses from the virtual machine**

sudo virsh domifaddr IxVerify9p2-Sumit-0 --source agent

**List all virtual machines**

sudo virsh list --all

Add a new virtual machine using xml

Start a virtual machine

Shutdown a virtual machine

# Access Hugepages without sudo with user directory

sudo mkdir /dev/hugepages/${USER}

sudo chown ${USER}:`id -gn` /dev/hugepages/${USER}

invoke DPDK App

./vtb --huge-dir /dev/hugepages/${USER} [other EAL flags]

# Useful p4 commands

**List down all files changes without p4 edit**

p4 diff -se ...

**Show local files**

p4 status ...

**Revert back files which were changed without p4 edit**

p4 clean <filenames>

# How to rename interface names to eth0

**1. Modify the GRUB Configuration**

* Open the GRUB configuration file with root privileges:

sudo nano /etc/default/grub

* Locate the line starting with GRUB\_CMDLINE\_LINUX.
* Add net.ifnames=0 biosdevname=0 inside the quotes. It should look like this: **GRUB\_CMDLINE\_LINUX="net.ifnames=0 biosdevname=0"**
* Save and exit

**2. Update GRUB**

After saving the file, you must regenerate the GRUB bootloader configuration for the changes to take effect:

sudo update-grub

# Qemu Guest VM Silent Network Interface

(Requires sudo permissions)

The below steps were tested on ubuntu 24 for only eth0

**Step 1: Disable network manager**

sudo systemctl disable NetworkManager

sudo systemctl mask NetworkManager

**Step: Add a sysctl configuration file**

sudo vim /etc/sysctl.d/99-vtb-silence.conf

add the following replacing eth with the actual name of the network interface

# Disable IPv6 to stop NDP/RS/MLD traffic

net.ipv6.conf.<eth>.disable\_ipv6 = 1

# Stop ARP chatter

net.ipv4.conf.<eth>.arp\_announce = 2

net.ipv4.conf.<eth>.arp\_ignore = 1

# Optional: If you want to silence ALL interfaces by default

# net.ipv6.conf.all.disable\_ipv6 = 1

# net.ipv6.conf.default.disable\_ipv6 = 1

**Reboot or sudo sysctl –system to apply the changes immediately**

**Quick Verification:** After rebooting, run:

cat /proc/sys/net/ipv6/conf/<eth>/disable\_ipv6

If it returns 1, your permanent silence is in effect.

**After reboot the interface will be ‘down’ by defaults. Enable it by**

ip link set ens7 up

**For reference: Steps to enable network manager**

systemctl unmask NetworkManager

sudo systemctl enable --now NetworkManager

sudo systemctl restart NetworkManager (optional)

systemctl status NetworkManager (status check)

# Qemu Guest VM Silent ALL Network Interface

**Step 1: Disable network manager**

sudo systemctl disable NetworkManager

sudo systemctl mask NetworkManager

**Step: Add a sysctl configuration file**

sudo vim /etc/sysctl.d/99-vtb-silence.conf

Add the global silense policy

# Apply to all currently existing interfaces

net.ipv6.conf.all.disable\_ipv6 = 1

net.ipv4.conf.all.arp\_announce = 2

net.ipv4.conf.all.arp\_ignore = 1

# Apply to any interface created in the future (e.g., hot-plugged VirtIO ports)

net.ipv6.conf.default.disable\_ipv6 = 1

net.ipv4.conf.default.arp\_announce = 2

net.ipv4.conf.default.arp\_ignore = 1

**Auto-Bring Up New Interfaces**

While sysctl silences the protocols, the kernel still boots interfaces in a **DOWN** state by default. If you don't want to manually run ip link set up every time you add a port in QEMU, you can use a **udev rule**.

Create a file at /etc/udev/rules.d/99-vtb-up.rules

# Automatically bring up any virtio-net interface the moment it is detected

ACTION=="add", SUBSYSTEM=="net", DRIVERS=="virtio\_net", RUN+="/sbin/ip link set %k up"

DRIVERS==

To get the exact driver that is used by the interface run this command

udevadm info -a -p /sys/class/net/enp0s7 | grep DRIVERS

pick the driver name from the output and use it for DRIVERS==

**Reload and trigger udev rules immedidately or reboot to take effect**

udevadm control --reload-rules

udevadm trigger --subsystem-match=net --action=add

# DPDK and VM without Hugepages

**Modify DPDK parameters:**

* Add --no-huge: This forces DPDK to use standard memory instead of looking for hugetlbfs.
* Remove --huge-unlink: This flag is irrelevant when not using hugepages.
* **Optional:** You may need to define the amount of memory to allocate using -m or --socket-mem if DPDK has trouble auto-detecting requirements in no-huge mode.

./vtb -l 1-2 --file-prefix=vhost\_backend --no-pci --no-telemetry \

--no-huge …

**Modify the QEMU Command:**

QEMU needs to stop looking at /dev/hugepages and instead use a standard memory backend. However, for vhost-user to work, **share=on is still strictly required**.

**Changes:**

* Change mem-path from /dev/hugepages to /dev/shm (shared memory) or a temporary directory in /tmp. Using /dev/shm is generally preferred for performance when not using hugepages.
* Remove -mem-prealloc (optional, but usually used with hugepages to ensure they are backed immediately).

qemu-system-x86\_64 \

-name vlm0,debug-threads=on \

-machine ubuntu,accel=kvm \

-cpu host \

-enable-kvm -m 1536 \

-object memory-backend-file,id=mem,size=1536M,

mem-path=/dev/shm,share=on \

-numa node,memdev=mem \

-smp 8,sockets=2,cores=2,threads=2 -no-user-config \

-nodefaults -rtc base=utc -boot strict=on \

# Sudo and Library Path issue

**Preserve Environment**

sudo -E ./your\_dpdk\_app [args]

**explicitly set the variable**

sudo env LD\_LIBRARY\_PATH=$LD\_LIBRARY\_PATH ./your\_dpdk\_app [args]

**System wide fix**

echo "/path/to/your/dpdk/libs" | sudo tee /etc/ld.so.conf.d/dpdk.conf

sudo ldconfig

**Running Hugepages without Sudo**

To avoid sudo entirely, you can change the ownership of the hugepage mount point. In **tcsh**, you can use the $user variable:

sudo chown -R ${user} /dev/hugepages

CHECK: but will this affect other users?

# Embedding library path inside

**Using RPATH/RUNPATH**

g++ main.o -o my\_dpdk\_app -L/usr/local/lib/dpdk -ldpdk \

-Wl,-rpath,/usr/local/lib/dpdk

**Check above using**

readelf -d my\_dpdk\_app | grep -E "RPATH|RUNPATH"

Output should look like:

0x000000000000000f (RPATH) Library rpath: [/usr/local/lib/dpdk]

**Using $ORIGIN (The "Pro" Way)**

g++ main.cpp -o my\_app -L./libs -ldpdk -Wl,-rpath,'$ORIGIN/libs'

1. **RPATH vs. RUNPATH**

You might see either in your binary depending on your distribution's default settings:

1. **RPATH:** The original standard. It is searched **before** LD\_LIBRARY\_PATH. This is powerful because it overrides user environment variables.
2. **RUNPATH:** The newer standard. It is searched **after** LD\_LIBRARY\_PATH. This allows a user to "override" the baked-in path by setting an environment variable if they need to test a different library version.

To force the linker to use the older RPATH (so it stays "hardcoded" and ignores environment variables), add --disable-new-dtags:

g++ ... -Wl,-rpath,/my/path -Wl,--disable-new-dtags

# Lstopo Commands

**Install lstopo using**

sudo dnf install hwloc hwlog-gui

**Display NUMA and cpu hierarcies in a gui window**

lstopo –no-factorize

**Save NUMA and cpu hierarcies to pdf file**

lstopo –no-factorize filename.pdf

lstopo –no-factorize filename.png

lstopo –no-factorize filename.svg

# VSCode

**Change line spacing**

Ctrl+, editor.lineHeight, set to 20

**Disable squiggly red line under #include that are not able to find included headers**

Ctrl+, C\_Cpp: IntelliSense Engine , from Default to Tag Parser

**Change current line highlight to block color**

Ctrl+, Editor: Render Line Highlight, change to all

If the above does not change, then it is overriden by the theme

Ctrl+shift+p, user settings json, Select Preferences: Open User Settings (JSON) it will be top right adjacent to the search box

Add the following, Ctrl+S to save and see the effect on the editor

    "editor.renderLineHighlight": "all",

    "workbench.colorCustomizations": {

        "editor.lineHighlightBackground": "#d8d8d865",

        "editor.lineHighlightBorder": "#00000000"

    }

# Adding Hugepages in GRUB

Configuring hugepages at the kernel level via GRUB is the most reliable method because it ensures the memory is allocated immediately upon boot, preventing memory fragmentation from blocking the allocation later.

Here is the step-by-step process for RHEL 8.6.

**1. Edit the GRUB Configuration**

You need to append the hugepage parameters to the GRUB\_CMDLINE\_LINUX line in the /etc/default/grub file.

1. Open the file with a text editor (as root):

sudo vi /etc/default/grub

1. Locate the line starting with GRUB\_CMDLINE\_LINUX.
2. Add hugepagesz (the size of each page) and hugepages (the number of pages) to the end of the quotes.

**Example for 2GB of hugepages using 2MB increments:**

GRUB\_CMDLINE\_LINUX="... quiet splash hugepagesz=2M hugepages=1024"

* **hugepagesz:** Usually 2M or 1G.
* **hugepages:** The total count. (e.g., 1024 \* 2M = 2048MB).

**2. Regenerate the GRUB Configuration**

Once the file is saved, you must rebuild the grub.cfg file for the changes to take effect. RHEL 8 uses different paths depending on whether your system uses BIOS or UEFI.

**For UEFI-based systems (most common now):**

sudo grub2-mkconfig -o /boot/efi/EFI/redhat/grub.cfg

**For BIOS-based systems:**

sudo grub2-mkconfig -o /boot/grub2/grub.cfg

**Note:** If you aren't sure which one you have, running [ -d /sys/firmware/efi ] && echo UEFI || echo BIOS will tell you.

**3. Reboot and Verify**

Restart your server to apply the kernel parameters:

sudo reboot

After the system comes back up, verify the allocation using the following command:

grep Huge /proc/meminfo

**Expected Output:**

AnonHugePages: 0 kB

ShmemHugePages: 0 kB

HugePages\_Total: 1024

HugePages\_Free: 1024

HugePages\_Rsvd: 0

HugePages\_Surp: 0

Hugepagesize: 2048 kB

**Important Tips**

* **1G Pages:** If you are using hugepagesz=1G, ensure your CPU flags support it by checking grep pdpe1gb /proc/cpuinfo.
* **Runtime Changes:** If you don't want to reboot and just need to test, you can try allocating them dynamically (if memory isn't too fragmented) via:

echo 1024 > /proc/sys/vm/nr\_hugepages

However, this won't persist after a reboot unless added to /etc/sysctl.conf.

# FFmpeg Commands

Select the best resolution and audio, download and merge to mp4

yt-dlp -f "bv+ba/b" --merge-output-format mp4 "URL"

# Create Patch between two folders

List only differing or unique files

diff -qr folder1/ folder2/

List file and show internal text changes

diff -ur folder1/ folder2/

Include missing files as new files

diff -qrN folder1/ folder2/

Ignore whitespace changes

diff -qrw folder1/ folder2/

**Create a patch file**

diff -Naur original\_folder/ modified\_folder/ > changes.patch

**Apply the patch file**

cd original\_folder

patch -p1 < /path/to/changes.patch

Note: -p1 flag strips the outermost folder prefix from the path layout in the patch file so it applies correctly from inside the folder

**Revert the patch file**

patch -p1 -R < /path/to/changes.patch

# Using Taskset

Pin a process pid to a CPU number

taskset -pc CPU# PID