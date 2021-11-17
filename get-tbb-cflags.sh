print_usage() {
  printf "Usage: ./get-tbb-cflags.sh ...
	-p: something that was append by tbb-install.sh to the built path (default = \"\") 

	-h: this help.
"
}

path=""

while getopts 'p:h' flag; do
  case "$flag" in
  	h) print_usage
       exit 1 ;;
	p) path="${OPTARG}" ;;
  esac
done

CURRENT_PATH=$(pwd)
TBB_INSTALL_DIR="${CURRENT_PATH}/oneTBB"
TBB_INCLUDE="${TBB_INSTALL_DIR}/include"

echo -I${TBB_INCLUDE} -Wl,-rpath,${TBB_INSTALL_DIR}/${path}build/*_release/ -L${TBB_INSTALL_DIR}/${path}build/*_release/
