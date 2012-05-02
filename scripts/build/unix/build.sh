#!/bin/sh
# -*- coding: utf-8 -*-

# sets the various global variables
version=0.1.0
architecture=amd64
name=viriatum_$version_$architecture
current=$PWD
build=$current/build
repo=$build/repo
target=$build/target
deb_dir=$target/deb
deb_build_dir=$deb_dir/$name

# retrieves the path to the current script's
# directory (parent path)
script_dir=$(dirname $(readlink -f $0))

# creates the necessary directories
mkdir -p $build
mkdir -p $target
mkdir -p $deb_dir
mkdir -p $deb_build_dir
mkdir -p $deb_build_dir/DEBIAN
mkdir -p $deb_build_dir/usr/sbin
mkdir -p $deb_build_dir/etc/init.d
mkdir -p $deb_build_dir/var/viriatum/www

# clones the repository to retrieve the source code
# for compilation
git clone git://github.com/hivesolutions/viriatum.git $repo --quiet

# in case the previous command didn't exit properly
# must return immediately with the error
if [ $? -ne 0 ]; then cd $current && exit $?; fi

# runs the necessary make instructions
# in the repository directory
cd $repo
make -f Makefile-autoconfig
./configure --prefix=$target --with-wwwroot=$target/var/viriatum/www
make && make install

# in case the previous command didn't exit properly
# must return immediately with the error
if [ $? -ne 0 ]; then cd $current && exit $?; fi

# gets back to the base directory
cd $current

# copies the binary files
cp -rf $target/bin/viriatum $deb_build_dir/usr/sbin
cp -rf $target/etc/init.d/viriatum $deb_build_dir/etc/init.d
cp -rf $target/var/viriatum/www $deb_build_dir/var/viriatum
cp -rf $script_dir/meta/* $deb_build_dir/DEBIAN

echo "Building deb package..."

# creates the deb file from the deb directory
dpkg-deb --build $deb_build_dir

# in case the previous command didn't exit properly
# must return immediately with the error
if [ $? -ne 0 ]; then cd $current && exit $?; fi
