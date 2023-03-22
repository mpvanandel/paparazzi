#include "opencv_image_functions.h"


using namespace std;
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;

void coloryuv_opencv_to_yuv422(Mat image, char *img, int width, int height)
{
  CV_Assert(image.depth() == CV_8U);
  CV_Assert(image.channels() == 3);

  int nRows = image.rows;
  int nCols = image.cols;

  int byte_index = 0;
  for(int r = 0; r < nRows; ++r) {
    for(int c = 0; c < nCols; ++c) {
      Vec3b yuv = image.at<Vec3b>(r, c);
      if((byte_index % 4) == 0) {
        img[byte_index++] = yuv.val[1]; // U
      } else {
        img[byte_index++] = yuv.val[2]; // V
      }
      img[byte_index++] = yuv.val[0]; // Y
    }
  }
}

