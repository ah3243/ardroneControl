              /////////////////////////////////////////////////////////////////////////////////
              //                                SINGLE IMAGE TESTING MODULE                  //
              /////////////////////////////////////////////////////////////////////////////////
              // This module will:                                                           //
              // 1. take in a single image.                                                  //
              // 2. Remap to remove distortion.                                              //
              // 3. process it (normalise, filter it).                                       //
              // 4. Crop it into 6 segments.                                                 //
              // 5. Classifiy each segment against stored models.                            //
              // 6. determine from the output what action should be taken                    //
              //     - Move forward                                                          //
              //     - Turn Left                                                             //
              //     - Turn Right                                                            //
              //     - Stop (to collect second image/validate target with higher resolution) //
              //     - Drop Ordinance (If target Validated)                                  //
              // 7. Return chosen option.                                                    //
              /////////////////////////////////////////////////////////////////////////////////

#include <opencv2/highgui/highgui.hpp> // Needed for HistCalc
#include <opencv2/imgproc/imgproc.hpp> // Needed for HistCalc
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <iostream> // General io
#include <stdio.h> // General io
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <assert.h>
#include <chrono>  // time measurement
#include <thread>  // time measurement
#include <map>

#include "filterbank.h" // Filterbank Handling Functions

using namespace cv;
using namespace std;

#define ERR(msg) fprintf(stderr,"\n\nERROR!: %s Line %d\nExiting.\n\n", msg, __LINE__);

#define CHISQU_MAX_threshold 6
#define CHISQU_DIS_threshold 0

#define showImgs 1

// DEBUG FLAGS
#define DEBUG_SCALE 0
#define DEBUG_SHOWSEGMENTS 0
#define DEBUG_SEGMENTS 0

// Structure to hold Image segment predictions
struct segStruct{
  // Top match for each segment position
  string TopLeft;
  string TopMiddle;
  string TopRight;
  string BottomLeft;
  string BottomMiddle;
  string BottomRight;
} segResStruct;

// Get Distortion Coefficients and Camera Matrix from file
void getCalVals(Mat &cameraMatrix, Mat &distCoeffs, double &image_Width, double &image_Height){
  FileStorage fs("/home/james-tt/Desktop/MyFilterbankCode/imageRemapping/calFile.xml", FileStorage::READ); // Get config Data from other dir
  fs["Camera_Matrix"] >> cameraMatrix;
  fs["Distortion_Coefficients"] >> distCoeffs;
  // fs["image_Width"] >> image_Width;
  // fs["image_Height"] >> image_Height;
  image_Width=640;
  image_Height=360;
  fs.release();
}

// Remap input image to remove distortion in line with saved calibration values
Mat remapImg(Mat input){
  // Create variables
  Mat map1, map2;
  Mat cameraMatrix, distCoeffs;
  double image_Width, image_Height;
  getCalVals(cameraMatrix, distCoeffs, image_Width, image_Height);
  // Set size
  Size imageSize(image_Width, image_Height);

  // Create maps
  initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
    getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 0, imageSize, 0),
    imageSize, CV_16SC2, map1, map2);

  // Create remap variables
  Mat view, rview;

  // Create display Windows
  namedWindow("before", 0);
  namedWindow("after", 0);

  view = input;
  // Remap image
  remap(view, rview, map1, map2, INTER_LINEAR);
  if(showImgs){
   imshow("before", view);
   imshow("after", rview);
   waitKey(500); // Wait for 500ms
  }
  return rview;
}

// Scale input image depending on input scale
void scaleTstImg(Mat in, Mat &out, int scale){
  double endW[] = {1280, 1152, 1024, 896, 768, 640, 512, 384, 256, 128};
  double endH[] = {720, 648, 576, 504, 432, 360, 288, 216, 144, 72};
  double effScale;

  effScale = (endW[scale]/in.cols);
  // Validate height and width match allowed pairs
  if((endH[scale]/in.rows)!= effScale){
    ERR("The scaling variables did not match.");
    exit(-1);
  }
  resize(in, out, out.size(), effScale, effScale, INTER_AREA);
  if(DEBUG_SCALE){
    cerr << "SCALE_DEBUG:- effScale: " << effScale << " outSize: " << out.size() << endl;
  }
}
// Flatten Matrix to single column for histogram creation
Mat flattenMat(Mat in){
  Mat points(in.rows*in.cols, 1,CV_32F);
  int cnt = 0;
//  cout << "inside. These are the rows: " <<  in.rows << " and cols: " << in.cols  << endl;
  for(int i =0;i<in.cols;i++){
//    cout << "outer loop: " << i << endl;
    for(int j=0;j<in.rows;j++){
      points.at<float>(cnt, 0) = in.at<Vec3b>(i,j)[0];
      cnt++;
    }
  }
  return points;
}

