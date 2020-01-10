#include "buffer.h"
#include <iomanip>
using namespace Simulator::Array;

void Buffer::print(std::ofstream& file)
{
	file << "----Buffer Element----" << std::endl;
	file <<std::setw(20)<< "valid control data bool" << std::endl;
	for (uint i = 0; i < entity.size(); i++)
	{
		for (uint j = 0; j < entity[i].size(); j++)
		{
			file << "bufferdep" << std::setw(2) << entity[i][j].valid;
			file << std::setw(4) << entity[i][j].condition << entity[i][j].last;
			file << std::setw(4) << entity[i][j].valued;
			file << std::setw(4) << entity[i][j].valueb;
			file << "      ";
		}
		file << std::endl;	
	}
	file << "head_ptr" << head_ptr[0] ;
	file << std::endl;
}
bool Buffer::haveFalseC(uint port)
{
	if (entity[head_ptr[port]][port].valid && !entity[head_ptr[port]][port].condition) {
		return true;
	}
	return false;
}
void Buffer::print_valid(std::ofstream& file)
{
	file << "----Buffer Element----" << std::endl;
	file << std::setw(20) << "valid control data bool" << std::endl;
	for (uint i = 0; i < entity.size(); i++)
	{
		bool some_valid = false;
		for (uint j = 0; j < entity[i].size(); j++)
		{
			if (entity[i][j].valid) {
				file << "bufferdep" <<i<< std::setw(2) << entity[i][j].valid;
				file << std::setw(4) << entity[i][j].condition << entity[i][j].last;
				file << std::setw(7) << entity[i][j].valued;
				file << std::setw(4) << entity[i][j].valueb;
				some_valid = true;
				file << "      ";
			}
		}
		if(some_valid)
			file << std::endl;
	}
	file << "head_ptr" << head_ptr[0];
	file << std::endl;
}
 
Buffer_in::Buffer_in(const Simulator::Preprocess::ArrayPara para):Buffer(para)
{
}

bool Buffer_in::input(Port_inout input, const uint port)
{
	if (input.valid)
		if (canReceiveInput(port))
		{
			bufferInput(input, port);
			return true;
		}
	return false;
}
bool Buffer_in::input_nolast(Port_inout input, const uint port) {
	if (input.valid&& input.last)
		if (canReceiveInput(port))
		{
			entity[tail_ptr[port]][port].valid = input.valid;
			entity[tail_ptr[port]][port].valueb = input.value_bool;
			entity[tail_ptr[port]][port].valued = input.value_data;
			entity[tail_ptr[port]][port].condition = input.condition;
//			entity[tail_ptr[port]][port].last = input.last;

			update_tail(port);
			return true;
		}
	return false;
}
bool Buffer_out::input_nolast(Port_inout input, const uint port) {
	DEBUG_ASSERT(false);
	return false;
}
bool Buffer_in::input_tag_lsu(Port_inout_lsu input, uint port, uint tag)
{
	DEBUG_ASSERT(false);
	return false;
}
bool Buffer_in::input_lsu(Port_inout_lsu input, uint port)
{
	DEBUG_ASSERT(false);
	return false;
}
bool Buffer_in::input_tag(Port_inout input, uint port, uint tag)
{
	DEBUG_ASSERT(true);
	return false;
}

void Buffer_in::output(Port_inout& output, uint port)
{
	if (entity[head_ptr[port]][port].valid)
	{
		output.valid = entity[head_ptr[port]][port].valid;
		output.value_data = entity[head_ptr[port]][port].valued;
		output.value_bool = entity[head_ptr[port]][port].valueb;
		output.condition = entity[head_ptr[port]][port].condition;
		output.last = entity[head_ptr[port]][port].last;

		reset(port);
	}
}

bool Buffer_in::output_ack(Port_inout& output, uint port)
{
	if (entity[head_ptr[port]][port].valid)
	{
		output.valid = entity[head_ptr[port]][port].valid;
		output.value_data = entity[head_ptr[port]][port].valued;
		output.value_bool = entity[head_ptr[port]][port].valueb;
		output.condition = entity[head_ptr[port]][port].condition;
		output.last = entity[head_ptr[port]][port].last;
		return true;
	}
	else {
		return false;
	}
}

void Buffer_in::outputTag(Port_inout& input, uint tag)
{
	DEBUG_ASSERT(true);
}

