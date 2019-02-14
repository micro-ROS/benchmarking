# Basic recipe

libini_git=https://github.com/rxi/ini.git
libini_src_dir=./libini
libini_version=master

libini_need_autotoolize=1
libini_source_files="src/ini.c"
libini_build_cmd="autoreconf -i . && ./configure && make"

