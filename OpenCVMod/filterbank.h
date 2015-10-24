
#ifndef Filterbank
#define Filterbank

void filterDEBUG(std::string msg, double in);
void func1(float *response, float *lengths, float sigma, int size);
void func2(float *response, float *lengths, int size);
void func3(float *response, float *lengths, float sigma, int size);
void normalize(float *response, int size);
void make_gaussian_filter(float *response, float *lengths, float sigma, int size, int order=0);
void getX(float *xCoords, cv::Point2f* pts, int size);
void getY(float *yCoords, cv::Point2f* pts, int size);
void multiplyArrays(float *gx, float *gy, float *response, int size);
void makeFilter(float scale, int phasey, cv::Point2f* pts, float *response, int size);
void createPointsArray(cv::Point2f *pointsArray, int radius);
void rotatePoints(float s, float c, cv::Point2f *pointsArray, cv::Point2f *rotatedPointsArray, int size);
void computeLength(cv::Point2f *pointsArray, float *length, int size);
void toMat(float *edgeThis, cv::Mat &edgeThisMat, int support);
void makeRFSfilters(std::vector<cv::Mat>& edge, std::vector<cv::Mat>& bar, std::vector<cv::Mat>& rot, std::vector<float> &sigmas, int n_orientations, int radius=24);
void apply_filterbank(cv::Mat &img, std::vector<std::vector<cv::Mat> >filterbank, std::vector<std::vector<cv::Mat> > &response, int n_sigmas, int n_orientations);
void createFilterbank(std::vector<std::vector<cv::Mat> > &filterbank, int &n_sigmas, int &n_orientations);
void aggregateImg(int num, double alpha, cv::Mat &aggImg, cv::Mat input);
int filterHandle(cv::Mat &in, cv::Mat &out, std::vector<std::vector<cv::Mat> > filterbank, int n_sigmas, int n_orientations);

#endif
