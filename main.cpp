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
                                                     //the line above had mat_diff i changed it to diff_frame
  if (diff_value > sensitivity){
    //motion detected
    return true;
  }
  return false;
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


int main(int argc, char* argv[]){

  // check for cmd line args
  //
  if(argc < 2){
    cerr << "Error: \n" << "Usage: ./main (capture device) \n" << "./main 1" << endl; 
    return -1;
  } else {
    int cam = stoi(argv[1]);
    cout << "Using Capture Device: " << cam << endl;
  }

  // open selected capture device and start capturing frames for processing
  VideoCapture cap(cam);
  if(!cap.isOpened())
  {
    cout << "--(!)Error opening camera 2\n";
    return -1;
  }

  Mat frame;
  Mat prev_frame;
  while(true)
  {
    prev_frame = frame.clone(); // save current frame for motion detection
    cap >> frame;

    if(frame.empty())
    {
      cout << "--(!)No frame captured from camera\n";
      break;
    }

    // Check for motion, if yes pass frame to catDetect
    //@BILLY: This segfaults when passing the values, i dont want to modify your code
    if(motiondetect(1, prev_frame, frame) == true){

      catDetect(frame);
    }

    // Exit on ESC key
    if(waitKey(1) == 27)
      break;
  }

  return 0;
}
