# The Weien Kernel Project
-
Cust Kernel for OnePlusTurbo6X
-------
Installation steps:

1. Download AK3 from releases section

2. Flash this AK3 via KernelFlasher or Recovery

3. Waiting for phone`s boot

-------

kernel version 6.1.157 OnePlus kernel base this kernel for ColorOS16(released kernel will test on real phone)

-------
Kernel Integrated:

DroidSpaces ＆ NoMount

-------

Kernel Build Steps(Example):

Host Ubuntu24.04

1.Clone Kernel source
```
git clone https://github.com/TheWeienKernelProject/android_kernel_oneplus_mt6878 kernel
git clone https://github.com/OnePlusOSS/android_kernel_modules_and_devicetree_oneplus_mt6878 modules
```

Remember copy modules repo files
```
cp -r ~/modules/vendor ~/
cp -r ~/modules/kernel ~/
```

2. Install Build deps

```
sudo apt install libssl-dev libelf-dev libdw-dev build-essential dwarves gcc-aarch64-linux-gnu linux-tools-common 
```

3.Install Toolchain
```
curl -LO https://android.googlesource.com/platform/prebuilts/clang/host/linux-x86/+archive/refs/heads/android14-qpr3-release/clang-r487747c.tar.gz

              mkdir -p toolchains/clang-r487747c
          
              tar -xf "clang-r487747c.tar.gz" -C "toolchains/clang-r487747c"
```

cd kernel source

```
cd kernel
```

4.Integrate DroidSpaces patch

```
curl -Lo ds.patch https://raw.githubusercontent.com/ravindu644/Droidspaces-OSS/refs/heads/main/Documentation/resources/kernel-patches/GKI/below-kernel-6.12/001.GKI-below-6.12-fix_sysvipc_kabi_6_7_8.patch
patch -p1 < ds.patch 
```

5.Make config and export path
```
export PATH="~/toolchains/clang-r487747c/bin:$PATH"
          export ARCH=arm64
          export SUBARCH=arm64
          export LLVM=1
          export LLVM_IAS=1
          export CROSS_COMPILE=aarch64-linux-gnu-
```

```
make device_build_
defconfig addon.config
```

6. Compile Kernel
```
make -j$(nproc) Image 2>&1 | tee ~/build.log
```

Image will execute in arch/arm64/boot/Image

-------
Thanks:

DroidSpaces - @ravindu644

NoMount @maxsteeel

-------
