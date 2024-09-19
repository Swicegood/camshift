#include "CameraApp.h"
#include <wx/wx.h>
#include <thread>
#include <chrono>

// OpenCV and face detection variables
using namespace cv;

CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;

Rect roi;
bool done = true;

auto tp1 = std::chrono::system_clock::now();
auto tp2 = std::chrono::system_clock::now();

// Main function that runs the camera capture and face detection
int runCameraApp(int camera_device, int resh, int resv) {
    // Load the cascades
    String face_cascade_name = "haarcascade_frontalface_alt.xml";
    String eyes_cascade_name = "haarcascade_eye_tree_eyeglasses.xml";

    if (!face_cascade.load(face_cascade_name)) {
        wxLogMessage("--(!)Error loading face cascade");
        return -1;
    }
    if (!eyes_cascade.load(eyes_cascade_name)) {
        wxLogMessage("--(!)Error loading eyes cascade");
        return -1;
    }

    // Control for speed (used in trackbar)
    int v = 165;
    namedWindow("Capture - Face detection");
    createTrackbar("Speed", "Capture - Face detection", &v, 254, NULL);

    // Video capture initialization
    VideoCapture capture;
    if (!capture.open(camera_device)) {
        std::cout << "--(!)Error opening video capture\n";
        return -1;
    }

    // Set resolution
    capture.set(CAP_PROP_FRAME_WIDTH, resh);
    capture.set(CAP_PROP_FRAME_HEIGHT, resv);

    Mat frame;
    int i = 0;
    Rect cur_roi, target_roi;
    float dx, dy, dw, D;
    dx = dy = dw = D = 0;
    bool zoom_reached = true;
    bool kill = false;
    float ar;

    while (capture.read(frame)) {
        if (frame.empty()) {
            std::cout << "--(!) No captured frame -- Break!\n";
            break;
        }

        // Aspect ratio
        ar = static_cast<float>(frame.rows) / static_cast<float>(frame.cols);

        // Handle timing
        tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        // Initialize ROI
        if (i == 0) {
            roi = Rect(0, 0, frame.cols, frame.rows);
            cur_roi = roi;
        }

        // Face detection logic
        if (done && zoom_reached) {
            target_roi = roi;
            dx = target_roi.x - cur_roi.x;
            dy = target_roi.y - cur_roi.y;
            dw = target_roi.width - cur_roi.width;
            D = std::sqrt(dx * dx + dy * dy);

            if (((D < 90) && (dw < 60)) || v == 0) {
                target_roi = cur_roi;
            }

            done = false;
        }

        // Run face detection occasionally to save CPU cycles
        if (i % 90 == 0)
            std::thread(detectAndDisplay, frame).detach();

        // Calculate movement towards target ROI
        dx = target_roi.x - cur_roi.x;
        dy = target_roi.y - cur_roi.y;
        dw = target_roi.width - cur_roi.width;

        if (!target_roi.empty() && !cur_roi.empty()) {
            int speed_factor = 255 - v;

            cur_roi.x += (dx / speed_factor) >= 0 ? std::ceil(dx / speed_factor) : std::floor(dx / speed_factor);
            cur_roi.y += (dy / speed_factor) >= 0 ? std::ceil(dy / speed_factor) : std::floor(dy / speed_factor);
            cur_roi.width += (dw / speed_factor) >= 0 ? std::ceil(dw / speed_factor) : std::floor(dw / speed_factor);

            // Boundary checks with manual clamping
            if (cur_roi.x < 0)
                cur_roi.x = 0;
            else if (cur_roi.x > frame.cols)
                cur_roi.x = frame.cols;

            if (cur_roi.y < 0)
                cur_roi.y = 0;
            else if (cur_roi.y > frame.rows)
                cur_roi.y = frame.rows;

            if (cur_roi.width < 0)
                cur_roi.width = 0;
            else if (cur_roi.width > frame.cols)
                cur_roi.width = frame.cols;

            cur_roi.height = static_cast<int>(cur_roi.width * ar);
            if (cur_roi.height + cur_roi.y > frame.rows)
                cur_roi.y = frame.rows - cur_roi.height;

            // Zoomed frame
            Mat z_frame = frame(cur_roi);
            resize(z_frame, z_frame, frame.size(), 0, 0, INTER_LINEAR);

            if (getWindowProperty("Capture - Face detection", WND_PROP_VISIBLE))
                imshow("Capture - Face detection", z_frame);
            else
                kill = true;

            // Check if zoom target is reached
            if ((cur_roi.x == target_roi.x) && (cur_roi.y == target_roi.y)) {
                zoom_reached = true;
            }
        }

        if ((waitKey(10) == 27) || kill) {
            break; // Exit on ESC key or window close
        }

        i++;
    }

    return 0;
}

// Function for face detection
void detectAndDisplay(Mat frame) {
    Mat frame_gray;
    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);

    // Detect faces
    std::vector<Rect> faces;
    face_cascade.detectMultiScale(frame_gray, faces);

    if (faces.empty() || faces[0].x < 0 || faces[0].y < 0 ||
        (faces[0].x + faces[0].width > frame.cols) ||
        (faces[0].y + faces[0].height > frame.rows)) {

        roi = Rect(0, 0, frame.cols, frame.rows);
    }
    else {
        // Calculate new ROI around the face
        int new_w = 3 * faces[0].width;
        int new_h = static_cast<int>(new_w * (static_cast<float>(frame.rows) / frame.cols));

        new_w = std::min(new_w, frame.cols);
        new_h = std::min(new_h, frame.rows);

        Point center(faces[0].x + faces[0].width / 2, faces[0].y + faces[0].height / 2);
        Point crop_p(center.x - new_w / 2, center.y - new_h / 2);

        // Boundary checks with manual clamping
        if (crop_p.x < 0)
            crop_p.x = 0;
        else if (crop_p.x > frame.cols - new_w)
            crop_p.x = frame.cols - new_w;

        if (crop_p.y < 0)
            crop_p.y = 0;
        else if (crop_p.y > frame.rows - new_h)
            crop_p.y = frame.rows - new_h;

        // Set new ROI
        roi = Rect(crop_p.x, crop_p.y, new_w, new_h);
        done = true;
    }
}


