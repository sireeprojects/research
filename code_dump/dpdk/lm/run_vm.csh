#!/bin/csh

./startvm $*                                                    \
   -num_lm          1                                           \
   -ports_per_lm    1                                           \
   -ixia_ctrl_path  /tmp/vm_ctrl.sock                           \
   -qemu_path       /sw/tools/qemu-6.2.0/bin/qemu-system-x86_64 \
   -disk_path       /sw/vm_images/Ubuntu20.04-Minimal-test      \
   -ifup_path       ${PWD}/qemu-ifup.csh                        \
   -disk_template   load_module_
