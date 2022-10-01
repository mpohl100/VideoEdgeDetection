#pragma once

#include <unordered_map>
#include <vector>

namespace od {


enum class ExtremeType {
	Minimum,
	Maximum,
};

template<size_t barrier = 0>
struct LocalExtreme {
	LocalExtreme() = default;
	LocalExtreme(LocalExtreme const&) = default;
	LocalExtreme& operator=(LocalExtreme const&) = default;
	LocalExtreme(LocalExtreme&&) = default;
	LocalExtreme& operator=(LocalExtreme&&) = default;

	LocalExtreme(ExtremeType type, size_t mid)
		: mid(mid)
		, type(type)
	{}

	size_t mid = 0;
	size_t begin = 0;
	size_t end = 0;
	ExtremeType type = ExtremeType::Minimum;
};

template<size_t barrier>
std::vector<LocalExtreme<barrier>> find_extremes(std::vector<double> const& values, ExtremeType type, bool calc_neighbours = true)
{
	if constexpr (barrier == 0) {
		if (type == ExtremeType::Minimum)
		{
			std::vector<LocalExtreme<0>> minima;
			for (size_t i = 0; i < values.size() - 2; ++i) {
				double left = values[i];
				double mid = values[i + 1];
				double right = values[i + 2];
				if (left > mid and right > mid)
					minima.push_back(LocalExtreme<0>(ExtremeType::Minimum,i + 1));
			}
			if (calc_neighbours) {
				auto maxima = find_extremes<0>(values, ExtremeType::Maximum, false);
				// fill in left and right of barrier 0 for minima
				for (auto minIt = minima.begin(); minIt != minima.end(); ++minIt)
				{
					auto left_maximum = std::lower_bound(maxima.begin(), maxima.end(), minIt->mid,
						[](LocalExtreme<0> const& extreme, size_t mid) { return extreme.mid < mid; });
					if (left_maximum == maxima.end())
						left_maximum = maxima.end() - 1;
					auto right_maximum = left_maximum + 1;
					if (right_maximum == maxima.end())
						right_maximum = maxima.begin();
					minIt->begin = left_maximum->mid;
					minIt->end = right_maximum->mid;
				}
			}
			return minima;
		}
		else
		{
			std::vector<LocalExtreme<0>> maxima;
			for (size_t i = 0; i < values.size() - 2; ++i) {
				double left = values[i];
				double mid = values[i + 1];
				double right = values[i + 2];
				if (left < mid and right < mid)
					maxima.push_back(LocalExtreme<0>(ExtremeType::Maximum, i + 1));
			}
			if (calc_neighbours) {
				auto minima = find_extremes<0>(values, ExtremeType::Minimum, false);
				// fill in left and right of barrier 0 for maxima
				for (auto maxIt = maxima.begin(); maxIt != maxima.end(); ++maxIt)
				{
					auto left_minimum = std::lower_bound(minima.begin(), minima.end(), maxIt->mid,
						[](LocalExtreme<0> const& extreme, size_t mid) { return extreme.mid < mid; });
					if (left_minimum == minima.end())
						left_minimum = minima.end() - 1;
					auto right_minimum = left_minimum + 1;
					if (right_minimum == minima.end())
						right_minimum = minima.begin();
					maxIt->begin = left_minimum->mid;
					maxIt->end = right_minimum->mid;
				}
			}
			return maxima;
		}
	}
	else
	{
		std::vector<LocalExtreme<0>> minima = find_extremes<0>(values, ExtremeType::Minimum);
		std::vector<LocalExtreme<0>> maxima = find_extremes<0>(values, ExtremeType::Maximum);
		static_assert(false, "LocalExtreme with barrier other than zero was initially planned but not yet implemented.");
	}
}

class BarSequence {
public:
	BarSequence() = default;
	BarSequence(BarSequence const&) = default;
	BarSequence& operator=(BarSequence const&) = default;
	BarSequence(BarSequence&&) = default;
	BarSequence& operator=(BarSequence&&) = default;

	BarSequence(size_t len);

	void addBars(std::vector<double> values);


private:
	void calculateExtremes();
	size_t len_ = 5;
	std::vector<std::vector<double>> bars_;
	std::unordered_map<int, std::vector<LocalExtreme<0>>> extremes_;
};

}
