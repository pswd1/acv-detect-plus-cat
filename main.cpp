#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <utility>
#include <tuple>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/objdetect.hpp"

using namespace cv;
using namespace std;

//MOTION PROTOTYPES
bool motiondetect(int sensitivity, Mat prev_frame, Mat curr_frame);
void exclusionzone(tuple<int, int> top_left,tuple<int, int> bottom_right,  Mat in_mat, Mat &ex_mat);
void populatezones(Mat in, Mat &out);
void onmouse(int action, int x, int y, int, void*);
void onTrackbar(int pos, void*);
int motioncalibrate(VideoCapture camera);

//PET PROTOTYPES
bool catDetect(Mat frame);

//GLOBAL VARIABLES
CascadeClassifier cat_cascade;
vector<pair<tuple<int, int>, tuple<int, int>>> exclusion_collection;//data-struct of tuples each rectangle is comprised of a pair of points
tuple<int, int> tl(-1,-1); //top right
tuple<int, int> br(-1,-1); //bot left
Mat temp;
int sliderValue = 0;
int sensitivity = 1;


//KEYBINDS
#define YES_KEY (89)
#define YES_KEY_LOWER (121)
#define NO_KEY (78)
#define NO_KEY_LOWER (110)
#define ENTER_KEY (13)
#define ESCAPE_KEY (27)


// MAIN
int main(int argc, char* argv[]) {
  //create the keys for the arg parser
  const String keys =
    "{help h usage ? || print this message }"
    "{c camera       || specify a camera to use}"
    "{b calibrate    || run motion calibration for exclusions and sensitivity before starting the program}"
    "{d diff         || use the diff tool to compare two images for changes}"
    ;
  //parce command line arguments arguments
  cv::CommandLineParser parser(argc, argv, keys);

  //local variables
  VideoCapture cap;
  
  if (parser.has("help") || parser.has("h")){
      parser.printMessage();
      return 0;
  }

  if (parser.has("camera") || parser.has("c")){
    cout << "Using Capture Device: " << argv[1] << endl;
    int cam = parser.get<int>("camera");
    cap.open(cam);
    if(!cap.isOpened()) {
      cout << "--(!)Error opening camera\n";
      return -1;
    }
  }

  if (parser.has("calibrate") || parser.has("b")){
    //check if a camera is opened already
    if(!cap.isOpened()){
      //attempt to open default camera
      cap.open(0);
    }
    if(!cap.isOpened()) {
      cout << "--(!)Error opening camera\n";
      return -1;
    }
    //run calibration
    motioncalibrate(cap);
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
    //if exclusion zones exist populate them all now
    populatezones(frame, excluded_frame);


    // exclusionzone(tr, bl, frame, excluded_frame);

    imshow("Original", frame);
    imshow("Excluded", excluded_frame);

    if(!prev_frame.empty()) {
      Mat excluded_prev_frame;
      populatezones(prev_frame, excluded_prev_frame);

      // Check for motion, if yes pass frame to catDetect
      if(motiondetect(sensitivity, excluded_prev_frame, excluded_frame) == true) {
        catDetect(frame);
      }
    }
    // Exit on ESC key
    if(waitKey(1) == 27)
      break;
  }

  return 0;
}



//MOTION DETECTION FUNCTIONS
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

void exclusionzone(tuple<int, int> top_left, tuple<int, int> bottom_right,  Mat in_mat, Mat &ex_mat){
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

  if (x1 > x2) swap(x1, x2);
  if (y1 > y2) swap(y1, y2);

  //loop through ex_mat and overwrite pixels
  // cout << ex_mat
  for(int y = 0; y < ex_mat.rows; y++){
      if(y >= y1 && y <=y2){
          //if row is in range
          for(int x = 0; x < ex_mat.cols; x++){
              //if col is in range
              if(x >= x1 && x <=x2){
                  ex_mat.at<Vec3b>(y,x)[0] = 0;
                  ex_mat.at<Vec3b>(y,x)[1] = 0;
                  ex_mat.at<Vec3b>(y,x)[2] = 0;
                  //pixel == 255
              }
              
          }
          
      }
  }
}

void populatezones( Mat in, Mat &out){
  if(!exclusion_collection.empty()){
    //loop through collection
    out = in.clone();
    for (auto& [p1, p2] : exclusion_collection){
        exclusionzone(p1, p2, out, out);
    }
  }
}

void onTrackbar(int pos, void*) {
  sensitivity = pos;
}

void onmouse(int action, int x, int y, int, void*){
  if(action == EVENT_LBUTTONDOWN){
      cout << "click detected (" << x << ", " << y << ")" << endl;
      //mouse click
      tl = make_tuple(x,y);
  }else if(action == EVENT_LBUTTONUP){
      cout << "unclick detected (" << x << ", " << y << ")" << endl;
      //unclick
      br = make_tuple(x,y);
      cout << "keep? (y/n/enter/esc)" << endl;
  }
}

