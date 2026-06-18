# Collected Notes

## Table of Contents
* [Network Bridges in RHEL 8.6](#network-bridges-in-rhel-86)
* [2. Common Troubleshooting](#2-common-troubleshooting)




# Network Bridges in RHEL 8.6

## Creating empty bridges: (Requires sudo permissions)

**Create the network config file under bp_arunp**

Filename: `/etc/sysconfig/network-scripts`

  ```bash
	bp_arunp content should be like 
	BOOTPROTO="none"
	DEVICE="bp_arunp"
	NAME="bp_arunp"
	ONBOOT="yes"
	TYPE="Bridge"
  ```
**Execute the following commands**
```bash
sudo nmcli connection reload
sudo nmcli connection up bp_arunp
```

**Check if the interfaces are up and running**

Check status

```bash
ip addr show bp_arunp
```

Check bridge details

```bash
bridge link show
nmcli device status
nmcli connection show
```
