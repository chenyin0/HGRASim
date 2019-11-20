#include "module.h"
#include <iomanip>
#include "../ClkDomain.h"
#include "../debug.h"
using namespace Simulator::Array;

//对于每周期operation可变的alu，需要按序完成
Pealu::Pealu()
{
	DEBUG_ASSERT(false);
}
void Pealu::print()
{
	if (pipeline.size() != 0) {
		for (auto& trans : pipeline) {
			Debug::getInstance()->getRegFile() << "trans data " << trans.data1 << "  " << trans.data2 <<"  "<<trans.data3 << std::endl;;
		}
	}
}
Pealu::Pealu(Simulator::PEOpcode opcode) {
	pipeline.resize(0);
	switch (opcode)
	{
	case PEOpcode::null:
	//	DEBUG_ASSERT(false);
		depth= 0;
		break;
	case PEOpcode::add:
		depth = 1;
		break;
	case PEOpcode::mul:
		depth = 3;
		break;
	case PEOpcode::sub:
		depth = 1;
		break;
	case PEOpcode::div:
		depth = 3;
		break;
	case PEOpcode::mod:
		depth = 3;
		break;
	case PEOpcode::lt:
		depth = 1;
		break;
	case PEOpcode::eq:
		depth = 1;
		break;
	case PEOpcode::mux:
		depth = 1;
		break;
	case PEOpcode::mac:
		depth = 4;
		break;
	case PEOpcode::add1:
		depth = 1;
		break;
	case PEOpcode::leshift:
		depth = 1;
		break;
	case PEOpcode::rishift:
		depth = 1;
		break;
	case PEOpcode::bit_and:
		depth = 1;
		break;
	case PEOpcode::bit_or:
		depth = 1;
		break;
	case PEOpcode::cos:
		depth = 10;
		break;
	case PEOpcode::sin:
		depth = 10;
		break;
	case PEOpcode::bit_xor:
		depth = 1;
		break;
	case PEOpcode::smux:
		depth = 1;
		break;
	case PEOpcode::neq:
		depth = 1;
		break;
	case PEOpcode::lte:
		depth = 1;
		break;
	case PEOpcode::bit_not:
		depth = 1;
		break;
	case PEOpcode::merge:
		depth = 1;
		break;
	case PEOpcode::hlt:
		depth = 0;
		break;
	case PEOpcode::hadd:
		depth = 0;
		break;
	default:
		DEBUG_ASSERT(false);
		depth = 0;
		break;
	}
}

bool Pealu::canReceiveInput()
{
	return (pipeline.size() < depth);
}

bool Pealu::input(vector<Port_inout> alu_in, Simulator::PEOpcode opin,bool nolast,bool rs_cd)//应该先出trans再进trans，否则前面的消耗不掉
{
	if (pipeline.size() < depth)
	{
		newtrans.valid = nolast;
		newtrans.cycle = 0;
		newtrans.data1 = alu_in[0].value_data;
		newtrans.data2 = alu_in.size()>1?alu_in[1].value_data:0;
		newtrans.data3 = alu_in.size() > 2 ? alu_in[2].value_data : 0;
		newtrans.opcode = opin;
		newtrans.delay = getDelay(opin);
		newtrans.rs_cd = rs_cd;

		pipeline.push_back(newtrans);
		return true;
	}
	else 
		return false;
}

