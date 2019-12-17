#pragma once

#include "../../preprocess/preprocess.h"

namespace Simulator::Mpr
{
	using Preprocess::ArchBlock;
	using Preprocess::PortIndex;
	using Preprocess::PortPos;

	struct Interval final
	{
		uint left;
		uint right;

		explicit Interval(uint a_ = 0, uint b_ = 0)
			: left(a_)
			, right(b_)
		{
		}

		bool operator==(const Interval& b_) const
		{
			return (left == b_.left && right == b_.right);
		}

		void swap(Interval& a_, Interval& b_) const noexcept
		{
			using std::swap;
			swap(a_.left, b_.left);
			swap(a_.right, b_.right);
		}

		explicit operator string() const
		{
			auto s = "[" + std::to_string(left) + ", " + std::to_string(right) + ")";
			return s;
		}
	};

}

