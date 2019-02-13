# Basic recipe

libini_git=https://github.com/rxi/ini.git
libini_src_dir=./ini
libini_version=master

libini_build_cmd="gcc -Wall -fPIC -c *.c && gcc -shared -Wl,-soname,libini.so.1 -o libini.so.1.0 *.o && ln -sf /opt/lib/libini.so.1.0 /opt/lib/libini.so"
