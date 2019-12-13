#!/bin/bash 

DATE=$(date +%Y%d%m%H%M%S)
TEST_BM="./res/tests/"
NAME_BM="bm_${1}_${DATE}"
PATH_BM="${TEST_BM}/${NAME_BM}"
mkdir -p ${PATH_BM}
docker cp ${2}:/uros_ws/firmware/NuttX/nuttx ${PATH_BM}/nuttx.elf
docker cp ${2}:/uros_ws/firmware/NuttX/.config ${PATH_BM}/config

(docker exec -it $2 cd /uros_ws/firmware/NuttX/ && git rev-parse HEAD) >> ${PATH_BM}/git_revs
(docker exec -it $2 cd /uros_ws/firmware/apps/ && git rev-parse HEAD) >> ${PATH_BM}/git_revs

rm  $(pwd)/${TEST_BM}/nuttx.elf

ln -s $(pwd)/${PATH_BM}/nuttx.elf $(pwd)/${TEST_BM}/

#./apps/mfa