void Buffer_in::resetTag(uint tag)
{
	DEBUG_ASSERT(false);
}
//credit机制时该函数只需要assertion检错；ack机制时该函数需要负责检查能否进数
bool Buffer_in::canReceiveInput(uint port)
{
	return (!entity[tail_ptr[port]][port].valid);
//	return true;
}


void Buffer_in::bufferInput(Port_inout input, uint port)
{
	entity[tail_ptr[port]][port].valid = input.valid;
	entity[tail_ptr[port]][port].valueb = input.value_bool;
	entity[tail_ptr[port]][port].valued = input.value_data;
	entity[tail_ptr[port]][port].condition = input.condition;
	entity[tail_ptr[port]][port].last = input.last;

	update_tail(port);
}

void Buffer_in::update_tail(uint port)
{
	tail_ptr[port]++;
	if (tail_ptr[port] == depth)
		tail_ptr[port] = 0;

	if (tail_ptr[port] == head_ptr[port])
		port_full[port] = true;
	else
		port_full[port] = false;

	port_empty[port] = false;
}

void Buffer_in::update_head(uint port)
{
	head_ptr[port]++;
	if (head_ptr[port] == depth)
		head_ptr[port] = 0;

	if (head_ptr[port] == tail_ptr[port])
		port_empty[port] = true;
	else
		port_empty[port] = false;

	port_full[port] = false;
}
void Buffer_in::back_head(uint port)
{
	if(head_ptr[port] == 0)
		head_ptr[port]=depth-1;
	head_ptr[port]--;
	if (tail_ptr[port] == head_ptr[port])
		port_full[port] = true;
	else
		port_full[port] = false;

	port_empty[port] = false;
	entity[head_ptr[port]][port].valid = true;
}
void Buffer_out::back_head(uint port)
{
	DEBUG_ASSERT(false);
}
void Buffer_in::reset()
{
	for (auto&i : tail_ptr)
		i = 0;
	for (auto&i : head_ptr)
		i = 0;
	port_empty = { true,true,true };
	port_full = { false,false,false };

	for (auto i = entity.begin(); i < entity.end(); ++i)
	{
		for (auto j = i->begin(); j < i->end(); ++j)
		{
			j->valid = false;
			j->valued = 0;
			j->valueb = false;
			j->condition = true;
			j->last = false;
		}
	}
}

void Buffer_in::reset(uint port)
{
	if (entity[head_ptr[port]][port].valid) {
		entity[head_ptr[port]][port].valid = false;
		entity[head_ptr[port]][port].valueb = false;
		entity[head_ptr[port]][port].condition = true;
		entity[head_ptr[port]][port].valued = 0;
		entity[head_ptr[port]][port].last = false;

		update_head(port);
	}
}

//void Buffer_in::reset_head()
//{
//	for (uint j = 0; j < (system_parameter.bool_inport_breadth + system_parameter.data_inport_breadth); j++)
//	{
//		for (auto& i : entity[head_ptr[j]])
//		{
//			i.valid = false;
//			i.valueb = false;
//			i.valued = false;
//			i.condition = true;
//			i.last = false;
//		}
//
//		head_ptr[j] = 0;
//		tail_ptr[j] = 0;
//		port_empty[j] = true;
//		port_full[j] = false;
//	}
//}
void Buffer_in::reset_head()
{
	for (uint j = 0; j < (system_parameter.bool_inport_breadth + system_parameter.data_inport_breadth); j++)
	{
		for (auto& i : entity[head_ptr[j]])
		{
			i.valid = false;
			i.valueb = false;
			i.valued = false;
			i.condition = true;
			i.last = false;
		}

		head_ptr[j] = 0;
		tail_ptr[j] = 0;
		port_empty[j] = true;
		port_full[j] = false;
	}
}

bool Buffer_in::empty()
{
	bool buffer_empty = true;
	for (uint i = 0; i < (databreadth + boolbreadth); ++i)
	{
		buffer_empty = buffer_empty && port_empty[i];
	}
	return buffer_empty;
}

bool Buffer_in::isBufferNotFull(uint port)
{
	return !port_full[port];
}

bool Buffer_in::isInputSuccess(uint port)
{
	return ack[port];
}

void Buffer_in::update()
{
	//ack = false;
}