uint Pealu::getDelay(Simulator::PEOpcode opin)
{
	switch (opin)
	{
	case PEOpcode::null:
	//	DEBUG_ASSERT(false);
		return 1;

	case PEOpcode::add:
		return 1;

	case PEOpcode::mul:
		return 3;

	case PEOpcode::sub:
		return 1;

	case PEOpcode::div:
		return 3;

	case PEOpcode::mod:
		return 3;

	case PEOpcode::lt:
		return 1;

	case PEOpcode::eq:
		return 1;

	case PEOpcode::mux:
		return 1;

	case PEOpcode::mac:
		return 4;

	case PEOpcode::add1:
		return 1;

	case PEOpcode::leshift:
		return 1;

	case PEOpcode::rishift:
		return 1;

	case PEOpcode::bit_and:
		return 1;

	case PEOpcode::bit_or:
		return 1;

	case PEOpcode::cos:
		return 10;

	case PEOpcode::sin:
		return 10;

	case PEOpcode::bit_xor:
		return 1;

	case PEOpcode::smux:
		return 1;

	case PEOpcode::neq:
		return 1;

	case PEOpcode::lte:
		return 1;

	case PEOpcode::bit_not:
		return 1;

	case PEOpcode::merge:
		return 1;

	default:
		DEBUG_ASSERT(false);
		return 0;
	}
}
/*
bool Pealu::output(Port_inout& out)
{
	if (pipeline.front().valid && (pipeline.front().cycle == pipeline.front().delay))
	{
		
	}
}
*/
void Pealu::compute(Port_inout& out)
{
	if (!pipeline.empty())
	{
		if (pipeline.front().cycle == pipeline.front().delay-1)//trans刚进pipeline的时候就是0的delay了
		{
			if (pipeline.front().rs_cd || !pipeline.front().valid) {
				if (pipeline.front().rs_cd) {
					out.valid = true;
					out.condition = false;
					out.value_data = pipeline.front().data2;
				}
				if (!pipeline.front().valid) {
					out.valid = true;
					out.last = true;
					out.value_data = pipeline.front().data2;
				}
			}
			else {
				switch (pipeline.front().opcode)
				{
				case PEOpcode::null:
					out.valid = true;
					out.value_data = pipeline.front().data2;
					break;

				case PEOpcode::add:
					out.valid = true;
					out.value_data = pipeline.front().data1 + pipeline.front().data2;
					break;

				case PEOpcode::mul:
					out.valid = true;
					out.value_data = pipeline.front().data1 * pipeline.front().data2;
					break;

				case PEOpcode::sub:
					out.valid = true;
					out.value_data = pipeline.front().data1 - pipeline.front().data2;
					break;

				case PEOpcode::div:
					out.valid = true;
					out.value_data = pipeline.front().data1 / pipeline.front().data2;
					break;

				case PEOpcode::mod:
					out.valid = true;
					out.value_data = pipeline.front().data1 % pipeline.front().data2;
					break;

				case PEOpcode::lt:
					out.valid = true;
	//				out.value_bool = pipeline.front().data1 < pipeline.front().data2;
					out.value_data = pipeline.front().data1 < pipeline.front().data2;
					break;

				case PEOpcode::eq:
					out.valid = true;
	//				out.value_bool = pipeline.front().data1 == pipeline.front().data2;
					out.value_data = pipeline.front().data1 == pipeline.front().data2;
					break;

				case PEOpcode::mux:
					out.valid = true;
	//				out.value_data = pipeline.front().value_bool3 ? pipeline.front().data2 : pipeline.front().data1;
					out.value_data = pipeline.front().data3 ? pipeline.front().data1 : pipeline.front().data2;
					break;

				case PEOpcode::mac:
					out.valid = true;
					out.value_data = pipeline.front().data1 * pipeline.front().data2 + pipeline.front().data3;
					break;

				case PEOpcode::add1:
					out.valid = true;
					out.value_data = pipeline.front().data2 + 1;
					break;

				case PEOpcode::leshift:
					out.valid = true;
					out.value_data = pipeline.front().data1 << pipeline.front().data2;
					break;

				case PEOpcode::rishift:
					out.valid = true;
					out.value_data = pipeline.front().data1 >> pipeline.front().data2;
					break;

				case PEOpcode::bit_and:
					out.valid = true;
					out.value_data = pipeline.front().data1 & pipeline.front().data2;
					break;

				case PEOpcode::bit_or:
					out.valid = true;
					out.value_data = pipeline.front().data1 | pipeline.front().data2;
					break;

				case PEOpcode::cos:
					out.valid = true;
					out.value_data = cos(pipeline.front().data2);
					break;

				case PEOpcode::sin:
					out.valid = true;
					out.value_data = sin(pipeline.front().data2);
					break;

				case PEOpcode::bit_xor:
					out.valid = true;
					out.value_data = pipeline.front().data1 ^ pipeline.front().data2;
					break;

				case PEOpcode::smux:
					out.valid = true;
					out.value_data = bool(pipeline.front().data3) ? pipeline.front().data2 : pipeline.front().data1;
					break;

				case PEOpcode::neq:
					out.valid = true;
					out.value_data = pipeline.front().data2 != pipeline.front().data1;
					break;

				case PEOpcode::lte:
					out.valid = true;
					out.value_data = pipeline.front().data2 <= pipeline.front().data1;
					break;

				case PEOpcode::bit_not:
					out.valid = true;
					out.value_data = !pipeline.front().data2;
					break;

				case PEOpcode::merge:
					out.valid = true;
					out.value_data = ~pipeline.front().data2;
					break;

				default:
					DEBUG_ASSERT(false);
					break;
				}
			}
			//pipeline.erase(pipeline.begin());             ack机制清数而不是credit
		}
	}
}

bool Pealu::empty() const
{
	return (pipeline.size()==0);
}

void Pealu::pop()
{
	DEBUG_ASSERT(!pipeline.empty());//////////////////////////////////////////////有问题/////////////////////////
	pipeline.erase(pipeline.begin());
}

void Pealu::update()
{
	//vector<Bool> stage_occupied(pipeline.size() + 1, false);
	for (auto i = pipeline.begin(); i < pipeline.end(); ++i)
	{
		if ((*i).cycle < (*i).delay-1)
		{
			bool stage_occupied = false;
  /*			for (auto j = pipeline.begin(); j < i; ++j)
				if ((*j).cycle == (*i).cycle + 1)
					stage_occupied = true;
			if (!stage_occupied)*/
				(*i).cycle++;
		}
		DEBUG_ASSERT((*i).cycle <= (*i).delay);
	}
}

//credit protocol
void Localreg::input(Port_inout input)
{
	uint clk = ClkDomain::getInstance()->getClk();
//	DEBUG_ASSERT(!reg_v);/////////////////需要去掉注释
//	if (input.valid && !input.last&&input.condition) {
	if (input.valid && input.condition) {
		value = input.value_data;
		reg_v = input.valid;
	}
}

void Localreg::input_transout(Port_inout input)
{
	uint clk = ClkDomain::getInstance()->getClk();
	//	DEBUG_ASSERT(!reg_v);/////////////////需要去掉注释
	if (input.valid && input.last && input.condition) {
		value = input.value_data;
		reg_v = input.valid;
	}
}

void Localreg::output(Port_inout& output)
{
	if (reg_v)
	{
		output.value_data = value;
		output.valid = reg_v;
	}
}

void Localreg::setvalue(int value)
{
	this->value = value;
	reg_v = true;
}

void Localreg::reset()
{
	reg_v = false;
}

void Localreg::print(std::ofstream& file) const
{
	//file << std::setw(20) << "valid control data bool" << std::endl;
	file << "----LocalReg Element----" << std::endl;
	file << std::setw(20) << "valid control data bool" << std::endl;
	file.width(15);
	file << "valid";
	file.width(15);
	file << "value";
	file << std::endl;

	file << std::setw(15) << this->reg_v;
	file << std::setw(15) << this->value;
	file << std::endl;
}

