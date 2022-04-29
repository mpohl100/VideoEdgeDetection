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

struct Point { double x = 0; double y = 0; };
struct PolarVec { double len = 0; double angle = 0; };
struct Data { PolarVec polarVec; Point point; };
constexpr size_t N = 18;
std::array<Point, N> calculate_orientation(cv::Mat const& gradient)
{
	std::vector<Data> directions;
	directions.reserve(gradient.rows* gradient.cols);
	const cv::Vec3b* currentRow;
	std::array<double, N> sumLen;
	for (auto& d : sumLen)
		d = 0.0;
	std::array<Point, N> partials = { {} };
	for (auto i = 0; i < gradient.rows; ++i)
	{
		currentRow = gradient.ptr<cv::Vec3b>(i);
		for (auto j = 0; j < gradient.cols; ++j)
		{
			double len = double(currentRow[j][0]);
			if (len == 0.0)
				continue;
			double angle = currentRow[j][1] != 0 ? double(currentRow[j][1]) : double(-currentRow[j][2]);
			directions.push_back({ { len, angle }, { double(i), double(j) } });
			double factor = (angle + 180.0) / 360.0;
			if (factor < 0)
				factor = 0;
			size_t index =  size_t(factor  * double(partials.size()));
			if (index >= partials.size())
				index = partials.size() - 1;
			sumLen[index] += len;
		}
	}
	std::array<int, N> num;
	for (auto& i : num)
		i = 0;
	for (const auto& dir : directions)
	{
		double factor = (dir.polarVec.angle + 180.0) / 360.0;
		if (factor < 0)
			factor = 0;
		size_t index = size_t(factor * double(partials.size()));
		if (index >= partials.size())
			index = partials.size() - 1;
		partials[index].x += dir.point.x * dir.polarVec.len/sumLen[index];
		partials[index].y += dir.point.y * dir.polarVec.len / sumLen[index];
		num[index]++;
	}
#if 0
	size_t index = 0;
	for (auto& point : partials)
	{
		point.x /= double(num[index]);
		point.y /= double(num[index++]);
	}
#endif
	return partials;
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
		auto partials = calculate_orientation(gradient);
		size_t index = 0;
		for (const auto& partial : partials)
			cv::circle(contours, cv::Point(int(partial.x), int(partial.y)), 5, cv::Scalar(0, 0, 256));
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