#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

Mat take_diff(Mat cur, Mat prev);


char difftext[20];

int main( int argc, char** argv )
{
    Mat mat_frame1, mat_gray1;
    Mat mat_frame2, mat_gray2, mat_diff;
    VideoCapture vcap;
    unsigned int diffsum, maxdiff;
    double percent_diff;


    // Check command line arguments
    if(argc < 3) 
    {
	    printf("Usage: brighten <input-file>\n");
            exit(-1);
    }

    // Mat is a matrix object
    mat_frame1 = imread( argv[1] ); // read in image file
    mat_frame2 = imread( argv[2] ); // read in image file
    cv::Size size(800, 600);
    Mat mat_f1, mat_f2, mat_d2;

    //instead break into the 3 color frames

    cv::cvtColor(mat_frame1, mat_gray1, cv::COLOR_BGR2GRAY);
    cv::cvtColor(mat_frame2, mat_gray2, cv::COLOR_BGR2GRAY);


    mat_diff = mat_gray1.clone();

	// absdiff(mat_gray1, mat_gray2, mat_diff);
    mat_diff = take_diff(mat_frame1, mat_frame2);

    cv::resize(mat_frame1, mat_f1, size, 0,0,cv::INTER_LINEAR);
    cv::resize(mat_frame2, mat_f2, size, 0,0,cv::INTER_LINEAR);
    cv::resize(mat_diff, mat_d2, size, 0,0,cv::INTER_LINEAR);


    while(1){
        cv::imshow("bi-color Diff", mat_f1);
        if (waitKey(500) == 27){
            break;
        } 
        cv::imshow("bi-color Diff", mat_d2 + mat_f1);
        if (waitKey(250) == 27){
            break;
        } 
        cv::imshow("bi-color Diff", mat_d2 + mat_f2);
        if (waitKey(250) == 27){
            break;
        } 
        cv::imshow("bi-color Diff", mat_f2);
        if (waitKey(250) == 27){
            break;
        } 
    }
	
};


Mat take_diff(Mat cur, Mat prev){
    // cerr << "Take difference" << endl;
    Mat difference(cur.rows, cur.cols, CV_8UC3);

    //loop through all
    for(int i=0; i < cur.rows; i++){
        for(int j=0; j < cur.cols; j++){
    
            //if difference is less than use the red pane if difference is greater than use the green pane
            if ((cur.at<Vec3b>(i,j)[0] - prev.at<Vec3b>(i, j)[0]) < 0){
                //red
                difference.at<Vec3b>(i,j)[2] = abs(abs(cur.at<Vec3b>(i,j)[0]) - abs(prev.at<Vec3b>(i, j)[0]));
            }else{
                //green
                difference.at<Vec3b>(i,j)[1] = abs(abs(cur.at<Vec3b>(i,j)[0]) - abs(prev.at<Vec3b>(i, j)[0]));
            }
            //blue
            difference.at<Vec3b>(i,j)[0] = 0;
        }
    }
    return difference;
}