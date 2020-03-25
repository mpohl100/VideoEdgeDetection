#include "stdafx.h"
#include "Ant.h"
#include <iostream>
#include <opencv2\imgproc.hpp>


namespace od 
{
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
		if (false) {
			std::cout << "gradient of:\n";
			std::cout << tl << " " << tc << " " << tl << '\n';
			std::cout << cl << " " << cc << " " << cl << '\n';
			std::cout << bl << " " << bc << " " << bl << '\n';
			std::cout << "grad_x = " << grad_x << "; grad_y = " << grad_y << "\n";
			std::cout << "grad = " << grad << '\n';
		}
		return grad;
	}

	cv::Mat detect_edges(cv::Mat const& img)
	{
		cv::Mat ret;
		cv::cvtColor(img, ret, cv::COLOR_BGR2GRAY);
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
						imgCenter[j - 1][color], imgCenter[j][color], imgCenter[j + 1][color],
						imgLower[j - 1][color], imgLower[j][color], imgLower[j + 1][color]);
					max = grad_c > max ? grad_c : max;
					sum += grad_c;
				}
				float val = float(max) / float(sum);
				val = std::sqrt(val);
				val *= max;
				ret.at<uchar>(i, j) = int(val);
			}
		}
		return ret;
	}
}
