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
  if [ -d ./yocto/$branch/poky ]; then
    rm -rf ./yocto/$branch/poky
  fi
  if [ -d ./yocto/$branch/meta-openembedded ]; then
    rm -rf ./yocto/$branch/meta-openembedded
  fi
  if [ -d ./yocto/$branch/meta-security ]; then
    rm -rf ./yocto/$branch/meta-security
  fi
  rm -r ./yocto/$branch/source_mirrors

  git clone -b $branch https://git.yoctoproject.org/git/poky yocto/$branch/poky
  git clone -b $branch https://github.com/openembedded/meta-openembedded.git yocto/$branch/meta-openembedded
  git clone -b $branch https://git.yoctoproject.org/git/meta-security yocto/$branch/meta-security
done

