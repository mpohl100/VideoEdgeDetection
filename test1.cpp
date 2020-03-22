#include "stdafx.h"

#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


using namespace cv;
using namespace std;

Mat getContours(Mat const& img)
{
	Mat imgGray;
	std::vector<Mat> channels;
	Mat hsv;
	cvtColor(img, hsv, COLOR_RGB2HSV);
	split(hsv, channels);
	imgGray = channels[0];

	Mat contours;
	int min = 35;
	int max = 90;
	Canny(imgGray, contours, min, max);

	return contours;
}

Mat homeBrewEdgeDetection(Mat const& img)
{
	Mat ret(img);

	Vec3b* retCenter;
	const Vec3b* imgUpper;
	const Vec3b* imgCenter;
	const Vec3b* imgLower;
	for (int j = 1; j < img.rows - 1; ++j)
	{
		retCenter = ret.ptr<Vec3b>(j);
		imgUpper = img.ptr<Vec3b>(j - 1);
		imgCenter = img.ptr<Vec3b>(j);
		imgLower = img.ptr<Vec3b>(j+1);
		for (int i = 1; i < img.cols - 1; ++i)
		{
			// 0=Blue, 1=Green, 2=Red
			//for (int color = 0; color < 3; color++)
			//{

			//}
			int color = 0;
			float grad_tl_br = imgLower[i + 1][color] - imgUpper[i - 1][color];
			float grad_cl_cr = imgCenter[i + 1][color] - imgCenter[i - 1][color];
			float grad_bl_tr = imgUpper[i + 1][color] - imgLower[i - 1][color];
			float grad_bc_tc = imgUpper[i][color] - imgLower[i][color];
			float sqrt2 = std::sqrt(2.0);
			float grad_x = grad_cl_cr + grad_tl_br * sqrt2 + grad_bl_tr * sqrt2;
			float grad_y = -grad_bc_tc + grad_tl_br * sqrt2 - grad_bl_tr * sqrt2;
			float grad_total = std::sqrt(grad_x * grad_x + grad_y * grad_y);
			retCenter[i][color] = int(grad_total);
		}
	}
	return ret;
}

int main(int argc, char** argv)
{
	VideoCapture cap("D:\ToiletBank.mp4"); //capture the video from web cam
	if (!cap.isOpened())  // if not success, exit program
	{
		cout << "Cannot open the web cam" << endl;
		return -1;
	}

	std::string control = "Control";
	std::string threshold = "Thresholded Image";
	std::string original = "Original";
	namedWindow(control, WINDOW_AUTOSIZE); //create a window called "Control"
	namedWindow(threshold, WINDOW_AUTOSIZE);
	namedWindow(original, WINDOW_AUTOSIZE);

	int iLowH = 0;
	int iHighH = 179;

	int iLowS = 0;
	int iHighS = 255;

	int iLowV = 0;
	int iHighV = 255;

	//Create trackbars in "Control" window
	createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	createTrackbar("HighH", "Control", &iHighH, 179);

	createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	createTrackbar("HighS", "Control", &iHighS, 255);

	createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
	createTrackbar("HighV", "Control", &iHighV, 255);

	while (true)
	{
		Mat imgOriginal;
		bool bSuccess = cap.read(imgOriginal); // read a new frame from video
		if (!bSuccess) //if not success, break loop
		{
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}
		//Mat contours = getContours(imgOriginal);
		Mat contours = homeBrewEdgeDetection(imgOriginal);
		imshow(threshold, contours); //show the thresholded image
		imshow(original, imgOriginal); //show the original image

		if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}
	return 0;
}