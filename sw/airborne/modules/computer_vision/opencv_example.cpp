#include "modules/computer_vision/cv.h"
#include "modules/computer_vision/cv_opencvdemo.h"
#include "modules/computer_vision/opencv_example.h"


#include "modules/orange_avoider/orange_avoider.h"
#include "firmwares/rotorcraft/navigation.h"
#include "generated/airframe.h"
#include "state.h"
#include "modules/core/abi.h"
#include <time.h>
#include <stdio.h>

#define NAV_C // needed to get the nav functions like Inside...

#include "generated/flight_plan.h"

#define ORANGE_AVOIDER_VERBOSE TRUE

#define PRINT(string,...) fprintf(stderr, "[orange_avoider->%s()] " string,__FUNCTION__ , ##__VA_ARGS__)
#if ORANGE_AVOIDER_VERBOSE
#define VERBOSE_PRINT PRINT
#else
#define VERBOSE_PRINT(...)
#endif

#ifndef OPENCVDEMO_FPS
#define OPENCVDEMO_FPS 0       ///< Default FPS (zero means run at camera fps)
#endif


///////////////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv_example.h"
#include "opencv2/video/tracking.hpp"
#include <opencv2/core/types.hpp>

#define MOVING_MEAN_COUNT 3

using namespace cv;

int frame_id = 0;
float flow_left_arr[MOVING_MEAN_COUNT];
float flow_right_arr[MOVING_MEAN_COUNT];
Mat previous_frame_left, previous_frame_right;

void scale_mat(const Mat matrix, Mat& matrix_left, Mat& matrix_right, const int width, const int height, const int width_img, const int height_img)
{
  std::cout << "w"<< width_img<<" "<<width<<"h"<<height_img<<" "<<height<<"\n";

  std::cout << "1: "<<(int) (height_img/2-height/2)<<", 2: "<< (int) (height_img/2)<<", 3: "<<(int) (width_img/2-width/2)<<", 4: "<<(int) (width_img/2+width/2)<<"\n";
  matrix_left = matrix(Range(95,145), Range(160,260));//(Range((int) (height_img/2-height/2), (int) (height_img/2)), Range((int) (width_img/2-width/2), (int) (width_img/2+width/2)));

  matrix_right =matrix(Range(95,145),Range(260,360)); //matrix(Range((int) (height_img/2), (int) (height_img/2+height/2)), Range((int) (width_img/2-width/2), (int) (width_img/2+width/2)));
  
}
void calculate_magnitudes_flow(Mat& mag, Mat prvs, Mat next)
{
    Mat flow(prvs.size(), CV_32FC1);
    calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 15, 3, 5, 1.2, 0);      
    Mat flow_parts[2];
    split(flow, flow_parts);
    magnitude(flow_parts[0], flow_parts[1], mag);
}


void farneback(char *img, float* output_flow, int width, int height, int width_img, int height_img)
{
    std::cout<<"farneback"<<"\n";
    Mat next_frame(width_img, height_img, CV_8UC2, img); 
    // cvtColor(next_frame, next_frame, CV_YUV2RGB_Y422);
    // cvtColor(next_frame, next_frame, CV_RGB2GRAY);
    
    Mat next_frame_gray;

    cvtColor(next_frame, next_frame_gray, CV_YUV2GRAY_Y422);

    Mat next_frame_left, next_frame_right;

    next_frame_left = next_frame_gray;
    next_frame_right = next_frame_gray;
    scale_mat(next_frame_gray, next_frame_left, next_frame_right, width, height, width_img, height_img);

    if (frame_id==0)
    {
    for (int i=0; i<MOVING_MEAN_COUNT; i++)
    {
      flow_left_arr[i] = 0.0;
      flow_right_arr[i] = 0.0;
    }
      previous_frame_left = next_frame_left;
      previous_frame_right = next_frame_right;
      frame_id++;
      return;
    }  
    Mat mag_left(next_frame_left.size(), CV_32FC1);
    calculate_magnitudes_flow(mag_left, previous_frame_left, next_frame_left);
    
    Mat mag_right(next_frame_right.size(), CV_32FC1);
    calculate_magnitudes_flow(mag_right, previous_frame_right, next_frame_right);
    
    float flow_right_sum = 0.0;
    float flow_left_sum = 0.0;
      
      for (int i=0; i<mag_left.rows; i++)
      {
        for (int j=0; j<mag_left.cols; j++)
        {
          flow_left_sum+=mag_left.at<float>(i,j);
          flow_right_sum+=mag_right.at<float>(i,j);
        }
      }
      flow_left_arr[frame_id%MOVING_MEAN_COUNT] = flow_left_sum;
      flow_right_arr[frame_id%MOVING_MEAN_COUNT] = flow_right_sum;
      
      float flow_left_sum_tot = 0.0;
      float flow_right_sum_tot = 0.0;
      for (int i=0; i<MOVING_MEAN_COUNT; i++)
      {
        flow_left_sum_tot += flow_left_arr[i];
        flow_right_sum_tot += flow_right_arr[i];
      }
      
      frame_id++;
      output_flow[0] = flow_left_sum_tot;
      output_flow[1] = flow_right_sum_tot;

      previous_frame_left = next_frame_left;
      previous_frame_right = next_frame_right;
      std::cout<<"left: "<<output_flow[0]<<", right: "<<output_flow[1]<<"\n";

     
}

