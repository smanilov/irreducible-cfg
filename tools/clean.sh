SCRIPTS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$SCRIPTS_DIR/.."

rm -rf $ROOT_DIR/install
rm -rf $ROOT_DIR/build
find . -type d -name gen | xargs rm -rf
