#!/bin/bash

tsc -p ./tsconfig.json

TSC_EXIT_CODE="$?"
if [ "$TSC_EXIT_CODE" -ne 0 ]; then
  echo "Error: TypeScript compilation failed with exit code $TSC_EXIT_CODE"
  exit $TSC_EXIT_CODE
fi

echo "TypeScript compilation completed successfully."

# Create the build directory if it doesn't exist
mkdir -p ./build/public
if [ "$?" -ne 0 ]; then
  echo "Error: Failed to create directory ./build/public"
  exit 1
fi

# Copy files from ./src/public to ./build/public
cp -r ./src/public/* ./build/public

if [ "$?" -ne 0 ]; then
  echo "Error: Failed to copy files from ./src/public to ./build/public"
  exit 1
fi

echo "Post-build script executed successfully."