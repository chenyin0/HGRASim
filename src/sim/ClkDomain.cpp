#include "ClkDomain.h"

namespace Simulator::Array
{
	uint ClkDomain::_clk = 0;


	uint ClkDomain::getClk()
	{
		return _clk;
	}

	void ClkDomain::selfAdd()
	{
		_clk++;
	}

	void ClkDomain::setClk(uint clk_)
	{
		_clk = clk_;
	}

	void ClkDomain::initClk()
	{
		setClk(0);
	}
}