bool Buffer_in::isBufferNotEmpty(uint port)
{
	return !port_empty[port];
}

void Buffer_in::getTagMatchIndex(vector<uint>& vec, uint port,uint tag)
{
	DEBUG_ASSERT(true);
}

Bufferpe_in::Bufferpe_in(const Simulator::Preprocess::ArrayPara para):Buffer_in(para)
{
	depth = system_parameter.pe_in_buffer_depth;
	databreadth = system_parameter.data_inport_breadth;
	boolbreadth = system_parameter.bool_inport_breadth;
	entity.resize(depth);
	//boolbuffer.resize(depth);

	for (auto it = entity.begin(); it < entity.end(); ++it)
		it->resize(databreadth + boolbreadth);

	tail_ptr.resize(databreadth + boolbreadth);
	head_ptr.resize(databreadth + boolbreadth);
	port_full.resize(databreadth + boolbreadth);
	port_empty.resize(databreadth + boolbreadth);

	for (auto& i : tail_ptr)
		i = 0;
	for (auto& i : head_ptr)
		i = 0;
	for (auto& i : port_full)
		i = false;
	for (auto& i : port_empty)
		i = true;

	ack = { false };
	//bp = new Creditbp(this);
}


Buffer_out::Buffer_out(const Simulator::Preprocess::ArrayPara para):Buffer(para)
{
	//DEBUG_ASSERT(false);       //仍然是抽象类
	/*
	depth = system_parameter.pe_out_buffer_depth;
	databreadth = system_parameter.data_outport_breadth;
	boolbreadth = system_parameter.bool_outport_breadth;
	entity.resize(depth);

	for (auto it = entity.begin(); it < entity.end(); ++it)
		it->resize(databreadth + boolbreadth);

	tail_ptr.resize(databreadth + boolbreadth);
	head_ptr.resize(databreadth + boolbreadth);
	buffer_empty.resize(databreadth + boolbreadth);
	buffer_full.resize(databreadth + boolbreadth);
	ack.resize(databreadth + boolbreadth);

	tail_ptr = { 0 };
	head_ptr = { 0 };
	buffer_empty = { true };
	buffer_full = { false };
	ack = { false };
	*/
}



bool Buffer_out::input(Port_inout input, const uint port)
{
	if (input.valid)
		if (canReceiveInput(port))
		{
			bufferInput(input, port);
			return true;
		}
	return false;
}

bool Buffer_out::input_tag_lsu(Port_inout_lsu input, uint port, uint tag)
{
	if (input.valid)
	{
		if (canReceiveInput(port, tag))
		{
			bufferInput(input, port, tag);
			return true;
		}
	}
	return false;
}
bool Buffer_out::input_lsu(Port_inout_lsu input, uint port)
{
	if (input.valid)
	{
		if (canReceiveInput(port))
		{
			Port_inout portinp;
			portinp.valid = true;
			portinp.condition = input.condition;
			portinp.last = input.last;
			portinp.value_data= input.value_data;
			portinp.value_bool = false;
			bufferInput(portinp, port);
			return true;
		}
	}
	return false;
}
bool Buffer_out::input_tag(Port_inout input, uint port, uint tag)
{
	if (input.valid)
	{
		if (canReceiveInput(port, tag))
		{
			bufferInput(input, port, tag);
			return true;
		}
	}
	return false;
}

void Buffer_out::output(Port_inout& output, uint port)
{
	if (entity[head_ptr[port]][port].valid)
	{
		output.valid = entity[head_ptr[port]][port].valid;
		output.value_data = entity[head_ptr[port]][port].valued;
		output.value_bool = entity[head_ptr[port]][port].valueb;
		output.condition = entity[head_ptr[port]][port].condition;
		output.last = entity[head_ptr[port]][port].last;

		reset(port);   //credit机制，在输出时直接清数
	}
}

bool Buffer_out::output_ack(Port_inout& output, uint port)
{
	if (entity[head_ptr[port]][port].valid)
	{
		output.valid = entity[head_ptr[port]][port].valid;
		output.value_data = entity[head_ptr[port]][port].valued;
		output.value_bool = entity[head_ptr[port]][port].valueb;
		output.condition = entity[head_ptr[port]][port].condition;
		output.last = entity[head_ptr[port]][port].last;
		return true;
	}
	else {
		return false;
	}
}

