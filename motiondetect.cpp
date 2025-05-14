
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

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
    diff_value = (unsigned int)cv::sum(mat_diff)[0]; //add up only the blue frame
    if (diff_value > sensitivity){
        //motion detected
        return true;
    }
    return false;
}