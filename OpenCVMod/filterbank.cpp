// --------Filterbank source File-------- //

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/opencv.hpp>
#include "opencv2/core/core.hpp"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm> // Maybe fix DescriptorExtractor doesn't have a member 'create'

using namespace cv;
using namespace std;

const float PI = 3.1415;

#define filterBankDEBUG 0
#define ERR(msg) printf("\n\nERROR!: %s Line %d\nExiting.\n\n", msg, __LINE__);

void filterDEBUG(string msg, double in){
  if(filterBankDEBUG){
    cout << msg;
    if(in != 0){
      cout << in;
    }
    cout << "\n";
  }
}

//response = np.exp(-x ** 2 / (2. * sigma ** 2))
void func1(float *response, float *lengths, float sigma, int size) {
    for(int i = 0; i < size; i++)
        response[i] = exp(- lengths[i] * lengths[i] / (2 * sigma * sigma));
}

//response = -response * x
void func2(float *response, float *lengths, int size) {
    for(int i = 0; i < size; i++)
        response[i] = -response[i] * lengths[i];
}

//response = response * (x ** 2 - sigma ** 2)
void func3(float *response, float *lengths, float sigma, int size) {
    for(int i = 0; i < size; i++)
        response[i] = response[i] * (lengths[i] * lengths[i] - sigma * sigma);
}

// response /= np.abs(response).sum()
void normalize(float *response, int size) {
    float summ = 0;
    for(int i = 0; i < size; i++)
        summ += std::abs(response[i]);
    for(int i = 0; i < size; i++)
        response[i] /= summ;
}

void make_gaussian_filter(float *response, float *lengths, float sigma, int size, int order=0) {
    assert(order <= 2);//, "Only orders up to 2 are supported"

    // compute unnormalized Gaussian response
    func1(response, lengths, sigma, size);
    if (order == 1)
        func2(response, lengths, size);
    else if (order == 2)
        func3(response, lengths, sigma, size);

    normalize(response, size);
}

void getX(float *xCoords, Point2f* pts, int size) {
    for(int i = 0; i < size; i++)
        xCoords[i] = pts[i].x;
}

void getY(float *yCoords, Point2f* pts, int size) {
    for(int i = 0; i < size; i++)
        yCoords[i] = pts[i].y;
}

void multiplyArrays(float *gx, float *gy, float *response, int size) {
    for(int i = 0; i < size; i++)
        response[i] = gx[i] * gy[i];
}

void makeFilter(float scale, int phasey, Point2f* pts, float *response, int size) {
    float xCoords[size];
    float yCoords[size];
    getX(xCoords, pts, size);
    getY(yCoords, pts, size);

    float gx[size];
    float gy[size];
    make_gaussian_filter(gx, xCoords, 3 * scale, size);
    make_gaussian_filter(gy, yCoords, scale, size, phasey);
    multiplyArrays(gx, gy, response, size);
    normalize(response, size);
}

void createPointsArray(Point2f *pointsArray, int radius) {
    int index = 0;
    for(int x = -radius; x <= radius; x++)
        for(int y = -radius; y <= radius; y++)
        {
            pointsArray[index] = Point2f(x,y);
            index++;
        }
}

void rotatePoints(float s, float c, Point2f *pointsArray, Point2f *rotatedPointsArray, int size) {
    for(int i = 0; i < size; i++)
    {
        rotatedPointsArray[i].x = c * pointsArray[i].x - s * pointsArray[i].y;
        rotatedPointsArray[i].y = s * pointsArray[i].x - c * pointsArray[i].y;
    }
}

void computeLength(Point2f *pointsArray, float *length, int size) {
    for(int i = 0; i < size; i++)
        length[i] = sqrt(pointsArray[i].x * pointsArray[i].x + pointsArray[i].y * pointsArray[i].y);
}

void toMat(float *edgeThis, Mat &edgeThisMat, int support) {
    edgeThisMat = Mat::zeros(support, support, CV_32FC1);
    float* nextPts = (float*)edgeThisMat.data;
    for(int i = 0; i < support * support; i++)
    {
        nextPts[i] = edgeThis[i];
    }
}

void makeRFSfilters(vector<Mat>& edge, vector<Mat>& bar, vector<Mat>& rot, vector<float> &sigmas, int n_orientations, int radius=24) {
    int support = 2 * radius + 1;
    int size = support * support;
    Point2f orgpts[size];
    createPointsArray(orgpts, radius);

    for(uint sigmaIndex = 0; sigmaIndex < sigmas.size(); sigmaIndex++)
        for(int orient = 0; orient < n_orientations; orient++)
        {
            float sigma = sigmas[sigmaIndex];
            //Not 2pi as filters have symmetry
            float angle = PI * orient / n_orientations;
            float c = cos(angle);
            float s = sin(angle);
            Point2f rotpts[size];
            rotatePoints(s, c, orgpts, rotpts, size);
            float edgeThis[size];
            makeFilter(sigma, 1, rotpts, edgeThis, size);
            float barThis[size];
            makeFilter(sigma, 2, rotpts, barThis, size);
            Mat edgeThisMat;
            Mat barThisMat;
            toMat(edgeThis, edgeThisMat, support);
            toMat(barThis, barThisMat, support);

            edge.push_back(edgeThisMat);
            bar.push_back(barThisMat);
        }

    float length[size];
    computeLength(orgpts, length, size);

    float rotThis1[size];
    float rotThis2[size];
    make_gaussian_filter(rotThis1, length, 10, size);
    make_gaussian_filter(rotThis2, length, 10, size, 2);

    Mat rotThis1Mat;
    Mat rotThis2Mat;
    toMat(rotThis1, rotThis1Mat, support);
    toMat(rotThis2, rotThis2Mat, support);
    rot.push_back(rotThis1Mat);
    rot.push_back(rotThis2Mat);
}