void Buffer_out::outputTag(Port_inout& output, uint tag)
{
	DEBUG_ASSERT(true);
}

void Buffer_out::reset(uint port)
{
	entity[head_ptr[port]][port].valid = false;
	entity[head_ptr[port]][port].valueb = false;
	entity[head_ptr[port]][port].condition = true;
	entity[head_ptr[port]][port].valued = 0;
	entity[head_ptr[port]][port].last = false;

	update_head(port);
}

void Buffer_out::reset_head()
{
	DEBUG_ASSERT(false);
}

void Buffer_out::resetTag(uint tag)
{
	DEBUG_ASSERT(false);
}

bool Buffer_out::empty()
{
	//DEBUG_ASSERT(false);
	//return false;
	bool buffer_empty = true;
	for (uint i = 0; i < (databreadth + boolbreadth); ++i)
	{
		buffer_empty = buffer_empty && port_empty[i];
	}
	return buffer_empty;
}

bool Buffer_out::isBufferNotFull(uint port)
{
	return !port_full[port];
}

void Buffer_out::reset()
{
	for (auto& i : tail_ptr)
		i = 0;
	for (auto& i : head_ptr)
		i = 0;
	for (auto& i : port_full)
		i = false;
	for (auto& i : port_empty)
		i = true;

	for (auto i = entity.begin(); i < entity.end(); ++i)
	{
		for (auto j = i->begin(); j < i->end(); ++j)
		{
			j->valid = false;
			j->valued = 0;
			j->valueb = false;
			j->condition = true;
			j->last = false;
		}
	}
}

//outbuffer的输入使用ack机制, 返回的bool值表示ack
bool Buffer_out::canReceiveInput(uint port)
{
	return (!entity[tail_ptr[port]][port].valid);
}

bool Buffer_out::canReceiveInput(uint port, uint tag)
{
	return !entity[tag][port].valid;
}


void Buffer_out::bufferInput(Port_inout input, uint port)
{
	entity[tail_ptr[port]][port].valued = input.value_data;
	entity[tail_ptr[port]][port].valueb = input.value_bool;
	entity[tail_ptr[port]][port].valid = input.valid;
	entity[tail_ptr[port]][port].condition = input.condition;
	entity[tail_ptr[port]][port].last = input.last;

	update_tail(port);
}

//void Buffer_out::bufferInput(Port_inout input, uint port, uint tag)
//{
//	entity[port][tag].valued = input.value_data;
//	entity[port][tag].condition = true;
//	entity[port][tag].valid = input.valid;
//	entity[port][tag].last = false;
//}

void Buffer_out::bufferInput(Port_inout input, uint port, uint tag)
{
	entity[tag][port].valued = input.value_data;
	entity[tag][port].condition = input.condition;
	entity[tag][port].valid = input.valid;
//	entity[tag][port].last = false;////////////////////////////////有问题
	entity[tag][port].last = input.last;
}

void Buffer_out::bufferInput(Port_inout_lsu input, uint port, uint tag)
{
	entity[tag][port].valued = input.value_data;
	entity[tag][port].condition = true;
	entity[tag][port].valid = input.valid;
	entity[tag][port].last = false;
}

void Buffer_out::update_tail(uint port)
{
	tail_ptr[port]++;
	if (tail_ptr[port] == depth)
		tail_ptr[port] = 0;

	if (tail_ptr[port] == head_ptr[port])
		port_full[port] = true;
	else
		port_full[port] = false;

	port_empty[port] = false;
}

void Buffer_out::update_head(uint port)
{
	head_ptr[port]++;
	if (head_ptr[port] == depth)
		head_ptr[port] = 0;

	if (head_ptr[port] == tail_ptr[port])
		port_empty[port] = true;
	else
		port_empty[port] = false;

	port_full[port] = false;
}

bool Buffer_out::isInputSuccess(uint port)
{
	return ack[port];
}

void Buffer_out::update()
{
	//ack = false;
}

bool Buffer_out::isBufferNotEmpty(uint port)
{
	return !port_empty[port];
}

void Buffer_out::getTagMatchIndex(vector<uint>& vec, uint port,uint tag)
{
	uint i = tag;
	if (entity[i][port].valid)
		vec.push_back(i);
	i++;
	if (i == depth)
		i = 0;
	while (i != tag) {
		if (entity[i][port].valid)
			vec.push_back(i);
		i++;
		if (i == depth)
			i = 0;
	}
	//for(uint i = 0; i < entity.size(); i++)
	//{
	//	if (entity[i][port].valid)
	//		vec.push_back(i);
	//}
}


