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
TARGET=$(getprop ro.product.device 2>/dev/null | cut -c -5)
tools/7za e kernel-binaries.7z -so "$TARGET" > Image.gz-dtb
tools/7za e kernel-binaries.7z -so "pronto_wlan.ko" | tee modules/vendor/lib/modules/pronto_wlan.ko modules/system/lib/modules/pronto_wlan.ko

## AnyKernel install
split_boot;
flash_boot;
flash_dtbo;
## end install
