#!/bin/bash
export MODEL_VIEWER_ROOT="C:\GitHub\gltf_viewer"    
if [ -z "$MODEL_VIEWER_ROOT" ]
then
    echo "MODEL_VIEWER_ROOT is not set"
    exit 1
fi

# Generate build script
cd $MODEL_VIEWER_ROOT && \
if [ ! -d build ]; then
    mkdir build
fi
cd build && \
cmake ../ -DCMAKE_INSTALL_PREFIX=$MODEL_VIEWER_ROOT && \
cmake ../ -DCMAKE_INSTALL_PREFIX="C:\GitHub\gltf_viewer" && \

# Build and install the program
make -j4 && \
make install && \

# Run the program
cd ../bin && \
./model_viewer


mkdir build
cd build
cmake ../ -DCMAKE_INSTALL_PREFIX="/mnt/c/GitHub/gltf_viewer/build"
make -j4
make install
cd bin/
./model_viewer