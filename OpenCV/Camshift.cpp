#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
using namespace cv;
using namespace std;
int main()
{
    VideoCapture cam;
    while (!cam.open(0))cerr << "failed to open cam" << endl;
    Mat frame, roi, hsv_roi, mask;
    // take first frame of the video
    cam >> frame;
    // setup initial location of window
    Rect track_window(300, 200, 100, 50); // simply hardcoded the values
    // set up the ROI for tracking
    roi = frame(track_window);
    cvtColor(roi, hsv_roi, COLOR_BGR2HSV);
    inRange(hsv_roi, Scalar(0, 60, 32), Scalar(180, 255, 255), mask);
    float range_[] = { 0, 180 };
    const float* range[] = { range_ };
    Mat roi_hist;
    int histSize[] = { 180 };
    int channels[] = { 0 };
    calcHist(&hsv_roi, 1, channels, mask, roi_hist, 1, histSize, range);
    normalize(roi_hist, roi_hist, 0, 255, NORM_MINMAX);
    // Setup the termination criteria, either 10 iteration or move by atleast 1 pt
    TermCriteria term_crit(TermCriteria::EPS | TermCriteria::COUNT, 10, 1);
    while (true) {
        Mat hsv, dst;
        cam >> frame;
        if (frame.empty())
            break;
        cvtColor(frame, hsv, COLOR_BGR2HSV);
        calcBackProject(&hsv, 1, channels, roi_hist, dst, range);
        // apply camshift to get the new location
        RotatedRect rot_rect = CamShift(dst, track_window, term_crit);
        // Draw it on image
        Point2f points[4];
        rot_rect.points(points);
        for (int i = 0; i < 4; i++)
            line(frame, points[i], points[(i + 1) % 4], 255, 2);
       /* Mat roi_final;
        roi_final = frame;
        bool fullscreen(false);
        float width(points[3].x - points[0].x);
        float height(points[2].y - points[1].y);
        if (width < 0 || height < 0 || width > frame.cols || height > frame.cols)
            fullscreen = true;
        Point2f new_p[4];
        for (int i = 0; i < 4; i++) {
            if ((points[i].x < 0) || (points[i].y < 0) || (points[i].x > frame.cols) || (points[i].y > frame.rows)) {
                fullscreen = true;
                break;
            }

        }
        for (int i = 0; i < 4; i++) {
            switch (i)
            {
            case 0:
                new_p[i].x = (points[i].x - width);
                new_p[i].y = (points[i].y - width);
                break;
            case 1:
                new_p[i].x = (points[i].x + width);
                new_p[i].y = (points[i].y - width);
                break;
            case 2:
                new_p[i].x = (points[i].x - width);
                new_p[i].y = (points[i].y + width);
                break;
            case 3:
                new_p[i].x = (points[i].x - width);
                new_p[i].y = (points[i].y + width);
                break;
           
            }
            if (new_p[i].x < 0)
                new_p[i].x = 0;
            if (new_p[i].x > frame.cols)
                new_p[i].x = frame.cols;
            if (new_p[i].y < 0)
                new_p[i].y = 0;
            if (new_p[i].y > frame.rows)
                new_p[i].y = frame.rows;

        }
       
        if (((new_p[3].x - new_p[0].x) > frame.cols) || ((new_p[2].y - new_p[1].y) > frame.rows)) 
            fullscreen = true;
        int new_w(new_p[3].x - new_p[0].x);
        int new_h(new_p[2].y - new_p[1].y);
        if (((new_p[0].x + new_w) > frame.cols) || ((new_p[0].y + new_h) > frame.rows))
            fullscreen = true;
        if (!fullscreen) {
            Rect window(new_p[0].x, new_p[0].y, new_w, new_h);
            roi_final = frame(window);
        }*/
        imshow("img2", frame);
         int keyboard = waitKey(30);
        if (keyboard == 'q' || keyboard == 27)
            break;
    }
}