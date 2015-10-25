
#!/bin/bash/

# Change the CD to Build directory
cd OpenCVMod/build

#rm -rf *
# Build
cmake ..
make

#Move to Image Folder
cd ../../

echo
echo This is the current folder
pwd
echo
