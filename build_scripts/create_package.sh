#!/usr/bin/env bash

BLUE='\033[0;34m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
CYAN='\033[0;36m'
PLAIN='\033[0m'
BOLD='\033[1m'
DIM='\033[2m'

echo -e "${BLUE}     ____    ${GREEN}${BOLD}NightShell${PLAIN}"
echo -e "${BLUE}  __/ / /_   ${PLAIN}${DIM}bash script for build from sources with make${PLAIN}"
echo -e "${BLUE} /_  . __/   ${PLAIN}${DIM}Powerful command interpreter (shell) for linux written in C${PLAIN}"
echo -e "${BLUE}/_    __/    ${CYAN}${DIM}maintained by alexeev-prog${PLAIN}"
echo -e "${BLUE} /_/_/       ${CYAN}https://github.com/alexeev-prog/nightshell${PLAIN}"
echo -e "${PLAIN}"

CURRDIR=$(basename $(pwd))

if [ $CURRDIR == "build_scripts" ]; then
	cd .. && make build ctypes
	cd build_scripts
elif [ $CURRDIR == "nightshell" ]; then
	make build ctypes
	cd build_scripts
fi

DIR_NAME=nightshell-$(date +"%Y-%m-%d")-package
echo -e "${BLUE}Create package-file ${DIRNAME}.tar.xz${PLAIN}"

mkdir -p $DIR_NAME
echo -e "${CYAN}Copy manual...${PLAIN}"
cp ../nightshell_en.man -r $DIR_NAME
echo -e "${CYAN}Copy binary...${PLAIN}"
cp ../bin/nightshell -r $DIR_NAME
echo -e "${CYAN}Copy README...${PLAIN}"
cp ../README.md -r $DIR_NAME
echo -e "${CYAN}Copy LICENSE...${PLAIN}"
cp ../LICENSE -r $DIR_NAME

echo -e "${CYAN}Package files: ${PLAIN}"
tree $DIR_NAME

echo -e "${CYAN}Create tar-archive...${PLAIN}"

tar -cf $DIR_NAME.tar.xz $DIR_NAME

echo -e "${CYAN}Cleaning...${PLAIN}"

rm -rf $DIR_NAME

echo -e "${GREEN}Package ${DIRNAME}.tar.xz has been successfully created!${PLAIN}"
