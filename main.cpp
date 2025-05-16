#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/objdetect.hpp"

using namespace cv;
using namespace std;

bool motiondetect(int sensitivity, Mat prev_frame, Mat curr_frame);
void exclusionzone(tuple<int, int> top_left,tuple<int, int> bottom_right,  Mat in_mat, Mat &ex_mat);

bool catDetect(Mat frame);
CascadeClassifier cat_cascade;


bool motiondetect(int sensitivity, Mat prev_frame, Mat curr_frame){
  /*
sensitivity: the minimum difference needed for motion detection to trigger
prev_frame: the previous frame captured from the camera
curr_frame: the most recent capture from the camera

motion detection will return true if the absolute difference detected is above the given threshold

TODO: a default should be determined if the sensitivity is not given or zero is given
*/
  int diff_value; 
  Mat diff_frame;

  //for now we will just do a color diff not a grayscale
  absdiff(prev_frame, curr_frame, diff_frame);
  diff_value = (unsigned int)cv::sum(diff_frame)[0]; //add up only the blue frame
  if (diff_value > sensitivity){
    //motion detected
    return true;
  }
  return false;
}

void exclusionzone(tuple<int, int> top_left,tuple<int, int> bottom_right,  Mat in_mat, Mat &ex_mat){
  /*
     example mat is 250x250
     example inputs
     x    y
     topleft =   (34,  27)
     topright =  (123, 238)

*/
  //this program will draw a white box over an area in the mat which excludes it from motion detection.
  //get the coordinates of the tuples for top right and lower left
  int x1 = get<0>(top_left);
  int y1 = get<1>(top_left);

  int x2 = get<0>(bottom_right);
  int y2 = get<1>(bottom_right);
  //clone in_mat to ex_mat
  ex_mat = in_mat.clone();

  //loop through ex_mat and overwrite pixels
  for(int x = 0; x < ex_mat.rows; x++){
    for(int y = 0; y < ex_mat.cols; y++){
      //if row is in range
      if(x >= x1 && x <=x2){
        //if col is in range
        if(y >= y1 && y <=y2){
          ex_mat.at<Vec3b>(x,y)[0] = 0;
          ex_mat.at<Vec3b>(x,y)[1] = 0;
          ex_mat.at<Vec3b>(x,y)[2] = 0;
          //pixel == 255
        }

      }

    }
  }
}
/*
Credits:
https://docs.opencv.org/4.11.0/db/d28/tutorial_cascade_classifier.html
https://github.com/opencv/opencv/tree/4.x/data/haarcascades
*/

// take in the current frame and check for cat
bool catDetect(Mat frame)
{
  // Load the cascade
  if(!cat_cascade.load("haarcascade_frontalcatface.xml"))
  {
    cout << "--(!)Error loading cat cascade\n";
    return -1;
  }

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
    cout << "Cat detected!" << endl;
  }

  imshow("Cat Detection", frame);
  return true;
}

int main(int argc, char* argv[]) {
  // check for cmd line args
  if(argc < 2) {
    cerr << "Error: \n" << "Usage: ./main (capture device) \n" << "./main 1" << endl; 
    return -1;
  } else {
    cout << "Using Capture Device: " << argv[1] << endl;
  }

  // open selected capture device and start capturing frames for processing
  int cam = stoi(argv[1]);
  VideoCapture cap(cam);
  if(!cap.isOpened()) {
    cout << "--(!)Error opening camera\n";
    return -1;
  }

  tuple<int, int> tr(170, 83);
  tuple<int, int> bl(400, 377);

  namedWindow("Original");
  namedWindow("Excluded");

  Mat frame;
  Mat prev_frame;
  Mat excluded_frame;

  while(true) {
    prev_frame = frame.clone(); // save current frame for motion detection

    cap >> frame;
    if(frame.empty()) {
      cout << "--(!)No frame captured from camera\n";
      break;
    }

    exclusionzone(tr, bl, frame, excluded_frame);

    imshow("Original", frame);
    imshow("Excluded", excluded_frame);

    if(!prev_frame.empty()) {
      Mat excluded_prev_frame;
      exclusionzone(tr, bl, prev_frame, excluded_prev_frame);

      // Check for motion, if yes pass frame to catDetect
      if(motiondetect(1, excluded_prev_frame, excluded_frame) == true) {
        catDetect(excluded_frame);
      }
    }

    // Exit on ESC key
    if(waitKey(1) == 27)
      break;
  }

  return 0;
}
