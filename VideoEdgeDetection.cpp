#include "stdafx.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <algorithm>
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
struct Bar { double len = 0; };
struct Result { Point point; Bar bar; };
struct PolarVec { double len = 0; double angle = 0; };
struct Data { PolarVec polarVec; Point point; };
constexpr size_t N = 360;

template<class T, size_t N>
size_t calc_index(double angle, std::array<T, N> const& arr)
{
	double factor = (angle + 180.0) / 360.0;
	if (factor < 0)
		factor = 0;
	size_t index = size_t(factor * double(arr.size()));
	if (index >= arr.size())
		index = arr.size() - 1;
	return index;
}

std::array<Result, N> calculate_orientation(cv::Mat const& gradient)
{
	std::vector<Data> directions;
	directions.reserve(gradient.rows* gradient.cols);
	const cv::Vec3b* currentRow;
	std::array<double, N> sumLen;
	for (auto& d : sumLen)
		d = 0.0;
	std::array<Result, N> partials = { {} };
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
			sumLen[calc_index(angle, partials)] += len;
		}
	}
	std::array<int, N> num;
	for (auto& i : num)
		i = 0;
	for (const auto& dir : directions)
	{
		size_t index = calc_index(dir.polarVec.angle, partials);
		partials[index].point.x += dir.point.x * dir.polarVec.len/ sumLen[index];
		partials[index].point.y += dir.point.y * dir.polarVec.len / sumLen[index];
		partials[index].bar.len = sumLen[index];
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

template<size_t N>
void draw_bars(cv::Mat& result, std::array<Result, N> partials)
{
	double sum = 0;
	for (const auto& bar : partials)
		sum += bar.bar.len;
	for (auto& bar : partials)
	{
		bar.bar.len /= sum;
		bar.bar.len *= 100.0; // in percent
	}
	size_t nth = 5;
	auto partials_copy = partials;
	auto nth_it = partials_copy.begin() + nth;
	std::nth_element(partials_copy.begin(), nth_it,  partials_copy.end(), [](Result const& l, Result const& r) { return l.bar.len > r.bar.len; });
	double max = partials_copy[nth].bar.len;
	int barBreadth = int(double(result.rows) / double(N));
	for (size_t i = 0; i < N; ++i)
	{
		cv::Point tl{ int(i) * barBreadth, 0 };
		Bar& bar = partials[i].bar;
		cv::Point br{ int(i + 1) * barBreadth, int((bar.len / max) * (double(result.cols) / 4)) };
		cv::rectangle(result, cv::Rect(tl, br), cv::Scalar(0, 0, 256));
	}
}

template<int N>
std::array<Result, N> subtract_results(std::array<Result, N> const& current, std::array<Result, N> const& previous)
{
	auto result = current;
	for (size_t i = 0; i < result.size(); ++i)
		result[i].bar.len -= previous[i].bar.len;
	auto minIt = std::min_element(result.begin(), result.end(), [](const auto& l, const auto& r) { return l.bar.len < r.bar.len; });
	if (minIt->bar.len >= 0)
		return result;
	for (size_t i = 0; i < result.size(); ++i)
		result[i].bar.len -= minIt->bar.len;
	return result;
}

template<int N>
std::array<Result, N> smooth_results(std::array<Result, N> const& partials, int M)
{
	auto result = partials;
	for (size_t i = 0; i < result.size(); ++i)
	{
		double sum = 0;
		for (size_t j = i - M; j <= i; ++j)
		{
			size_t index = j;
			if (index < 0)
				index += N;
			sum += partials[index].bar.len;
		}
		result[i].bar.len = sum / double(M);
	}
	return result;
}

cv::Mat smooth_angles(cv::Mat const& angles, int rings, bool onlyRecordAngles)
{
	cv::Mat result = angles;
	std::vector<const cv::Vec3b*> rows;
	cv::Vec3b* resultRow = nullptr;
	for (size_t i = 0; i < 2 * rings + 1; ++i)
		rows.push_back(nullptr);
	for (int i = rings; i < angles.rows - rings; ++i)
	{
		resultRow = result.ptr<cv::Vec3b>(i);
		size_t index = 0;
		for (size_t j = i - rings; j < i + rings + 1; ++j)
			rows[index++] = angles.ptr<cv::Vec3b>(j);
		for (int j = rings; j < angles.cols - rings; ++j) 
		{
			double sumAngle = 0;
			double sumLen = 0;

			for (int k = 0; k < int(rows.size()); ++k)
				for(int l = j - rings; l < j + rings + 1; ++l)
				{

					double len = double(rows[k][l][0]);
					if (len > 0) 
					{
						sumLen += len;
						if (rows[k][l][1] > 0)
							sumAngle += len * rows[k][l][1];
						else
							sumAngle += len * rows[k][l][2];
					}
				}
			if (sumLen == 0)
			{
				resultRow[j][0] = 0;
				resultRow[j][1] = 0;
				resultRow[j][2] = 0;
			}
			else
			{
				int nb = 2 * rings + 1;
				nb *= nb; // squared
				double angle = sumAngle / sumLen;
				double len = sumLen / nb;
				if (!onlyRecordAngles)
					resultRow[j][0] = int(len);
				else
					resultRow[j][0] = 0;
				if (angle > 0)
				{
					resultRow[j][1] = int(angle);
				}
				else
				{
					resultRow[j][2] = int(-angle);
				}
			}
		}
	}
	return result;
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
	std::array<Result, N> previous;
	while (true)
	{
		cv::Mat imgOriginal;
		int retflag;
		readImageData(cap, imgOriginal, retflag);
		if (retflag == 2) break;
		cv::Mat contours = od::detect_angles(imgOriginal);
		cv::Mat gradient = od::detect_directions(imgOriginal);
		int rings = 1;
		auto smoothed_contours = smooth_angles(gradient, rings, true);
		auto smoothed_gradient = smooth_angles(gradient, rings, false);
		
		auto partials = calculate_orientation(gradient);
		//for (const auto& partial : partials)
		//	cv::circle(contours, cv::Point(int(partial.point.x), int(partial.point.y)), 5, cv::Scalar(0, 0, 256));
		if (j++ == 0)
			previous = partials;
		//auto diff = subtract_results(partials, previous);
		draw_bars(contours, partials);
		previous = partials;


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