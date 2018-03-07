#!/bin/sh

#repos="fido krogoth rocko"
repos="krogoth rocko"

if [ ! -d ./yocto ]; then
  mkdir ./yocto
fi

for branch in $repos
do
  # Make the branch if it does not exist.
  if [ ! -d ./yocto/$branch ]; then
    mkdir ./yocto/$branch
  fi
  # Remove the branches.
  if [ -d ./yocto/$branch/meta-xilinx ]; then
    rm -rf ./yocto/$branch/meta-xilinx
  fi
  # Clone Xilinx repo
  git clone -b $branch git://github.com/Xilinx/meta-xilinx yocto/$branch/meta-xilinx
done