// Crop Filter responses into segments
void segmentImages(vector<Mat>& out, Mat in, int cropsize, int overlap){
  //  int colstart =0, rowstart=0;

  // find the number of possible segments, then calculate gap around these
  int colspace = (in.cols -((in.cols/cropsize)*cropsize))/2;
  int rowspace = (in.rows -((in.rows/cropsize)*cropsize))/2;

  // int colspace =0; // For ease of testing
  // int rowspace =0; // For ease of testing

  // Make sure manual offset and cropsize are compatible with imagesize
  if((cropsize+rowspace)>in.rows || (cropsize+colspace)>in.cols){
    ERR("cropsize larger than input image. Exiting");
    exit(1);
  }

  stringstream ss;
  ss << "entering segmentImg this is the img rows: ";
  ss << in.rows << " cols: " << in.cols;
  if(DEBUG_SEGMENTS){
      cerr << "DEBUG_SEGMENTS: " << ss.str() << endl;
  }

  // // if no overlap and unable to make >1 segment, place segment in center of screen
  // if(overlap==0&&colspace>0){
  //   colstart= colspace;
  // }
  // if(overlap==0&&rowspace>0){
  //   rowstart= rowspace;
  // }

  // Extract the maximum Number of full Segments from the image
  for(int i=colspace;i<(in.cols-cropsize);i+=(cropsize-overlap)){
    for(int j=rowspace;j<(in.rows-cropsize);j+=cropsize){
      Mat tmp = Mat::zeros(cropsize,cropsize,CV_32FC1);
      Mat normImg = in(Rect(i, j, cropsize, cropsize));
      if(DEBUG_SHOWSEGMENTS){
       imshow("SegImg", normImg);
       cout << "Press any key to change segment." << endl;
       waitKey(0);
      }
      tmp = flattenMat(normImg);
      out.push_back(tmp);
    }
  }
  ss.str("");
  ss << "This is the number of segments: " << out.size() << " and the average cols: " << out[0].cols;
  if(DEBUG_SEGMENTS){
      cerr << "DEBUG_SEGMENTS: " << ss.str() << endl;
  }
}
// Convert Binvector into array for use in histogram creation
void vecToArray(vector<float> v, float* m){
  int size = v.size();
  for(int i=0;i<size;i++){
    m[i] = v[i];
  }
}

// Cluster the segments filter response
Mat clusterImg(Mat in, map<string, int> params, map<string, double> paramsDB){
  // Extract Parameters
  double kmeansEpsilon = paramsDB["kmeansEpsilon"];
  double kmeansIteration = paramsDB["kmeansIteration"];
  int numClusters = params["numClusters"];
  int clsAttempts = params["kmeansAttempts"];
  int flags = params["flags"];

  TermCriteria clsTc(TermCriteria::MAX_ITER, kmeansIteration, kmeansEpsilon);

  // Create Kmeans Trainier
  BOWKMeansTrainer novelTrainer(numClusters, clsTc, clsAttempts, flags);
  novelTrainer.add(in);
  return novelTrainer.cluster();
}
// Supplant cluster centres with the closest texton from the stored dictionary
void textonMatcher(Mat& clus, Mat dictionary, vector<double>& disVec){
  if(clus.empty() || dictionary.empty()){
    ERR("Texton Find inputs were empty.");
    exit(-1);
  }
  // Loop through input centers
  for(int h=0;h<clus.rows;h++){
    float distance = 0.0, nearest = 0.0;

    distance = abs(dictionary.at<float>(0,0) - clus.at<float>(h,0));
    nearest = dictionary.at<float>(0,0);

    // Compare current centre with all values in texton dictionary
    for(int k = 0; k < dictionary.rows; k++){
      if(abs(dictionary.at<float>(k,0) - clus.at<float>(h,0)) < distance){
        nearest = dictionary.at<float>(k,0);
        distance = abs(dictionary.at<float>(k,0) - clus.at<float>(h,0));
      }
    }
    // Replace input Center with closest Texton Center
    clus.at<float>(h,0) = nearest;
    disVec.push_back(distance);
  }
}