Buffer_lc::Buffer_lc(const Simulator::Preprocess::ArrayPara para):Buffer_out(para)
{
	depth = system_parameter.lc_buffer_depth;
	databreadth = system_parameter.lc_buffer_data_breadth;
	boolbreadth = system_parameter.lc_buffer_bool_breadth;
	entity.resize(depth);

	for (auto it = entity.begin(); it < entity.end(); ++it)
		it->resize(databreadth + boolbreadth);

	tail_ptr.resize(databreadth + boolbreadth);
	head_ptr.resize(databreadth + boolbreadth);
	port_empty.resize(databreadth + boolbreadth);
	port_full.resize(databreadth + boolbreadth);
	ack.resize(databreadth + boolbreadth);

	for (auto& i : port_full)
		i = false;
	for (auto& i : port_empty)
		i = true;
	for (auto& i : tail_ptr)
		i = 0;
	for (auto& i : head_ptr)
		i = 0;
	ack = { false };
}

Bufferpe_out::Bufferpe_out(const Simulator::Preprocess::ArrayPara para):Buffer_out(para)
{
	depth = system_parameter.pe_out_buffer_depth;
	databreadth = system_parameter.data_outport_breadth;
	boolbreadth = system_parameter.bool_outport_breadth;
	entity.resize(depth);

	for (auto it = entity.begin(); it < entity.end(); ++it)
		it->resize(databreadth + boolbreadth);

	tail_ptr.resize(databreadth + boolbreadth);
	head_ptr.resize(databreadth + boolbreadth);
	port_empty.resize(databreadth + boolbreadth);
	port_full.resize(databreadth + boolbreadth);
	ack.resize(databreadth + boolbreadth);

	for (auto& i : port_full)
		i = false;
	for (auto& i : port_empty)
		i = true;
	for (auto& i : tail_ptr)
		i = 0;
	for (auto& i : head_ptr)
		i = 0;
	ack = { false };
}


Bufferlse_in::Bufferlse_in(const Simulator::Preprocess::ArrayPara para):Buffer_in(para)
{
	depth = system_parameter.lse_inbuffer_depth;
	databreadth = system_parameter.lse_datain_breadth;
	boolbreadth = system_parameter.lse_boolin_breadth;
	entity.resize(depth);

	for (auto it = entity.begin(); it < entity.end(); ++it)
		it->resize(databreadth + boolbreadth);

	tail_ptr.resize(databreadth + boolbreadth);
	head_ptr.resize(databreadth + boolbreadth);
	port_full.resize(databreadth + boolbreadth);
	port_empty.resize(databreadth + boolbreadth);

	for (auto& i : port_full)
		i = false;
	for (auto& i : port_empty)
		i = true;
	for (auto& i : head_ptr)
		i = 0;
	for (auto& i : tail_ptr)
		i = 0;

	ack = { false };
}
Bufferlse_out::Bufferlse_out(const Simulator::Preprocess::ArrayPara para, Simulator::BufferSize size,Simulator::LSMode lsmode) :Buffer_out(para)
{
	if (lsmode != Simulator::LSMode::dummy) {
		switch (size)
		{
		case BufferSize::small:
			depth = system_parameter.le_outbuffer_depth_small;
			break;
		case BufferSize::middle:
			depth = system_parameter.le_outbuffer_depth_middle;
			break;
		case BufferSize::large:
			depth = system_parameter.le_outbuffer_depth_large;
			break;
		default:
			DEBUG_ASSERT(false);
		}
	}
	else {
		switch (size)
		{
		case BufferSize::small:
			depth = system_parameter.le_outbuffer_depth_small + system_parameter.lse_inbuffer_depth;
			break;
		case BufferSize::middle:
			depth = system_parameter.le_outbuffer_depth_middle + system_parameter.lse_inbuffer_depth;
			break;
		case BufferSize::large:
			depth = system_parameter.le_outbuffer_depth_large + system_parameter.lse_inbuffer_depth;
			break;
		default:
			DEBUG_ASSERT(false);
		}
	}

//	depth = system_parameter.le_outbuffer_depth_large;
	databreadth = system_parameter.le_dataout_breadth;
	boolbreadth = system_parameter.le_boolout_breadth;
	entity.resize(depth);
	head_ptr.resize(databreadth + boolbreadth);

	for (auto it = entity.begin(); it < entity.end(); ++it)
		it->resize(databreadth + boolbreadth);

	tail_ptr.resize(databreadth + boolbreadth);
	head_ptr.resize(databreadth + boolbreadth);
	port_empty.resize(databreadth + boolbreadth);
	port_full.resize(databreadth + boolbreadth);

	for (auto& i : port_full)
		i = false;
	for (auto& i : port_empty)
		i = true;
	for (auto& i : tail_ptr)
		i = 0;
	for (auto& i : head_ptr)
		i = 0;

	ack = { false };
}
Bufferlse_out::Bufferlse_out(const Simulator::Preprocess::ArrayPara para):Buffer_out(para)
{   	
	depth = system_parameter.le_outbuffer_depth_middle;
	databreadth = system_parameter.le_dataout_breadth;
	boolbreadth = system_parameter.le_boolout_breadth;
	entity.resize(depth);
	head_ptr.resize(databreadth + boolbreadth);

	for (auto it = entity.begin(); it < entity.end(); ++it)
		it->resize(databreadth + boolbreadth);

	tail_ptr.resize(databreadth + boolbreadth);
	head_ptr.resize(databreadth + boolbreadth);
	port_empty.resize(databreadth + boolbreadth);
	port_full.resize(databreadth + boolbreadth);

	for (auto& i : port_full)
		i = false;
	for (auto& i : port_empty)
		i = true;
	for (auto& i : tail_ptr)
		i = 0;
	for (auto& i : head_ptr)
		i = 0;

	ack = { false };
}

