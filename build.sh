#!/bin/bash
export MODEL_VIEWER_ROOT=$(pwd)    
# if [ -z "$MODEL_VIEWER_ROOT" ]
# then
#     echo "MODEL_VIEWER_ROOT is not set"
#     exit 1
# fi

# Generate build script
# cd $MODEL_VIEWER_ROOT && \

if [ ! -d build ]; then
    mkdir build
fi
cd build || exit

cmake ../ -DCMAKE_INSTALL_PREFIX=$MODEL_VIEWER_ROOT

# Build and install the program
# make -j4 && \
# make install && \

cmake --build . --config Debug

if [ -f "./Debug/model_viewer.exe" ]; then
    ./Debug/model_viewer.exe
elif [ -f "./model_viewer" ]; then
    ./model_viewer
else
    echo "Error: model_viewer executable not found"
    exit 1
fi
# Run the program
# cd ../bin && \
# ./model_viewer


# mkdir build
# cd build
# cmake ../ -DCMAKE_INSTALL_PREFIX="/mnt/c/GitHub/gltf_viewer/build"
# make -j4
# make install
# cd bin/
# ./model_viewer