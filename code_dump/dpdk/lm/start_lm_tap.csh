#!/bin/csh
set ENV_ROOT=${PWD}/..

sudo  LD_LIBRARY_PATH=$LD_LIBRARY_PATH ./startvm $*           \
   -num_lm          1                                         \
   -ports_per_lm    2                                         \
   -ixia_ctrl_path  /tmp/arunp_ixia_ctrl.sock                 \
   -qemu_path       ${ENV_ROOT}/qemu/bin/qemu-system-x86_64   \
   -disk_path       /lan/dscratch/arunp/test                  \
   -ifup_path       ${ENV_ROOT}/lm/qemu-ifup.csh              \
   -disk_template   load_module_
