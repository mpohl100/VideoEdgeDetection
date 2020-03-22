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


static int logCounter = 0;

int gradient(int tl, int tc, int tr, int cl, int cc, int cr, int bl, int bc, int br)
{
	int grad = 0;
	float grad_tl_br = br - tl;
	float grad_cl_cr = cr - cl;
	float grad_bl_tr = tr - bl;
	float grad_bc_tc = tc - bc;
	float sqrt2 = std::sqrt(2.0);
	float grad_x = grad_cl_cr + grad_tl_br * sqrt2 + grad_bl_tr * sqrt2;
	float grad_y = -grad_bc_tc + grad_tl_br * sqrt2 - grad_bl_tr * sqrt2;
	float grad_total = std::sqrt(grad_x * grad_x + grad_y * grad_y);
	grad = int(grad_total);
	if (logCounter++ < 100) {
		std::cout << "gradient of:\n";
		std::cout << tl << " " << tc << " " << tl << '\n';
		std::cout << cl << " " << cc << " " << cl << '\n';
		std::cout << bl << " " << bc << " " << bl << '\n';
		std::cout << "grad_x = " << grad_x << "; grad_y = " << grad_y << "\n";
		std::cout << "grad = " << grad;
	}
	return grad;
}

Mat homeBrewEdgeDetection(Mat const& img)
{
	Mat ret = img.clone();

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
			for(int color = 0; color <= 2; color++)
				retCenter[i][color] = gradient(imgUpper[i-1][color], imgUpper[i][color], imgUpper[i+1][color],
				                               imgCenter[i-1][color], imgCenter[i][color], imgCenter[i+1][color],
								               imgLower[i-1][color], imgLower[i][color], imgLower[i+1][color]);
		}
	}
	return ret;
}

void createControl(std::string const& control)
{
	namedWindow(control, WINDOW_AUTOSIZE); //create a window called "Control"
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

}

int main(int argc, char** argv)
{
	VideoCapture cap("D:\ToiletBank.mp4"); //capture the video from web cam
	if (!cap.isOpened())  // if not success, exit program
	{
		cout << "Cannot open the web cam" << endl;
		return -1;
	}

	std::string threshold = "Thresholded Image";
	std::string original = "Original";
	namedWindow(threshold, WINDOW_AUTOSIZE);
	namedWindow(original, WINDOW_AUTOSIZE);

	createControl("Control");

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