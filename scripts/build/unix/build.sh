#!/bin/sh
# -*- coding: utf-8 -*-

# sets the various global variables
version=0.1.0
architecture=amd64
name=viriatum_$version_$architecture
name_raw=viriatum_$version_$architecture-raw
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
cp -rp $result_dir $temp_dir/$name/

#cd %SRC_DIR%
#xcopy /q /y /e /k viriatum.exe %RESULT_DIR%
#xcopy /q /y /a /e /k config %RESULT_DIR%\config
#xcopy /q /y /a /e /k htdocs %RESULT_DIR%\htdocs
#xcopy /q /y /a /e /k %RESULT_DIR% %TEMP_DIR%\%NAME%\
#cd %RESULT_DIR%
#tar -cf %NAME_RAW%.tar viriatum.exe config htdocs
#move %NAME_RAW%.tar %DIST_DIR%
#cd %TEMP_DIR%
#zip -qr %NAME%.zip %NAME%
#tar -cf %NAME%.tar %NAME%
#gzip -c %NAME%.tar > %NAME%.tar.gz
#move %NAME%.zip %DIST_DIR%
#move %NAME%.tar %DIST_DIR%
#move %NAME%.tar.gz %DIST_DIR%
#cd %BUILD_DIR%

# creates the various compressed files for the
# file and then copies them to the dist directory
cd $result_dir
tar -cf $name_raw.tar *
mv $name_raw.tar $dist_dir
cd $temp_dir
zip -qr $name.tar $temp_dir
tar -cf $name.tar $temp_dir
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
