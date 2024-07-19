#!/bin/bash

# Set the installation prefix
PREFIX=/lustre/miifs01/project/zdvresearch/mrahmanp/cargo_dep/install

# Set the source directory for each dependency
JSONC_SRC=/lustre/miifs01/project/zdvresearch/mrahmanp/cargo_dep/git/json-c
ARGOBOTS_SRC=/lustre/miifs01/project/zdvresearch/mrahmanp/cargo_dep/git/argobots
LIBFABRIC_SRC=/lustre/miifs01/project/zdvresearch/mrahmanp/cargo_dep/git/libfabric
CEREAL_SRC=/lustre/miifs01/project/zdvresearch/mrahmanp/cargo_dep/git/cereal
YAMLCPP_SRC=/lustre/miifs01/project/zdvresearch/mrahmanp/cargo_dep/git/yaml-cpp

# Set the build report path
REPORT_PATH="/lustre/miifs01/project/zdvresearch/mrahmanp/cargo_dep/install/build_report.txt"

echo "Build report:" > "$REPORT_PATH"

# Build and install json-c
cd $JSONC_SRC
mkdir build && cd build
echo "Start JSON" >> "$REPORT_PATH"
cmake .. -DCMAKE_INSTALL_PREFIX=$PREFIX
make -j$(nproc)
if [ $? -eq 0 ]; then
    make install
    echo "json-c build successful" >> "$REPORT_PATH"
else
    echo "json-c build failed" >> "$REPORT_PATH"
fi

# Build and install Cereal
cd $CEREAL_SRC
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$PREFIX
make -j$(nproc)
if [ $? -eq 0 ]; then
    make install
    echo "Cereal build successful" >> "$REPORT_PATH"
else
    echo "Cereal build failed" >> "$REPORT_PATH"
fi


# Build and install yaml-cpp
cd $YAMLCPP_SRC
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$PREFIX
make -j$(nproc)
make install
if [ $? -eq 0 ]; then
    make install
    echo "yaml-cppbuild successful" >> "$REPORT_PATH"
else
    echo "yaml-cpp build failed" >> "$REPORT_PATH"
fi
# Build and install Argobots
cd $ARGOBOTS_SRC
./autogen.sh
mkdir build && cd build
../configure --prefix=$PREFIX
make -j$(nproc)
if [ $? -eq 0 ]; then
    make install
    echo "Argobots build successful" >> "$REPORT_PATH"
else
    echo "Argobots build failed" >> "$REPORT_PATH"
fi
    
# Build and install libfabric
cd $LIBFABRIC_SRC
./autogen.sh
mkdir build && cd build
../configure --prefix=$PREFIX --disable-verbs
make -j$(nproc)
if [ $? -eq 0 ]; then
    make install
    echo "libfabric build successful" >> "$REPORT_PATH"
else
    echo "libfabric build failed" >> "$REPORT_PATH"

fi

# Build and install Mercury
cd $MERCURY_SRC
mkdir build && cd build
ccmake ..  
make -j$(nproc)
if [ $? -eq 0 ]; then
    make install
    echo "Mercury build successful" >> "$REPORT_PATH"
else
    echo "Mercury build failed" >> "$REPORT_PATH"
fi


# Build and install Margo
cd $MARGO_SRC
./prepare.sh
mkdir build && cd build
../configure --prefix=$PREFIX PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig:$PREFIX/lib64/pkgconfig CFLAGS="-g -Wall"
make -j$(nproc)
if [ $? -eq 0 ]; then
    make install
    echo "Margo build successful" >> "$REPORT_PATH"
else
    echo "Margo build failed" >> "$REPORT_PATH"
fi


# Build and install Thallium
cd $THALLIUM_SRC
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_PREFIX_PATH=$PREFIX
make -j$(nproc)
if [ $? -eq 0 ]; then
    make install
    echo "Thallium build successful" >> "$REPORT_PATH"
else
    echo "Thallium build failed" >> "$REPORT_PATH"
fi



