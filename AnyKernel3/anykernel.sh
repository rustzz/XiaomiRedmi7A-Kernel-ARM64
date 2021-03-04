# AnyKernel3 Ramdisk Mod Script
# osm0sis @ xda-developers

## AnyKernel setup
# begin properties
properties() { '
kernel.string=Cherry Kernel - by @AkiraNoSushi & @SimplyJoel
do.devicecheck=1
do.modules=1
do.systemless=0
do.cleanup=1
do.cleanuponabort=0
device.name1=pine
device.name2=olive
device.name3=olivelite
device.name4=olivewood
supported.versions=
supported.patchlevels=
'; } # end properties

# shell variables
block=/dev/block/by-name/boot;
is_slot_device=0;
ramdisk_compression=auto;


## AnyKernel methods (DO NOT CHANGE)
# import patching functions/variables - see for reference
. tools/ak3-core.sh;

# Extract needed kernel binaries depending on device
case "$(getprop ro.product.device 2>/dev/null)" in
    pine) TARGET=pine;;
    olive|olivelite|olivewood) TARGET=olive;;
esac
tools/7za x -y kernel-binaries.7z $TARGET/*
(cd $TARGET && tar c .)|(cd . && tar xf -) && rm -r $TARGET

## AnyKernel install
split_boot;
flash_boot;
flash_dtbo;
## end install
