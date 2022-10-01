#include "stdafx.h"

#include "Algo.h"

namespace od {
	void BarSequence::addBars(std::vector<double> values)
	{
		if (bars_.size() >= len_) 
		{
			bars_.erase(bars_.begin());
		}
		bars_.push_back(std::move(values));
		calculateExtremes();
	}
	
	std::vector<std::pair<size_t, size_t>> mapIndeces(std::vector<LocalExtreme<0>> const& previous, std::vector<LocalExtreme<0>> const& next)
	{
		std::vector<std::pair<size_t, size_t>> ret;
		// compare two rows of maxima against each other
		// firstly for every item in previous, search in next wether this maximum 
		// has not changed with an accuracy of two or lower being regarded as no change
		// don't forget to mark all unchanged indeces
		// secondly we assume that the relative order of the maxima has not changed
		// and analyse with a rotate of the remaining next maxima, which configuration 
		// provides the least movement of the maxima
	}

	void BarSequence::calculateExtremes()
	{
		extremes_ = {}; // todo optimize to minimal work
		// firstly we enter the maxima of the first bar vector into the extremes container
		{
			auto maxima = find_extremes<0>(bars_.front(), ExtremeType::Maximum);
			for (size_t i = 0; i < maxima.size() - 1; ++i)
			{
				extremes_[static_cast<int>(i)].push_back(maxima[i]);
			}
		}
		// then we check the next row of maxima against the previous one 

	}

	BarSequence::BarSequence(size_t len)
		: len_(len)
	{
	}
}