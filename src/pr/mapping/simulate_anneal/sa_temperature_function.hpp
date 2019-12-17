#pragma once

namespace Simulator::Mpr
{
	enum class TemperatureFunType
	{
		Slow,
		Fast
	};

	class TemperatureFunDataBase
	{
	public:
		auto getFunction(TemperatureFunType type_) -> function<double(double, double)>
		{
			if (type_ == TemperatureFunType::Fast)
				return std::bind(&TemperatureFunDataBase::funFast, &*this, std::placeholders::_1, std::placeholders::_2);
			else if (type_ == TemperatureFunType::Slow)
				return std::bind(&TemperatureFunDataBase::funSlow, &*this, std::placeholders::_1, std::placeholders::_2);;
			return nullptr;
		}

	private:
		double funSlow(double temperature_, double alpha_) const
		{
			if (alpha_ > 0.96)
				return temperature_ * 0.5;
			else if (alpha_ > 0.8)
				return temperature_ * 0.95;
			else if (alpha_ > 0.15)
				return temperature_ * 0.99;
			else
				return temperature_ * 0.9;
		}

		double funFast(double temperature_, double alpha_) const
		{
			if (alpha_ > 0.96)
				return temperature_ * 0.5;
			else if (alpha_ > 0.8)
				return temperature_ * 0.9;
			else if (alpha_ > 0.15)
				return temperature_ * 0.95;
			else
				return temperature_ * 0.8;
		}
	};

}