# Basic recipe

libcjson_git=https://github.com/DaveGamble/cJSON.git
libcjson_src_dir=./libcjson
libcjson_version=master

libcjson_need_autotoolize=1
libcjson_source_files="cJSON.c"
libcjson_build_cmd="autoreconf -i && ./configure && make"
