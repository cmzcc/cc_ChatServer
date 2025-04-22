set -x

rm -rf `pwd`/build
mkdir -p `pwd`/build
cd `pwd`/build && 
cmake .. &&
make