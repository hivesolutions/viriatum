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
result_dir=$target/result
dist_dir=$target/dist
deb_dir=$target/deb
deb_build_dir=$deb_dir/$name

# retrieves the path to the current script's
# directory (parent path)
script_dir=$(dirname $(readlink -f $0))

# creates the necessary directories
mkdir -p $build
mkdir -p $target
mkdir -p $result_dir
mkdir -p $dist_dir
mkdir -p $deb_dir
mkdir -p $deb_build_dir
mkdir -p $deb_build_dir/DEBIAN
mkdir -p $deb_build_dir/usr/sbin
mkdir -p $deb_build_dir/etc/viriatum
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
./configure --prefix=$result_dir --with-wwwroot=$result_dir/var/viriatum/www --enable-defaults
make && make install

# in case the previous command didn't exit properly
# must return immediately with the error
if [ $? -ne 0 ]; then cd $current && exit $?; fi

# gets back to the base directory
cd $current

# copies the binary files
cp -rf $result_dir/bin/viriatum $deb_build_dir/usr/sbin
cp -rf $result_dir/etc/viriatum/viriatum.ini $deb_build_dir/etc/viriatum
cp -rf $result_dir/etc/init.d/viriatum $deb_build_dir/etc/init.d
cp -rf $result_dir/var/viriatum/www $deb_build_dir/var/viriatum
cp -rf $script_dir/meta/* $deb_build_dir/DEBIAN

# creates the various compressed files for the
# file and then copies them to the dist directory
cd $result_dir
tar -cf $name.tar *
gzip -c $name.tar > $name.tar.gz
mv $name.tar $dist_dir
mv $name.tar.gz $dist_dir
cd $current

echo "Building deb package..."

# creates the deb file from the deb directory
dpkg-deb --build $deb_build_dir

# in case the previous command didn't exit properly
# must return immediately with the error
if [ $? -ne 0 ]; then cd $current && exit $?; fi
