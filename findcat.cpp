/*
code largely gathered from:
https://docs.opencv.org/4.11.0/db/d28/tutorial_cascade_classifier.html
and the related github for the example:
https://github.com/opencv/opencv/tree/4.x/data/haarcascades
 */
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
using namespace std;
using namespace cv;
void detectAndDisplay(Mat frame);
CascadeClassifier cat_cascade;
int main()
{
    // Load the cascade
    if(!cat_cascade.load("haarcascade_frontalcatface.xml"))
    {
        cout << "--(!)Error loading cat cascade\n";
        return -1;
    }
    
    // Open camera 2
    VideoCapture cap(2);
    if(!cap.isOpened())
    {
        cout << "--(!)Error opening camera 2\n";
        return -1;
    }
    
    Mat frame;
    while(true)
    {
        cap >> frame;
        
        if(frame.empty())
        {
            cout << "--(!)No frame captured from camera\n";
            break;
        }
        
        // Apply the classifier to the frame
        detectAndDisplay(frame);
        
        // Exit on ESC key
        if(waitKey(1) == 27)
            break;
    }
    
    return 0;
}
void detectAndDisplay(Mat frame)
{
    Mat frame_gray;
    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);
    
    // Detect cats
    std::vector<Rect> cats;
    cat_cascade.detectMultiScale(frame_gray, cats);
    
    for(size_t i = 0; i < cats.size(); i++)
    {
        Point center(cats[i].x + cats[i].width/2, cats[i].y + cats[i].height/2);
        ellipse(frame, center, Size(cats[i].width/2, cats[i].height/2), 0, 0, 360, Scalar(255, 0, 255), 4);
    }
    
    // Show the output
    imshow("Cat Detection", frame);
}

