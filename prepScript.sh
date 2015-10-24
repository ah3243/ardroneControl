
#!/bin/bash/

# Change the CD to Build directory
cd ../multiDimen/TESTINGCMAKE/build

#rm -rf *
# Build
cmake ..
make

#Move to Image Folder
cd ../../../ARDRONE/

echo
echo This is the current folder
pwd
echo