// Take the average of each classes iterations and return the best match
string avgIterResults(map<string, vector<double> > results){
  // Store best Match distance and class name
  double bestMatch = DBL_MAX;
  string match = "noMatch";

  // Loop through each class
  for(auto const itrRes:results){
    // Calculate the mean of iteration results for each class
    double sum = accumulate(itrRes.second.begin(), itrRes.second.end(), 0.0);
    double mean = sum/itrRes.second.size();
    fprintf(stderr,"%s : %f\n",itrRes.first.c_str(), mean);

    // Store the best Match
    if(mean<bestMatch){
      bestMatch=mean;
      match = itrRes.first;
    }
  }
  fprintf(stderr,"This was the best match: %s distance: %f\n",match.c_str(), bestMatch);
  return match;
}

// Import TextonDictionary
void importTexDict(Mat &dictionary, vector<float> &m){
  // Load TextonDictionary
  FileStorage fs("/home/james-tt/Desktop/MyFilterbankCode/multiDimen/TESTINGCMAKE/build/dictionary.xml",FileStorage::READ);
  if(!fs.isOpened()){
    ERR("Unable to open Texton Dictionary.");
    exit(-1);
  }
  fs["vocabulary"] >> dictionary;
  fs["bins"] >> m;
  fs.release();
}
// Get Class Models
void importClassModels(map<string, vector<Mat> >& savedClassHist){
  // Load in Class Histograms(Models)
  FileStorage fs3("/home/james-tt/Desktop/MyFilterbankCode/multiDimen/TESTINGCMAKE/build/models.xml", FileStorage::READ);
  FileNode fn = fs3["classes"];
  if(fn.type() == FileNode::MAP){

    // Create iterator to go through all the classes
    for(FileNodeIterator it = fn.begin();it != fn.end();it++){
      string clsNme = (string)(*it)["Name"];
      savedClassHist[clsNme];

      // Create node of current Class
      FileNode clss = (*it)["Models"];
      // Iterate through each model inside class, saving to map
      for(FileNodeIterator it1  = clss.begin();it1 != clss.end();it1++){
        FileNode k = *it1;
        Mat tmp;
        k >> tmp;
        savedClassHist[clsNme].push_back(tmp);
      }
    }
    fs3.release();
  }else{
    ERR("Class file was not map.");
    exit(-1);
  }
}

// Store the best match for each segment in SegResults structure
void segResults(string match, int iteration){
  switch (iteration){
  case 0:
  // Top Left segment
  segResStruct.TopLeft = match;
  break;
  case 1:
  // Top Middle segment
  segResStruct.TopMiddle = match;
  break;
  case 2:
  // Top Right segment
  segResStruct.TopRight = match;
  break;
  case 3:
  // Bottom Left segment
  segResStruct.BottomLeft = match;
  break;
  case 4:
  // Bottom Middle segment
  segResStruct.BottomMiddle = match;
  break;
  case 5:
  // Bottom Right segment
  segResStruct.BottomRight = match;
  break;
  default:
  ERR("'segResStruct' unknown iteration entered. Exiting.");
  exit(1);
  }
}

///////////////////////////////////////////////
//            NAV OUTPUT KEY                 //
//                                           //
//  0: GOAL. Validated Positive Result       //
//  1: Stop and Validated. Positive Results  //
//     Must be Validated                     //
//  2: Move Forward. No positive results     //
//     or positive result in front.          //
//  3: Turn Left. Positive results to left.  //
//  4: Turn Right. Positive results to right.//
//                                           //
///////////////////////////////////////////////

// Interpret Results to output navigation decision
int navOutput(int scale, string goal){
  fprintf(stderr,"This is the goal: %s\n", goal.c_str());
  // If higher resolution currently used to validate goal and goal found
  if(scale==7 && segResStruct.BottomMiddle.compare(goal)==0){
    fprintf(stderr,"\n\nGOAL CLASS FOUND\n\n");
    return 0; // Exit with successful Class find
  }
  // If not currently on validation run but goal found
  else if(segResStruct.BottomMiddle.compare(goal)==0){
    fprintf(stderr,"\nValidate Positive Result \n");
    return 1; // Stop and Validate result with higher resolution
  }else if(segResStruct.BottomLeft.compare(goal)==0){
    fprintf(stderr,"\nTurn Left\n");
    return 3; // Turn Left
  }else if(segResStruct.BottomMiddle.compare(goal)==0){
    fprintf(stderr,"\nTurn Right\n");
    return 4; // Turn Right
  }else{
    fprintf(stderr, "\nMove Forward\n");
    return 2; // Return default move forward
  }
}


