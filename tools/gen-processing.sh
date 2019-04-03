#!/bin/sh

. ./generic-paths.sh

SRC_PATH=${TOP_DIR}/src
INC_PATH=${TOP_DIR}/inc
TEMPLATE_PATH_C=${SRC_PATH}/processing_template.c
TEMPLATE_PATH_H=${INC_PATH}/processing_template.h

usage() 
{
	echo "Usage: $(basename $1) [-h] [filename]"
	printf "\t filename: name of the generated file from template\n"
	printf "\t\t if user append the extension to the filename, it will be removed\n"
	printf "\t -h: prints this help\n"
}

if [ $# -ne 1 ]; then
	echo "Error: Provide the name of the processing element"
	usage $0
	exit 255
fi

if [ ${1} = "-h" ]; then 
	usage $0
	exit 255
fi

FILENAME=$(echo ${1} | cut -f -1 -d '.')
FILENAME_UPPER=$(echo ${FILENAME} | tr '[:lower:]' '[:upper:]')


if [ -e "${SRC_PATH}/${FILENAME}.c" ]; then
	echo "The file ${SRC_PATH}/${FILENAME}.c already exists !!!"
	exit 255
fi


if [ -e "${SRC_PATH}/${FILENAME}.h" ]; then
	echo "The file ${INC_PATH}/${FILENAME}.h already exists !!!"
	exit 255
fi

echo "Creating ${SRC_PATH}/${FILENAME}.c and ${INC_PATH}/${1}.h"

sed 's|@@processingname@@|'${FILENAME}'|g' 	 $TEMPLATE_PATH_C > ${SRC_PATH}/${FILENAME}.c
sed 's|@@processingname@@|'${FILENAME}'|g'	 $TEMPLATE_PATH_H > ${INC_PATH}/${FILENAME}.h
sed -i 's|@@PROCESSINGNAME@@|'${FILENAME_UPPER}'|g' ${INC_PATH}/${FILENAME}.h
