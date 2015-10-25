#!/bin/bash

  # Function to output to stderr
  DEBUG=0
  if [ $DEBUG -eq 1 ]
  then
    echoerr(){ echo "$@" 1>&2; }
  else
    echoerr(){ echo ; }
  fi

  #---------Program and Parameters----------#
  #GOAL CLASS#
  goal="grass"
  #Image Scale
  scale=8
  #Corresponding Segment Size to maintain 6 segments
  if [ "$scale" -eq 9 ]
  then
      cropSize=35
  elif [ "$scale" -eq 8 ]
  then
      cropSize=70
  elif [ "$scale" -eq 7 ]
  then
      cropSize=105
  fi

  # Check for an image path in input parameters, else use stock images
  if [ "$#" -ne 1 ]; then
    echoerr no imageLocation detected, using testimages.
    imgLocation="../../TESTIMAGES/grass.jpg"
  else
    echoerr input image path is: "$1"
    imgLocation="$1"
  fi

  # 1:"flag" 2:"image path" 3:"scale" 4:"cropSize" 5:"goal"
 echo $(OpenCVMod/build/singleImgEval $imgLocation $scale $cropSize $goal)
