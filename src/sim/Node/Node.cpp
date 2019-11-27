#include "Node.h"

using namespace Simulator::Array;

Node::Node(const Simulator::Preprocess::ArrayPara &para)
{
	system_parameter = para;
	//print_screen= system_parameter.print_screen;
	//uint clk = Simulator::Array::ClkDomain::getInstance()->getClk();
	//if (Simulator::Array::ClkDomain::getInstance()->getClk() >= Simulator::Array::Debug::getInstance()->print_screen_begin
	//	&& Simulator::Array::ClkDomain::getInstance()->getClk() < Simulator::Array::Debug::getInstance()->print_screen_end)
	//	print_screen = system_parameter.print_screen;
	////else
	//	print_screen = false;
}

