#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <thread>

using namespace std;
using namespace cv;
void detectAndDisplay(Mat frame);
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
Rect roi;

auto tp1 = std::chrono::system_clock::now();
auto tp2 = std::chrono::system_clock::now();

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
        cout << "--(!)Error loading face cascade\n";
        return -1;
    };
    if (!eyes_cascade.load(eyes_cascade_name))
    {
        cout << "--(!)Error loading eyes cascade\n";
        return -1;
    };
    int camera_device = parser.get<int>("camera");
    VideoCapture capture;
    //-- 2. Read the video stream
    capture.open(camera_device);
    if (!capture.isOpened())
    {
        cout << "--(!)Error opening video capture\n";
        return -1;
    }
    Mat frame;
    int i = 0; 
    Mat z_frame, z_frame2;
    roi = Rect(0, 0, frame.cols, frame.rows);
    Rect cur_roi(0, 0, frame.cols, frame.rows);
    float vx = 20, vy = 20, vw = 20, vh = 20;
        while (capture.read(frame))
    {
        // Handle Timing
        tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();
            
        if (frame.empty())
        {
            cout << "--(!) No captured frame -- Break!\n";
            break;
        }
        //-- 3. Apply the classifier to the frame
        if (i == 0)
            cur_roi = Rect(0, 0, frame.cols, frame.rows);
        
        if (i % 90 == 0)
            std::thread(detectAndDisplay,frame).detach(); 
            
        if (!roi.empty() && !cur_roi.empty()){
            
            if (cur_roi.x - roi.x > 0)
                cur_roi.x = cur_roi.x - vx * fElapsedTime;
            else if (cur_roi.x - roi.x < 0)
                cur_roi.x = cur_roi.x + vx * fElapsedTime;
            
            if (cur_roi.y - roi.y > 0)
                cur_roi.y = cur_roi.y - vy * fElapsedTime;
            else if (cur_roi.y - roi.y < 0)
                cur_roi.y = cur_roi.y + vy * fElapsedTime;
            
            if (cur_roi.width - roi.width > 0)
                cur_roi.width = cur_roi.width - vw * fElapsedTime;
            else if (cur_roi.width - roi.width < 0)
                cur_roi.width = cur_roi.width + vw * fElapsedTime;

            if (cur_roi.height - roi.height > 0)
                cur_roi.height = cur_roi.height - vh  * fElapsedTime;
            else if (cur_roi.height - roi.height < 0)
                cur_roi.height = cur_roi.height + vh * fElapsedTime;


            z_frame = frame(cur_roi);
            resize(z_frame, z_frame, frame.size(), 0, 0, INTER_LINEAR);
            imshow("Capture - Face detection", z_frame);
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
        int new_h(3 * faces[0].height);
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
       /* resize(z_frame, z_frame, frame.size(), 0,0, INTER_LINEAR);
        imshow("Capture - Face detection", z_frame);*/
    //    return roi;
    }
}