#!/bin/bash

CWD=`pwd`

# installation path
test -z "$BUILD_PREFIX" && BUILD_PREFIX="$CWD/packages"
#test -z "$INSTALL_PREFIX" && INSTALL_PREFIX="$CWD/local"
test -z "$MAKE" && MAKE="make -j10"

mkdir -p $INSTALL_PREFIX
function wget_untar { wget --progress=bar:force --no-check-certificate $1 -O- | tar xz; }
function addbashrc {
    TEST=$(grep -q "$@" ${HOME}/.bashrc; echo $?)
    if [[ "$TEST" -eq "1" ]]; then
	echo "" >> ~/.bashrc
        echo "$@" >> ~/.bashrc
    fi
}

mkdir -p $BUILD_PREFIX

# install RandomLib
cd $BUILD_PREFIX
test -d RandomLib-1.10 || wget_untar https://sourceforge.net/projects/randomlib/files/distrib/RandomLib-1.10.tar.gz/download
ln -sf $BUILD_PREFIX/RandomLib-1.10 $CWD/rlib
cd $CWD/rlib/src
g++ -g -Wall -Wextra -O3 -funroll-loops -finline-functions -fomit-frame-pointer -fpic -I../include -c -o Random.o Random.cpp

# install openssl 1.1
cd $BUILD_PREFIX
test -d openssl-1.1.1w || wget_untar https://github.com/openssl/openssl/releases/download/OpenSSL_1_1_1w/openssl-1.1.1w.tar.gz
mkdir -p $BUILD_PREFIX/local
cd openssl-1.1.1w
./config --prefix=$BUILD_PREFIX/local
make -j10
make install

# compile blinder
cd $CWD
g++ -I rlib/include -I $BUILD_PREFIX/local/include Blinders.cc -std=c++11 -Wall -Wextra -Werror -pedantic-errors -fpic -c
g++ -shared -o libBlinders.so Blinders.o rlib/src/Random.o -lssl -lcrypto

# test blinder
cd $CWD
addbashrc "export LD_LIBRARY_PATH=${CWD}:\$LD_LIBRARY_PATH"
addbashrc "export LD_LIBRARY_PATH=${CWD}/rlib/include:\$LD_LIBRARY_PATH"
source ~/.bashrc
g++ -I rlib/include testBlinding.cc -L./ -lBlinders -o testBlinding.exe
./testBlinding.exe
