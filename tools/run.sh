SCRIPTS_DIR="$( cd "$( dirname "$0" )" && pwd )"
ROOT_DIR="$SCRIPTS_DIR/.."
INSTALL_DIR="$ROOT_DIR/install"

LIB_PATH="$INSTALL_DIR/lib/libcic.so"

fullfile=$1
filename=$(basename "$fullfile")
dirname=$(dirname "$fullfile")
extension="${filename##*.}"
filename="${filename%.*}"

gendir="$dirname/gen"
if [ ! -d "$gendir" ]; then
  mkdir $gendir
fi

bc_filename="$filename.bc"
bc_filepath="$dirname/gen/$filename.bc"
if [ $extension == 'cpp' ]; then
  FLAGS=$CXXFLAGS
elif [ $extension == 'c' ]; then
  FLAGS=$CFLAGS
else
  echo "ERROR: unknown extension '$extension' of file '$filename'"
  exit 1
fi
clang $FLAGS $fullfile -c -emit-llvm -o $bc_filepath

opt -load $LIB_PATH -cic $bc_filepath -o /dev/null
