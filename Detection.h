#pragma once

#include "opencv2/core/mat.hpp"
namespace od {

	class DirPoint3D {
	public:
		DirPoint3D(float x, float y, float val);
	private:
		uchar x_;
		uchar y_;
		uchar val_;
	};

	/**
	 * returns the gradient over a pixel
	 * first calculate the x and y gradients over the pixel cc
	 * then return the sqrt(grad_x**2 + grad_y**2)
	 */
	int gradient(int tl, int tc, int tr, int cl, int cr, int bl, int bc, int br);
	/**
	 * returns a gray scale Mat, takes an BGR / RGB Mat
	 * iterate over all pixels and calculate the color gradient over it
	 * the gray scale return matrix represents the ratio of the biggest color channel in relation to the sum of all color channels
	 * a white pixel means a super high descent and a black pixel means a planar surface
	 */
	cv::Mat detect_edges(cv::Mat const& img);

	struct Directions {
		cv::Mat x_dir;
		cv::Mat y_dir;
		cv::Mat val;
		Directions(cv::Mat const img);
	};
}