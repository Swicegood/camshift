
#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <thread>
#include <stdio.h>


//using namespace std;
using namespace cv;
void detectAndDisplay(Mat frame);
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
Rect roi;

auto tp1 = std::chrono::system_clock::now();
auto tp2 = std::chrono::system_clock::now();



bool done = true;

int main(int argc, const char** argv)
{
    
    CommandLineParser parser(argc, argv,
        "{help h||}"
        "{face_cascade|data/haarcascades/haarcascade_frontalface_alt.xml|Path to face cascade.}"
        "{eyes_cascade|data/haarcascades/haarcascade_eye_tree_eyeglasses.xml|Path to eyes cascade.}"
        "{camera|0|Camera device number.}");
    parser.about("\nThis program demonstrates using the cv::CascadeClassifier class to detect objects (Face + eyes) in a video stream.\n"
        "You can use Haar or LBP features.\n\n");
    parser.printMessage();
    String face_cascade_name = samples::findFile(parser.get<String>("face_cascade"));
    String eyes_cascade_name = samples::findFile(parser.get<String>("eyes_cascade"));
    //-- 1. Load the cascades
    if (!face_cascade.load(face_cascade_name))
    {
        std::cout << "--(!)Error loading face cascade\n";
        return -1;
    };
    if (!eyes_cascade.load(eyes_cascade_name))
    {
        std::cout << "--(!)Error loading eyes cascade\n";
        return -1;
    };
    int camera_device = parser.get<int>("camera");

    //Control for speed
    int v = 165;
    namedWindow("Capture - Face detection");
    createTrackbar("Speed", "Capture - Face detection", &v, 254, NULL);

   
    VideoCapture capture;
    //-- 2. Read the video stream
    capture.open(camera_device);
    if (!capture.isOpened())
    {
        std::cout << "--(!)Error opening video capture\n";
        return -1;
    }
    VideoWriter writer(
        "appsrc ! videoconvert ! video/x-raw,format=YUY2,width=640,height=480,framerate=30/1 ! jpegenc ! rtpjpegpay ! udpsink host=127.0.0.1 port=5000",
        CAP_GSTREAMER,
        0, // four cc
        30, //fps
        Size(640, 480),
        true); //isColor
    if (!writer.isOpened()) {
        std::cerr << "VideoWriter not opened" << std::endl;
        exit(-1);
    }
    Mat frame;
    int i = 0; 
    Mat z_frame, z_frame2;
    roi = Rect(0, 0, frame.cols, frame.rows);
    Rect cur_roi(0, 0, frame.cols, frame.rows);
    float dx,dy,dw,D=dx=dy=dw=0;
    bool zoom_reached = true;
    Rect target_roi;
    float ar = (float(frame.rows) / float(frame.cols));
        while (capture.read(frame))
        {
        //Apsect ratio
        float ar = (float(frame.rows) / float(frame.cols));
        // Handle Timing
        tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();
            
        if (frame.empty())
        {
            std::cout << "--(!) No captured frame -- Break!\n";
            break;
        }
        //-- 3. Apply the classifier to the frame
        if (i == 0)
            cur_roi = Rect(0, 0, frame.cols, frame.rows);


        if (done && zoom_reached) {  //face detected and roi created. Ready to zoom in
            target_roi = roi;
            //Start panning and zooming gradually frame by frame
            dx=(target_roi.x - cur_roi.x);
            dy=(target_roi.y - cur_roi.y);
            dw = (target_roi.width - cur_roi.width); //Change in width
            D = sqrt((dx * dx) + (dy * dy));
            if (((D < 90) && (dw < 60)) || v ==0) {
                target_roi = cur_roi;
            }

            done = false;
        }

        if (i % 90 == 0)   // Run face detection code only occasionally to save CPU cycles
            std::thread(detectAndDisplay,frame).detach(); 
       
        dx = (target_roi.x - cur_roi.x);
        dy = (target_roi.y - cur_roi.y);
        dw = (target_roi.width - cur_roi.width); //Change in width
        
        
        if (!target_roi.empty() && !cur_roi.empty()) {


            cur_roi.x = (dx / (255-v)) >= 0 ? (cur_roi.x) + ceil(dx / (255-v)) : (cur_roi.x) + floor(dx / (255-v));
            if (cur_roi.x < 0)
                cur_roi.x = 0;
            if (cur_roi.x > frame.cols)
                cur_roi.x = frame.cols;

            cur_roi.y = (dy / (255-v)) >= 0? (cur_roi.y) + ceil(dy / (255-v)) : (cur_roi.y) + floor(dy / (255-v));
            if (cur_roi.y < 0)
                cur_roi.y = 0;
            if (cur_roi.y > frame.rows)
                cur_roi.y = frame.rows;

            cur_roi.width = (dw / (255-v)) >= 0 ? ((cur_roi.width) + ceil(dw / (255-v))) : ((cur_roi.width) + floor(dw / (255-v)));
            if (cur_roi.width < 0)
                cur_roi.width = 0;
            if (cur_roi.width > frame.cols)
                cur_roi.width = frame.cols;

            cur_roi.height = floor(float(cur_roi.width) * ar);
            if (cur_roi.height + cur_roi.y > frame.rows)
                cur_roi.y = frame.rows - cur_roi.height;
                                      
            z_frame = frame(cur_roi);  // Zoomed frame
            resize(z_frame, z_frame, frame.size(), 0, 0, INTER_LINEAR); //Fill whole "window"
            imshow("Capture - Face detection", z_frame);
            z_frame = (z_frame.reshape(0, 1)); // to make it continuous     
            writer.write(z_frame);           // Write fame to Gstreamer pipeline
            if ((cur_roi.x == target_roi.x) && (cur_roi.y == target_roi.y))  {
                zoom_reached = true;
            }


        }
        if (waitKey(10) == 27)
        {
            break; // escape
        }
        i++;
    }
    return 0;
}
void detectAndDisplay(Mat frame)
{
    Mat frame_gray;
    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);
    //-- Detect faces
    std::vector<Rect> faces;
    face_cascade.detectMultiScale(frame_gray, faces);
    //for (size_t i = 0; i < faces.size(); i++)
    //{
    //    Point center(faces[i].x + faces[i].width / 2, faces[i].y + faces[i].height / 2);
    //    ellipse(frame, center, Size(faces[i].width / 2, faces[i].height / 2), 0, 0, 360, Scalar(255, 0, 255), 4);
    //    Mat faceROI = frame_gray(faces[i]);
    //    //-- In each face, detect eyes
    //    std::vector<Rect> eyes;
    //    eyes_cascade.detectMultiScale(faceROI, eyes);
    //    for (size_t j = 0; j < eyes.size(); j++)
    //    {
    //        Point eye_center(faces[i].x + eyes[j].x + eyes[j].width / 2, faces[i].y + eyes[j].y + eyes[j].height / 2);
    //        int radius = cvRound((eyes[j].width + eyes[j].height) * 0.25);
    //        circle(frame, eye_center, radius, Scalar(255, 0, 0), 4);
    //    }
    //}
    if (faces.size() == 0 || faces[0].x < 0 || faces[0].y < 0 || (faces[0].x + faces[0].width > frame.cols) || (faces[0].y + faces[0].height > frame.rows)) {

          roi = Rect(0, 0, frame.cols, frame.rows);          
    }
    else{
        int new_w(3 * faces[0].width);
        int new_h(floor(float(new_w) * (float(frame.rows) / float(frame.cols))));

        if (new_w > frame.cols)
            new_w = frame.cols;
        if (new_h > frame.rows)
            new_h = frame.rows;
        Point _center(faces[0].x + faces[0].width / 2, faces[0].y + faces[0].height / 2);
        Point crop_p(_center.x - .5 * new_w, _center.y -  .5 * new_h);
        if (crop_p.x + new_w > frame.cols)
            crop_p.x = frame.cols - new_w;
        else if (crop_p.x < 0)
            crop_p.x = 0;
        if (crop_p.y + new_h > frame.rows)
            crop_p.y = frame.rows - new_h;
        else if (crop_p.y < 0)
            crop_p.y = 0;
        //-- Show what you got    
        roi= Rect(crop_p.x, crop_p.y, new_w, new_h);
        Mat z_frame= frame(roi);
        done = true;
    }
}
