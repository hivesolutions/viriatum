# sets the various global variables
architecture=amd64
name=viriatum_$architecture
current=$PWD
build=$current/build
repo=$build/repo
target=$build/target
deb_dir=$build/$name

# creates the necessary directories
mkdir -p $build
mkdir -p $deb_dir
mkdir -p $deb_dir/DEBIAN
mkdir -p $deb_dir/usr/sbin

# clones the repository to retrieve the source code
# for compilation
git clone git://github.com/hivesolutions/viriatum.git $repo

# runs the necessary make instructions
# in the repository directory
cd $repo
make -f Makefile-autoconfig
./configure --prefix=$target
make && make install

# gets back to the base directory
cd $current

# copies the binary files
cp -rf $target/bin/viriatum $deb_dir/usr/sbin
cp -rf $current/meta/* $deb_dir/DEBIAN

echo "Building deb package..."

# creates the deb file from the deb directory
dpkg-deb --build $deb_dir
