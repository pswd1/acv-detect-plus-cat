
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

void testexclusionzone(){
    Mat original;
    Mat excluded;

    VideoCapture cam0(0);

    cam0.read(original);

    if (!cam0.isOpened())
    {
        exit(-1);
    }


    tuple<int , int > tr(170, 83);
    tuple<int , int > bl(400, 377);

    exclusionzone(tr, bl, original, excluded);
    namedWindow("original");
    namedWindow("excluded");
    imshow("original", original);
    
    imshow("excluded", excluded);
    waitKey();
}

// int main(){
//     testexclusionzone();
//     return 0;
// }
