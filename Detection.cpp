#include "stdafx.h"
#include "Ant.h"
#include <iostream>
#include <opencv2\imgproc.hpp>
#include "Detection.h"


namespace od 
{
	std::tuple<int,int,int> gradient(int tl, int tc, int tr, int cl, int cr, int bl, int bc, int br)
	{
		float grad_tl_br = br - tl;
		float grad_cl_cr = cr - cl;
		float grad_bl_tr = tr - bl;
		float grad_bc_tc = tc - bc;
		float sqrt2 = std::sqrt(2.0);
		float grad_x = grad_cl_cr + grad_tl_br * sqrt2 + grad_bl_tr * sqrt2;
		float grad_y = -grad_bc_tc + grad_tl_br * sqrt2 - grad_bl_tr * sqrt2;
		float grad_total = std::sqrt(grad_x * grad_x + grad_y * grad_y);
		if (false) {
			std::cout << "gradient of:\n";
			std::cout << tl << " " << tc << " " << tl << '\n';
			std::cout << cl << " " << "center" << " " << cl << '\n';
			std::cout << bl << " " << bc << " " << bl << '\n';
			std::cout << "grad_x = " << grad_x << "; grad_y = " << grad_y << "\n";
			std::cout << "grad = " << grad_total << '\n';
		}
		return { int(grad_total), int(grad_x), int(grad_y),  };
	}

	cv::Mat detect_edges(cv::Mat const& img)
	{
		Directions dir(img);
		return dir.val;
	}

	Directions::Directions(cv::Mat const img)
	{
		cv::cvtColor(img, x_dir, cv::COLOR_BGR2GRAY);
		cv::cvtColor(img, y_dir, cv::COLOR_BGR2GRAY);
		cv::cvtColor(img, val, cv::COLOR_BGR2GRAY);
		const cv::Vec3b* imgUpper;
		const cv::Vec3b* imgCenter;
		const cv::Vec3b* imgLower;
		for (int i = 1; i < img.rows - 1; ++i)
		{
			imgUpper = img.ptr<cv::Vec3b>(i - 1);
			imgCenter = img.ptr<cv::Vec3b>(i);
			imgLower = img.ptr<cv::Vec3b>(i + 1);
			for (int j = 1; j < img.cols - 1; ++j)
			{
				int max = 0;
				int sum = 0;
				for (int color = 0; color <= 2; color++)
				{
					int grad_c = gradient(imgUpper[j - 1][color], imgUpper[j][color], imgUpper[j + 1][color],
						imgCenter[j - 1][color], imgCenter[j + 1][color],
						imgLower[j - 1][color], imgLower[j][color], imgLower[j + 1][color]);
					max = grad_c > max ? grad_c : max;
					sum += grad_c;
				}
				float value = float(max) / float(sum);
				value = std::sqrt(value);
				value *= max;
				val.at<uchar>(i, j) = int(value);
			}
		}
	}

	DirPoint3D::DirPoint3D(float x, float y, float val)
		: x_(int((x / 2) + 128) )
		, y_(int((y / 2) + 128))
		, val_(int(val))
	{

	}
}
