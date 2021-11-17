#!/bin/bash

print_usage() {
  printf "Usage: ./tbb-install.sh ...
	-c: CXXFLAGS to pass to make (default = \"\")
	-p: something to append to the built path (default = \"\") 

	-h: this help.
"
}

CFLAGS=""
path=""

while getopts 'c:p:h' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;
    c) CFLAGS="${OPTARG}" ;;
		p) path="${OPTARG}" ;;
  esac
done

if [ ! -d ./oneTBB ]; then
	git clone --branch tbb_2020 https://github.com/oneapi-src/oneTBB oneTBB
	echo -e "\n\n"
fi

echo -e "compiling..."

if [ "$CFLAGS" != "" ]; then
	(cd oneTBB && \
	make clean -j all CXXFLAGS="${CFLAGS}")
else
	(cd oneTBB && \
	make clean -j all)
fi

echo ""

if [ "$path" != "" ]; then
	echo -e "\nmoving files..."
	rm -r ./oneTBB/${path}build
	cp -r -f ./oneTBB/build ./oneTBB/${path}build
fi

echo -e "\nDone! You can now the output of \"./get-tbb-cflags.sh\" to cflags when compiling"