bool Bufferlse_out::output_ack(Port_inout& output, uint tag)
{
//	DEBUG_ASSERT(entity[tag][0].valid);
	if (entity[tag][0].valid) {
		output.valid = entity[tag][0].valid;
		output.value_data = entity[tag][0].valued;
		output.value_bool = entity[tag][0].valueb;
		output.condition = entity[tag][0].condition;
		output.last = entity[tag][0].last;
		return true;
	}
	else {
//		std::cout << "clk is" << ClkDomain::getInstance()->getClk();
		return false;
	}
}

//void Bufferlse_out::output(Port_inout& output, uint tag)
//{
//	if (!entity[head_ptr[0]][tag].valid)
//	{
//		output.valid = entity[head_ptr[0]][tag].valid;
//		output.value_data = entity[head_ptr[0]][tag].valued;
//		output.value_bool = entity[head_ptr[0]][tag].valueb;
//		output.condition = entity[head_ptr[0]][tag].condition;
//		output.last = entity[head_ptr[0]][tag].last;
//	}
//	reset(tag);   //credit机制，在输出时直接清数///////////////////////////////////////////////////////有问题，valid有效才输出////////////////////////
//}

//void Bufferlse_out::output(Port_inout& output, uint port)
//{
//	if (entity[head_ptr[port]][port].valid)
//	{
//		output.valid = entity[head_ptr[port]][port].valid;
//		output.value_data = entity[head_ptr[port]][port].valued;
//		output.value_bool = entity[head_ptr[port]][port].valueb;
//		output.condition = entity[head_ptr[port]][port].condition;
//		output.last = entity[head_ptr[port]][port].last;
//	}
//	reset(tag);   //credit机制，在输出时直接清数///////////////////////////////////////////////////////有问题，valid有效才输出////////////////////////
//}


void Bufferlse_out::outputTag(Port_inout& output, uint tag)
{
	if (entity[tag][0].valid)
	{
		output.valid = entity[tag][0].valid;
		output.value_data = entity[tag][0].valued;
		output.value_bool = entity[tag][0].valueb;
		output.condition = entity[tag][0].condition;
		output.last = entity[tag][0].last;
	}
	resetTag(tag);   //credit机制，在输出时直接清数
}

void Bufferlse_out::resetTag(uint tag)
{
	//DEBUG_ASSERT(entity[0][tag].valid);
	entity[tag][0].valid = false;
	entity[tag][0].condition = true;
	entity[tag][0].last = false;
	entity[tag][0].valued = 0;
	entity[tag][0].valueb = false;
}


