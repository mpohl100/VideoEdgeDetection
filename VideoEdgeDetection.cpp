#include "stdafx.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <set>
#include <random>
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
	int x = 0;
	int y = 0;
	int threshold = 5;
	int degrees = 0;
};

int rand_int()
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(0, 110000);
	return dist(mt);
}

Ball::Ball(cv::Mat const& mat)
	: x( rand_int() % mat.rows )
	, y( rand_int() % mat.cols )
	, threshold( 5 )
	, degrees( rand_int() % 360 )
{

}

Ball Ball::collide(cv::Mat contours, int x , int y)
{
	Ball ret;
	cv::Vec3b* center = contours.ptr<cv::Vec3b>(x);
	int innenwinkel = int(center[y][1]) - degrees;
	ret.degrees = degrees - 2 * innenwinkel;
	std::cout << degrees << ' ' << int(center[y][1]) << ' ' << ret.degrees << '\n';
	return ret;
}

double calculate_orientation(cv::Mat const& gradient)
{
	struct Point { double x = 0; double y = 0; };
	struct PolarVec { double len = 0; double angle = 0; };
	struct Data { PolarVec polarVec; Point point; };
	std::vector<Data> directions;
	directions.reserve(gradient.rows* gradient.cols);
	const cv::Vec3b* currentRow;
	double sumLen = 0;
	for (auto i = 0; i < gradient.rows; ++i)
	{
		currentRow = gradient.ptr<cv::Vec3b>(i);
		for (auto j = 0; j < gradient.cols; ++j)
		{
			double len = double(currentRow[j][0]);
			if (len == 0.0)
				continue;
			double angle = currentRow[j][1] != 0 ? double(currentRow[j][1]) : double(-currentRow[j][2]);
			directions.push_back({ { len, angle }, { i, j } });
			sumLen += len;
		}
	}
	double avgAngle = 0;
	for (const auto& dir : directions) 
		avgAngle += dir.polarVec.angle * dir.polarVec.len / sumLen;
	return avgAngle;
}

int main(int argc, char** argv)
{
	//cv::VideoCapture cap("D:\ToiletBank.mp4"); //capture the video from file
	cv::VideoCapture cap(0); //capture the video from web cam

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
		cv::Mat contours = od::detect_angles(imgOriginal);
		cv::Mat gradient = od::detect_directions(imgOriginal);
		double avg_angle = calculate_orientation(gradient);
		std::cout << avg_angle << '\n';
		//if (j++ == 0)
		//{
		//	Ball ball(contours);
		//	for (int i = 0; i < 100; i++)
		//		ball.collide(contours, i, 100);
		//}
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