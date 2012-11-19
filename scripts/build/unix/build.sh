#!/bin/sh
# -*- coding: utf-8 -*-

# exports the architectures for the current system
# this command allows the access to the achitecture
# variable for the system
eval `dpkg-architecture -s`

# sets the various global variables
version=0.3.0a1
architecture=$DEB_BUILD_ARCH
name=viriatum\_$version\_$architecture
name_raw=viriatum\_$version\_$architecture\_raw
name_src=viriatum\_$version\_src
current=$PWD
build=$current/build
repo=$build/repo
target=$build/target
temp_dir=$target/tmp
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
mkdir -p $temp_dir
mkdir -p $result_dir
mkdir -p $dist_dir
mkdir -p $deb_dir
mkdir -p $deb_build_dir
mkdir -p $deb_build_dir/DEBIAN
mkdir -p $deb_build_dir/usr/sbin
mkdir -p $deb_build_dir/etc/viriatum
mkdir -p $deb_build_dir/etc/init.d
mkdir -p $deb_build_dir/var/viriatum/www

# changes the permissions of some directories to
# comply with the security measures of the system
chmod -R -s $deb_build_dir
chmod -R 755 $deb_build_dir

# clones the repository to retrieve the source code
# for compilation
git clone git://github.com/hivesolutions/viriatum.git $repo --quiet

# in case the previous command didn't exit properly
# must return immediately with the error
if [ $? -ne 0 ]; then cd $current && exit $?; fi

# removes the internal git repository directory to avoid
# extra files in source distribution
rm -rf $repo/.git
rm -f $repo/.gitignore

# removes the extra (non source files) from the source
# distribution directory
rm -f $repo/TODO.md

# runs the necessary make instructions to generate
# the base scripts for configuration
cd $repo
./autogen.sh

# removes the extra (autoconf related files) from the source
# distribution directory
rm -rf $repo/autom4te.cache
rm -f $repo/autogen.sh $repo/Makefile-autoconfig

# iterates over all the modules to prepare their source code
# for compilation distribution
for module in $repo/modules/*; do
    # removes the extra (non source files) from the source
    # distribution directory (for the module)
    cd $module
    ./autogen.sh

    # removes the extra (autoconf related files) from the source
    # distribution directory (for the module)
    rm -rf $module/autom4te.cache
    rm -f $module/autogen.sh $module/Makefile-autoconfig
done

# returns to the repository directory in order to resume
# the file build (compilation
cd $repo

# copies the current source code snapshot (with configuration)
# to the temporary directory to be used latter
cp -rp $repo $temp_dir/$name_src/

# runs the configuration script and then initiates the
# comlete build process and installation into the result
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
cp -rf $script_dir/meta_$architecture/* $deb_build_dir/DEBIAN
cp -rp $result_dir $temp_dir/$name/

# creates the various compressed files for the
# file and then copies them to the dist directory
cd $result_dir
tar -cf $name_raw.tar *
mv $name_raw.tar $dist_dir
cd $temp_dir
zip -qr $name.zip $name
tar -cf $name.tar $name
gzip -c $name.tar > $name.tar.gz
zip -qr $name_src.zip $name_src
tar -cf $name_src.tar $name_src
gzip -c $name_src.tar > $name_src.tar.gz
mv $name.zip $dist_dir
mv $name.tar $dist_dir
mv $name.tar.gz $dist_dir
mv $name_src.zip $dist_dir
mv $name_src.tar $dist_dir
mv $name_src.tar.gz $dist_dir
cd $current

echo "Building deb package..."

# creates the deb file from the deb directory
dpkg-deb --build $deb_build_dir

# in case the previous command didn't exit properly
# must return immediately with the error
if [ $? -ne 0 ]; then cd $current && exit $?; fi

# moves the generated deb file into the appropriate
# (and target) dist directory
mv $deb_dir/$name.deb $dist_dir

# changes the current directory to the dist directory
# and then generates the checksum files for all the
# distribution (oriented) files
cd $dist_dir
for file in *; do
    md5sum $file > $temp_dir/$file.md5
    sha1sum $file > $temp_dir/$file.sha1
done
md5sum * > $temp_dir/MD5SUMS
sha1sum * > $temp_dir/SHA1SUMS
mv $temp_dir/*.md5 $dist_dir
mv $temp_dir/*.sha1 $dist_dir
mv $temp_dir/MD5SUMS $dist_dir
mv $temp_dir/SHA1SUMS $dist_dir
cd $current

# removes the directories that are no longer required
# for the build
rm -rf $temp_dir

# in case the previous command didn't exit properly
# must return immediately with the error
if [ $? -ne 0 ]; then cd $current && exit $?; fi