int motioncalibrate(VideoCapture camera){
    //sensitivity num
    Mat frame, exclusions, e_instructions;
    bool done = false;
    
    //start capture from cam
    // VideoCapture cam0(0); 
    namedWindow("exclusion selection");

    //TODO: write on the window instructions "click and drag a square to exclude area"

    //capture frame
    camera.read(frame);
    exclusions = frame.clone();
    

    //loop
    //void cv::setMouseCallback 	(const String &winname, MouseCallback onMouse, void *  userdata = 0 ) 
    	

    bool skip_switch = false;
    int key = -1;
    //GET EXCLUSION ZONES
    while(1){
        
        //wait for bottom left point to populate
        while(get<1>(br) == -1 && get<1>(tl) == -1){
            string instruct = "click and drag to select an area or press enter to continue";
            e_instructions = exclusions.clone();
            putText(e_instructions, instruct, Point(20, 20), 2, 0.5, Scalar(50,50,50), 2);
            imshow("exclusion selection", e_instructions);
            //draw exclusion with mouse
            setMouseCallback("exclusion selection", onmouse);
            char input;
            if((input = waitKey(1)) == ESCAPE_KEY){
                return -1;
            }else if(input == ENTER_KEY){
                skip_switch = true;
                done = true;
                break;
            }
        }  
        
        string instruct = "keep this area (y/n); esc to quit; enter to discard and continue";
        e_instructions = exclusions.clone();
        exclusionzone(tl, br, exclusions, e_instructions);
        putText(e_instructions, instruct, Point(20, 20), 2, 0.5, Scalar(50,50,50), 2);
        imshow("exclusion selection", e_instructions);
        if(!skip_switch){
            key = waitKey(1);
        }

        pair<tuple<int, int>, tuple<int, int>> pair;

        switch(key){
            //keep (y/n)
            case YES_KEY:
            case YES_KEY_LOWER:
                //write
                cout << "yes" << endl;
                cout << "(" << get<0>(tl) << ", " << get<1>(tl) << ") ; (" << get<0>(br) << ", " << get<1>(br) << ")" << endl;
                pair = make_pair(tl,br);
                exclusion_collection.push_back(pair);
                //update mat
                exclusionzone(tl, br, exclusions, exclusions);
                tl = make_tuple(-1,-1);
                br = make_tuple(-1,-1);
                break;
            case NO_KEY:
            case NO_KEY_LOWER:
                cout << "no" << endl;
                tl = make_tuple(-1,-1);
                br = make_tuple(-1,-1);
            cout << "click and drag again, enter to continue, esc to quit" << endl;
                break;
            //done? (enter)
            case ENTER_KEY:
                done = true;
                break;
            //exit (esc)
            case ESCAPE_KEY:
                return -1;
                break;//useless but looks nice
            case -1:
                continue;
            default:
                break;
        }
        if(done){
            break;
        }
    }

    //show camera with sensitivity bar
    Mat ex2, ex1, f1;
    ex2 = exclusions.clone();
    bool capture = false;
    sensitivity = 1;
    destroyWindow("exclusion selection");
    //SENSITIVITY CALIBRATE
    while(1){
        //press enter to save value
        //TODO: toggle exclusions (space?)
        //create window
        namedWindow("sensitivity calibration");
        
        //read cam
        camera.read(frame);
        //exclude selection(s)
        populatezones(frame, ex1);

        //create slider
        createTrackbar("sensitivity", "sensitivity calibration", nullptr, (frame.rows * frame.cols * 5), onTrackbar);

        //check motion
        capture = motiondetect(sensitivity, ex2, ex1);

        //write if detect is true on f1
        string text = (capture) ? "motion detected! (" : "no motion (";
        text.append(to_string(sensitivity));
        text.append(")");
        f1 = ex1.clone();
        putText(f1, text, Point(20, 20), 2, 0.5, Scalar(50,50,50), 2);
        
        //show frame
        imshow("sensitivity calibration", f1);
        key = waitKey(1);

        //exit if enter or esc
        if (key == ESCAPE_KEY){
            return -1;
        }else if(key == ENTER_KEY){
            break;
        }

        ex2 = ex1.clone();
    }
    destroyWindow("sensitivity calibration");
    return 0;
}


/*
Credits:
https://docs.opencv.org/4.11.0/db/d28/tutorial_cascade_classifier.html
https://github.com/opencv/opencv/tree/4.x/data/haarcascades
*/

//PET DETECTION FUNCTIONS
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