void  apply_filterbank(Mat &img, vector<vector<Mat> >filterbank, vector<vector<Mat> > &response, int n_sigmas, int n_orientations) {
    response.resize(3);
    vector<Mat>& edges = filterbank[0];
    vector<Mat>& bar = filterbank[1];
    vector<Mat>& rot = filterbank[2];
    int i = 0;

 // #pragma omp parallel
 // {
        i=0;

        // Apply Edge Filters //
      //  #pragma omp for ordered
        for(int sigmaIndex = 0; sigmaIndex < n_sigmas; sigmaIndex++)
        {
          Mat newMat = Mat::zeros(img.rows, img.cols, img.type());
          for(int orient = 0; orient < n_orientations; orient++)
          {
              Mat dst;
              filter2D(img, dst,  -1, edges[i], Point( -1, -1 ), 0, BORDER_DEFAULT );
              newMat = cv::max(dst, newMat); // take maximum values
              i++;
          }
          Mat newMatUchar;
          newMat = cv::abs(newMat);
          newMat.convertTo(newMatUchar, CV_8UC1);

          //         #pragma omp ordered

          response[0].push_back(newMatUchar);
          filterDEBUG("Applying Edge Filters..", 0);
        }

        i = 0;
        // Apply Bar Filters
        //       #pragma omp for ordered
        for(int sigmaIndex = 0; sigmaIndex < n_sigmas; sigmaIndex++)
        {
          Mat newMat = Mat::zeros(img.rows, img.cols, img.type());
          for(int orient = 0; orient < n_orientations; orient++)
          {
              Mat dst;
              filter2D(img, dst,  -1 , bar[i], Point( -1, -1 ), 0, BORDER_DEFAULT );
              newMat = max(dst, newMat);
              i++;
          }
          Mat newMatUchar;
          newMat = cv::abs(newMat);
          newMat.convertTo(newMatUchar, CV_8UC1);

          //         #pragma omp ordered
          response[1].push_back(newMatUchar);
          filterDEBUG("Applying Bar Filters..", 0);
        }

        // Apply Gaussian and LoG Filters
        //       #pragma omp for ordered
        for(uint i = 0; i < 2; i++)
        {
          Mat newMat = Mat::zeros(img.rows, img.cols, img.type());
          Mat dst;
          filter2D(img, dst,  -1 , rot[i], Point( -1, -1 ), 0, BORDER_DEFAULT );
          newMat = max(dst, newMat);
          Mat newMatUchar;
          newMat = cv::abs(newMat);
          newMat.convertTo(newMatUchar, CV_8UC1);
          //         #pragma omp ordered
          response[2].push_back(newMatUchar);
          filterDEBUG("Applying Gaussian Filters..", 0);
        }
        //     }
  filterDEBUG("Leaving Filterbank", 0);
}

void createFilterbank(vector<vector<Mat> > &filterbank, int &n_sigmas, int &n_orientations){
  vector<float> sigmas;
  sigmas.push_back(1);
  sigmas.push_back(2);
  sigmas.push_back(4);
  n_sigmas = sigmas.size();
  n_orientations = 6;

  vector<Mat > edge, bar, rot;
  makeRFSfilters(edge, bar, rot, sigmas, n_orientations);

  // Store created filters in fitlerbank 2d vector<Mat>
  filterbank.push_back(edge);
  filterbank.push_back(bar);
  filterbank.push_back(rot);
}

// Aggregate Images
void aggregateImg(int num, double alpha, Mat &aggImg, Mat input) {
  double beta = 1.0 - alpha;
  if (num == 0) {
    input.copyTo(aggImg);
  } else {
    addWeighted(aggImg, beta , input, alpha, 0.0, aggImg, -1);
  }
}

int filterHandle(Mat &in, Mat &out, vector<vector<Mat> > filterbank, int n_sigmas, int n_orientations){

  vector<vector<Mat> > response;
  filterDEBUG("Before Applying Filterbank",0);
  Mat grey, tmp;
  cvtColor(in, grey, CV_BGR2GRAY);
  equalizeHist(grey,tmp);
  apply_filterbank(tmp, filterbank, response, n_sigmas, n_orientations);
  if(response.empty()){
    ERR("Filtered image response is empty.");
     exit(0);
  }
  // Aggrgate filter responses to single img
  int flag = 0;
  double alpha = 0.5;
  for(int i=0;i<response.size();i++){
    for(int j = 0; j < response[i].size(); j++){
        // Aggregate for Texton Dictionary
        aggregateImg(flag, alpha, out, response[i][j]);
        alpha *= 0.5;
      flag = 1;
    }
  }
  assert(out.size() == response[0][0].size());
  return 1;
}
