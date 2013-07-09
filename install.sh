#!/bin/sh

SRC_ROOT=$(pwd)

#################
#  Build MPICH  #
#################
cd ${SRC_ROOT}/mpich
./configure --with-pm=hydra
make
sudo make install

################
#  Build pmap  #
################
cd ${SRC_ROOT}
make
sudo make install
