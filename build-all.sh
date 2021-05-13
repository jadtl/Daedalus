#!/bin/bash
# Build script for rebuilding everything
set echo on

echo "Building everything..."


pushd engine
source build.sh
popd

ERROR_LEVEL=$?
if [ $ERROR_LEVEL -ne 0 ]
then
echo "Error:"$ERROR_LEVEL && exit
fi

pushd testbed
source build.sh
popd
ERROR_LEVEL=$?
if [ $ERROR_LEVEL -ne 0 ]
then
echo "Error:"$ERROR_LEVEL && exit
fi

echo "All assemblies built successfully."