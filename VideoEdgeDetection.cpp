#include "stdafx.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <set>
#include "VideoEdgeDetection.h"

void readImageData(cv::VideoCapture& cap, cv::Mat& imgOriginal, int& retflag)
{
	retflag = 1;
	bool bSuccess = cap.read(imgOriginal); // read a new frame from video
	if (!bSuccess) //if not success, break loop
	{
		std::cout << "Cannot read a frame from video stream" << std::endl;
		{ retflag = 2; return; };
	}
}

struct Ball {
	Ball() = default;
	Ball(Ball const&) = default;
	Ball& operator=(Ball const&) = default;
	Ball(Ball&&) = default;
	Ball& operator=(Ball&&) = default;
	Ball(cv::Mat const& mat);

	Ball collide(cv::Mat contours, int x, int y);
private:
	int x = 0;
	int y = 0;
	int threshold = 5;
	int degrees = 0;
};

Ball::Ball(cv::Mat const& mat)
	: x( rand() % mat.rows )
	, y( rand() % mat.cols )
	, threshold(5)
	, degrees( rand() % 360)
{

}

Ball Ball::collide(cv::Mat contours, int x , int y)
{
	Ball ret;
	cv::Vec3b* center = contours.ptr<cv::Vec3b>(x);
	int innenwinkel = center[y][1] - degrees;
	ret.degrees = degrees - 2 * innenwinkel;
	std::cout << degrees << ' ' << center[y][1] << ' ' << ret.degrees << '\n';
	return ret;
}

int main(int argc, char** argv)
{
	cv::VideoCapture cap("D:\ToiletBank.mp4"); //capture the video from web cam
	if (!cap.isOpened())  // if not success, exit program
	{
		std::cout << "Cannot open the web cam" << std::endl;
		return -1;
	}

	std::string original = "Original";
	std::string threshold = "Thresholded Image";
	namedWindow(original, cv::WINDOW_AUTOSIZE);
	namedWindow(threshold, cv::WINDOW_AUTOSIZE);

	int j = 0;
	while (true)
	{
		cv::Mat imgOriginal;
		int retflag;
		readImageData(cap, imgOriginal, retflag);
		if (retflag == 2) break;
		cv::Mat contours = od::detect_directions(imgOriginal);

		if (j++ == 0)
		{
			Ball ball(contours);
			for (int i = 0; i < 100; i++)
				ball.collide(contours, 100, 100);
		}
		//void moveBall(ball, contours);


		imshow(threshold, contours); //show the thresholded image
		imshow(original, imgOriginal); //show the original image

		if (cv::waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
		{
			std::cout << "esc key is pressed by user" << std::endl;
			break;
		}
	}
	return 0;
}