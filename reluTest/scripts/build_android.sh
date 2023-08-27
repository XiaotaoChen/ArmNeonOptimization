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

##### geenrate disassembly files
DISASSEMBLY_FILES_PATH=${basepath}/disassemble_files
${BUILD_ANDROID_NDK_HOME}/toolchains/aarch64-linux-android-4.9/prebuilt/darwin-x86_64/bin/aarch64-linux-android-objdump \
    -d  ${BUILD_DIR}/CMakeFiles/assemblyReluTest.dir/assemblyReluTest.cpp.o \
    > ${DISASSEMBLY_FILES_PATH}/assemblyReluTest_${ABI}.txt

adb shell "rm -rf ${DEPLOY_DIR}"
adb shell "mkdir -p ${DEPLOY_DIR}"

##### run relu experiments
adb push ${BUILD_DIR}/assemblyReluTest ${DEPLOY_DIR}
adb shell "cd ${DEPLOY_DIR}; ./assemblyReluTest"



