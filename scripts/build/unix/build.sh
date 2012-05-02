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

# creates the necessary directories
mkdir -p $build
mkdir -p $target
mkdir -p $deb_dir
mkdir -p $deb_build_dir
mkdir -p $deb_build_dir/DEBIAN
mkdir -p $deb_build_dir/usr/sbin

# clones the repository to retrieve the source code
# for compilation
git clone git://github.com/hivesolutions/viriatum.git $repo --quiet

# runs the necessary make instructions
# in the repository directory
cd $repo
make -f Makefile-autoconfig
./configure --prefix=$target
make && make install

# gets back to the base directory
cd $current

# copies the binary files
cp -rf $target/bin/viriatum $deb_build_dir/usr/sbin
cp -rf $current/meta/* $deb_build_dir/DEBIAN

echo "Building deb package..."

# creates the deb file from the deb directory
dpkg-deb --build $deb_build_dir
