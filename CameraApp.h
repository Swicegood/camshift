#pragma once
#ifndef CAMERAAPP_H
#define CAMERAAPP_H

#include <opencv2/opencv.hpp>

void detectAndDisplay(cv::Mat frame);
int runCameraApp(int camera_device, int resh, int resv);

#endif // CAMERAAPP_H