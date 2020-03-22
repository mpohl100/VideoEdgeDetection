#pragma once

#include "opencv2/core/mat.hpp"
namespace ant {

	class Ant 
	{
	public:
		Ant(cv::Mat const& img);
		int walk();
	private:

	};

}