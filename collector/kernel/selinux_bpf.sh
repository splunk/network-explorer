#!/bin/bash

if [ $EUID -ne 0 ]; then
   echo "This script must be run as root" 
   exit 1
fi

#selinuxenabled
#if [ $? -ne 0 ]
#then
    #echo "SELinux is not enabled"
    #exit 1
#fi

echo "Adding SELinux policy to allow eBPF operations."

FLOWMILL_TEMP=$(mktemp -d -t flowmill-XXXXX)

cat > $FLOWMILL_TEMP/spc_bpf_allow.te <<END
module spc_bpf_allow 1.0;
require {
    type spc_t;
    class bpf {map_create map_read map_write prog_load prog_run};
}
#============= spc_t ==============
allow spc_t self:bpf { map_create map_read map_write prog_load prog_run };
END
checkmodule -M -m -o $FLOWMILL_TEMP/spc_bpf_allow.mod $FLOWMILL_TEMP/spc_bpf_allow.te
semodule_package -o $FLOWMILL_TEMP/spc_bpf_allow.pp -m $FLOWMILL_TEMP/spc_bpf_allow.mod
semodule -i $FLOWMILL_TEMP/spc_bpf_allow.pp