int directionHandle(string imgPath, map<string, int> params, map<string, double> paramsDB, string goal){
  int scale = params["scale"];
  int cropSize = params["cropSize"];
  // Read in image
  Mat inImg = imread(imgPath,1);
  if(inImg.empty()){
    ERR("Imported image is empty. Exiting");
    exit(1);
  }

  // Remap input to remove distortion
  Mat rectImg = remapImg(inImg);

  // Scale image
  Mat scaledImg;
  scaleTstImg(rectImg, scaledImg, scale);

  // Generate and Store Filterbank
  vector<vector<Mat> > filterbank;
  int n_sigmas, n_orientations;
  createFilterbank(filterbank, n_sigmas, n_orientations);

  // Filter input Image
  Mat filteredImg;
  filterHandle(scaledImg, filteredImg, filterbank, n_sigmas, n_orientations);
//  imshow("filtered", filteredImg);

  // Crop Image
  vector<Mat> segments;
  segmentImages(segments, filteredImg, cropSize, 0);

  // Import Saved Texton Dictionary
  Mat texDict;
  vector<float> texDictbins;
  importTexDict(texDict, texDictbins);
  // Prepare Bins for histogram
  float bins[texDictbins.size()];
  vecToArray(texDictbins, bins);

  // Import Models
  map<string, vector<Mat> > savedClassModels;
  importClassModels(savedClassModels);

  // Iterate through each segment
  for(int i=0;i<segments.size();i++){
    map<string, vector<double> > tmpVals; // For storing all best matches between classes over several iterations
    int repeats = params["testRepeats"];

    // Take repeat readings of segments taking the mean of the best results for each class
    for(int k=0;k<repeats;k++){
      // Cluster Segment
      Mat imgClusters = clusterImg(segments[i], params, paramsDB);

      // Supplant cluster centers with nearest textons
      vector<double> textonDistance;
      textonMatcher(imgClusters, texDict, textonDistance);

      // Calculate Histogram of textons
      Mat out1;
      int histSize = texDictbins.size()-1;
      const float* histRange = {bins};
      calcHist(&imgClusters, 1, 0, Mat(), out1, 1, &histSize, &histRange, false, false);

      // Compare all saved histograms against novelimg
      double high = DBL_MAX, secHigh = DBL_MAX, clsHigh = DBL_MAX;
      string match, secMatch;

      // Loop through all stored Models and compare
      for(auto const ent2 : savedClassModels){
        vector<double> tmpVec;
        // Compare saved histgrams from each class
        for(int j=0;j < ent2.second.size();j++){
          Mat tmpHist = ent2.second[j].clone();
          double val = compareHist(out1,tmpHist,CV_COMP_CHISQR);

          tmpVec.push_back(val); // Push back all results (all of same class)
        }
       sort(tmpVec.begin(), tmpVec.end()); // Sort from best to worse match (for a single class)
       tmpVals[ent2.first].push_back(tmpVec[0]); // Store the best match for a class to the test image
      }
    }
    // Average results and return first and return first and second
    string bestMatch = avgIterResults(tmpVals);
    segResults(bestMatch, i);
  }
  return navOutput(scale, goal);
}


int main(int argc, char** argv){
  // Validate the number of inputs is correct
  if(argc<3){
    ERR("Incorrect number of inputs detected. Exiting.");
    exit(1);
  }

  int kmeansIteration = 100000;
  int kmeansEpsilon = 0.000001;
  int numClusters = 10; // For model and test images
  int flags = KMEANS_PP_CENTERS;

  // Print out input parameters
  fprintf(stderr,"argc: %d\n", argc);
  fprintf(stderr,"These are the inputs: \n");

  for(int a =0;a<argc;a++){
    cerr << ", " << a << ": " << argv[a];
  }fprintf(stderr,"\n");

  string imgPath = argv[1];
  string goal = argv[4];


  map<string, int> testParams;
  map<string, double> testParamsDB;
    testParams["scale"] = atoi(argv[2]);
    testParams["cropSize"] = atoi(argv[3]);
    testParams["numClusters"] = numClusters;
    testParamsDB["kmeansIteration"] = kmeansIteration;
    testParamsDB["kmeansEpsilon"] = kmeansEpsilon;
    testParams["kmeansAttempts"] = 35;
    testParams["flags"] = flags;
    testParams["testRepeats"] = 10;
    int navOut = directionHandle(imgPath, testParams, testParamsDB, goal);
    fprintf(stderr,"\n\nOutput Value..%d\n", navOut);

    fprintf(stdout, "%d",navOut); // Output return value to stdout
    return 0;
}
