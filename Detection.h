#pragma once

#include "opencv2/core/mat.hpp"
#include <cmath>
namespace od {

	cv::Mat detect_directions(cv::Mat const& bgrImg);
	cv::Mat detect_edges_gray(cv::Mat const& bgrImg);
	cv::Mat detect_angles(cv::Mat const& bgrImg);
}