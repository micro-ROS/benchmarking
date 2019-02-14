# Basic recipe

libini_git=https://github.com/rxi/ini.git
libini_src_dir=./libini
libini_version=master

libini_build_cmd="gcc -Wall -fPIC -c ./src/ini.c && gcc -shared -Wl,-soname,libini.so.1 -o libini.so.1.0 ini.o && ln -sf libini.so.1.0 libini.so"
