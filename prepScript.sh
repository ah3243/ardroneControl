
#!/bin/bash/

# Change the CD to Build directory
cd ~/Desktop/MyFilterbankCode/TESTINGCMAKE/build

#rm -rf *
# Build
cmake ..
make


#Move to Image Folder
cd ~/Desktop/MyFilterbankCode/ARDRONE/


echo
echo This is the current folder
pwd
echo
wait # wait for all processes to finish before changing directory
