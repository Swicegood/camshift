
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
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

//using namespace std;
using namespace cv;
void detectAndDisplay(Mat frame);
int my_s();
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
Rect roi;

auto tp1 = std::chrono::system_clock::now();
auto tp2 = std::chrono::system_clock::now();


SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket = INVALID_SOCKET;

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

// Run socket server 

    std::thread(my_s).detach();
   


    VideoCapture capture;
    //-- 2. Read the video stream
    capture.open(camera_device);
    if (!capture.isOpened())
    {
        std::cout << "--(!)Error opening video capture\n";
        return -1;
    }
    Mat frame;
    int i = 0; 
    Mat z_frame, z_frame2;
    roi = Rect(0, 0, frame.cols, frame.rows);
    Rect cur_roi(0, 0, frame.cols, frame.rows);
    float v = 30;
    float d=0;
    float dx_a, dy_a, dw_a;
    bool zooming = false;
    Rect w_roi;
        while (capture.read(frame))
        {
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
        
        if (i % 90 == 0)
            std::thread(detectAndDisplay,frame).detach(); 
       
        float dx(abs(cur_roi.x - roi.x));
        float dy(abs(cur_roi.y - roi.y));
        float D = sqrt((dx * dx) + (dy * dy));
        
 
        if (D > 120 && !zooming) {
            w_roi = roi;
            zooming = true;
            dx_a = cur_roi.x;
            dy_a = cur_roi.y;
            dw_a = cur_roi.width;
        }
        
        if (!w_roi.empty() && !cur_roi.empty()) {
            if (zooming) {
                d =  (v * fElapsedTime);

                if (dx_a - w_roi.x > 0)
                    dx_a = dx_a - d;
                else if (dx_a - w_roi.x < 0)
                    dx_a = dx_a + d;
                if (dx_a < 0)
                    dx_a = 0;
                if (dx_a > frame.cols)
                    dx_a = frame.cols;

                if (dy_a - w_roi.y > 0)
                    dy_a = dy_a - d;
                else if (dy_a - w_roi.y < 0)
                    dy_a = dy_a + d;

                if (dy_a < 0)
                    dy_a = 0;
                if (dy_a > frame.rows)
                    dy_a = frame.rows;

                if (dw_a - w_roi.width > 0)
                    dw_a = dw_a - d;
                else if (dw_a - w_roi.width < 0)
                    dw_a = dw_a + d;
                if (dw_a < 0)
                    dw_a = 0;
                if (dw_a > frame.cols)
                    dw_a = frame.cols;
                
            cur_roi.x = dx_a;
            cur_roi.y = dy_a;
            cur_roi.width = dw_a;
            }
            /*  if (cur_roi.height - roi.height > 0)
                  cur_roi.height = cur_roi.height - d;
              else if (cur_roi.height - roi.height < 0)
                  cur_roi.height = cur_roi.height + d;
              if (cur_roi.height < 0)
                  cur_roi.height = 0;
              if (cur_roi.height > frame.rows)
                  cur_roi.height = frame.rows;*/
            



            cur_roi.height = floor(float(cur_roi.width) * (float(frame.rows) / float(frame.cols)));
            if (cur_roi.height + cur_roi.y > frame.rows)
                cur_roi.height = frame.rows - cur_roi.y;
                                      
            z_frame = frame(cur_roi);
            resize(z_frame, z_frame, frame.size(), 0, 0, INTER_LINEAR);
            imshow("Capture - Face detection", z_frame);
            z_frame = (z_frame.reshape(0, 1)); // to make it continuous           
            int  imgSize = z_frame.total() * z_frame.elemSize();
            if (ClientSocket != INVALID_SOCKET)
                int bytes = send(ClientSocket, reinterpret_cast<const char*>(z_frame.data), imgSize, 0);
            if (cur_roi.x == roi.x && cur_roi.y == roi.y) {
                zooming = false;
                d = 0;
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
//        imshow("Capture - Face detection", frame);
          roi = Rect(0, 0, frame.cols, frame.rows);          
    }
    else{
        int new_w(3 * faces[0].width);
        int new_h(floor(float(new_w) * (float(frame.rows) / float(frame.cols))));
    /*    int new_h(3 * faces[0].height);*/
        if (new_w > frame.cols)
            new_w = frame.cols;
        if (new_h > frame.rows)
            new_h = frame.rows;
        Point _center(faces[0].x + faces[0].width / 2, faces[0].y + faces[0].height / 2);
        Point crop_p(_center.x - .5 * new_w, _center.y - .5 * new_h);
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
    }
}
int my_s() {

    // Declare some variables
    WSADATA wsaData;

    int iResult = 0;            // used to return function results

    //// the listening socket to be created
    //SOCKET ListenSocket = INVALID_SOCKET;
    //SOCKET ClientSocket = INVALID_SOCKET;

    // The socket address to be passed to bind
    sockaddr_in service;

    //----------------------
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        wprintf(L"Error at WSAStartup()\n");
        return 1;
    }
    //----------------------
    // Create a SOCKET for listening for 
    // incoming connection requests
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        wprintf(L"socket function failed with error: %u\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
    service.sin_port = htons(2345);

    //----------------------
    // Bind the socket.
    iResult = bind(ListenSocket, (SOCKADDR*)&service, sizeof(service));
    if (iResult == SOCKET_ERROR) {
        wprintf(L"bind failed with error %u\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    else
        wprintf(L"bind returned success\n");


    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    // Accept a client socket
    while (true) {
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            //return 1;
        }
    }
}