# Basic recipe

openocd_git=https://github.com/ntfreak/openocd.git
openocd_src_dir=./openocd
openocd_version=v0.10.0

openocd_build_cmd="./bootstrap && ./configure --disable-werror && make -j8"
openocd_patches="0001-openocd_build_shared 0002-openocd-stm32f407-configurations.patch"

