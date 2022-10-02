#include "stdafx.h"

#include "Algo.h"

#include <algorithm>
#include <cmath>

namespace od {
	void BarMatrix::addBars(std::vector<double> values)
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

		// second idea for an algorithm
		// a precondition is that both seqences are sorted, so if they both have the same size
		// one does not need to examine the maxima in any way
		// if the sequences diverge in size then we take the size difference as lookahead size
		// and we insert the maximum with the least amount of wandering as the mapped element,
		// the skipped maxima are the ones that newly emerged or that disappeared
		if (previous.size() == next.size()) 
		{
			for (size_t i = 0; i < previous.size(); ++i)
			{
				// return one to one mapping as all the maxima are sorted
				ret.push_back({ i, i });
			}
			return ret;
		}
		auto& shorter = previous.size() < next.size() ? previous : next;
		auto& longer = previous.size() < next.size() ? next : previous;
		size_t N = longer.size() - shorter.size();
		size_t actualShift = 0;
		for (size_t i = 0; i < shorter.size(); ++i)
		{
			auto it = std::min_element(longer.begin() + i + actualShift,
				longer.begin() + i + N,
				[shorter, i](const auto& l, const auto& r) {
					return std::abs(static_cast<int>(l.mid) - static_cast<int>(shorter[i].mid)) 
						< std::abs(static_cast<int>(r.mid) - static_cast<int>(shorter[i].mid));
				});
			actualShift += std::distance(longer.begin() + i + actualShift, it);
			ret.push_back({ i, i + actualShift });
		}
		if (shorter.size() == next.size()) {
			for (auto& pr : ret) 
			{
				std::swap(pr.first, pr.second);
			}
		}
		return ret;
	}

	void BarMatrix::calculateExtremes()
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

	BarMatrix::BarMatrix(size_t len)
		: len_(len)
	{
	}
}