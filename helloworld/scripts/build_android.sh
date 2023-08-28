#!/usr/bin/env bash

set -e
# set -x

basepath=$(cd `dirname $0`/..; pwd)

BUILD_DIR=${basepath}/build_android
BUILD_ANDROID_NDK_HOME=$ANDROID_NDK
DEPLOY_DIR=/data/local/tmp/assembly
CMAKE_PATH="cmake"

# ABI="armeabi-v7a"
ABI="arm64-v8a"
ANDROID_API_LEVEL=android-21
ANDROID_TOOLCHAIN=clang

rm -rf ${BUILD_DIR}
if [[ ! -d ${BUILD_DIR} ]]; then
    mkdir -p ${BUILD_DIR}
fi

cd ${BUILD_DIR}
${CMAKE_PATH} \
-DCMAKE_TOOLCHAIN_FILE=${BUILD_ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake \
-DANDROID_NDK=${BUILD_ANDROID_NDK_HOME} \
-DANDROID_ABI=${ABI} \
-DANDROID_ARM_NEON=ON \
-DANDROID_NATIVE_API_LEVEL=${ANDROID_API_LEVEL} \
-DANDROID_TOOLCHAIN=${ANDROID_TOOLCHAIN} \
../

make all -j4
