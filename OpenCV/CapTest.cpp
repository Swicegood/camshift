#include<opencv2/opencv.hpp>
#include<iostream>
using namespace std;
using namespace cv;
int main() {
    VideoCapture cam;
    while (!cam.open(0))cerr << "failed to open cam" << endl;
    namedWindow("test");
    while (1) {
        Mat img;
        cam >> img;
        imshow("test", img);
  //      if (waitKey() == 27)break;
    }
    destroyWindow("test");
}