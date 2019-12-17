#include "LSE.h"
#include <iomanip>

using namespace Simulator::Array;

Loadstore_element::Loadstore_element(const Simulator::Preprocess::ArrayPara para, uint index, Lsu* lsu):Node(para)
{
	this->lsu = lsu;
	inbuffer = Buffer_factory::createLseInBuffer(para);
	outbuffer = Buffer_factory::createLseBuffer(para);
	tag_counter = 0;
	nextpe_bp.resize(system_parameter.le_dataout_breadth + system_parameter.le_boolout_breadth);
	input_port_pe.resize(system_parameter.lse_boolin_breadth + system_parameter.lse_datain_breadth);
	input_port_lsu.resize(1);
	auto dict = Preprocess::DFG::getInstance()->getDictionary();
	auto ls_vec = dict.find(Simulator::NodeType::ls)->second;
	attribution = dynamic_cast<const Preprocess::DFGNode<Simulator::NodeType::ls>*>(ls_vec[index]);
	inbuffer_bp = Bp_factory::createLseBp(para, this);
	break_state.resize(system_parameter.lse_boolin_breadth + system_parameter.lse_datain_breadth);
}

void Loadstore_element::update()
{
	simStep2();
	simStep1();
	simBp();
	wireReset();
}

void Loadstore_element::wireReset()
{
	//wire signal每周期重新从寄存器读出
	for (auto& i : input_port_pe)
		i.reset();

	for (auto& i : input_port_lsu)
		i.reset();

	output_port_2lsu.reset();
	output_port_2array.reset();
	//output_port_2lc.reset();
	//vector<Bool> this_bp;         //bp线由于每周期更新且没有valid位，不需要reset
	//vector<Bool> nextpe_bp;
	//Bool nextlsu_bp;
	//uint sended_tag;
}

void Loadstore_element::leSimStep1()
{
	//get input from pe
	for (uint i = 0; i < input_port_pe.size(); ++i)
	{
		if (break_state[i])
		{
			if (input_port_pe[i].last && input_port_pe[i].valid)
			{
				break_state[i] = false;       //当输入last信号时该端口的break状态结束
											  //last不携带其他数据，无需进入buffer       //INSPECTION
			}
			else if (!input_port_pe[i].last && input_port_pe[i].valid)
			{
				//input不进入inbuffer
				//bp仍返回   ack-入数成功/credit-可以入数
			}
		}
		else
		{
			if (input_port_pe[i].valid)
				inbuffer->input(input_port_pe[i], i);
		}
	}

	//get input from lsu，不能被break影响，因为之前发出的都是有效的访问请求
	for (uint i = 0; i < input_port_lsu.size(); ++i)
	{
		if (input_port_lsu[i].valid)
		{
			if (outbuffer->input_tag_lsu(input_port_lsu[i], i, input_port_lsu[i].tag))
			{
				if (system_parameter.attach_memory)
					this_bp[1] = true;
				else if (!system_parameter.attach_memory)
					fake_lsu.erase(fake_lsu.end());
			}
		}
	}	
}

void Loadstore_element::seSimStep1()
{
	//get input from pe
	for (uint i = 0; i < input_port_pe.size(); ++i)
	{
		if (break_state[i])
		{
			if (input_port_pe[i].last && input_port_pe[i].valid)
			{
				break_state[i] = false;       //当输入last信号时该端口的break状态结束
											  //last不携带其他数据，无需进入buffer       //INSPECTION
			}
			else if (!input_port_pe[i].last && input_port_pe[i].valid)
			{
				//input不进入inbuffer
				//bp仍返回   ack-入数成功/credit-可以入数
			}
		}
		else
		{
			if (input_port_pe[i].valid)
			{
				if (attribution->match)                                         //match时需要参与matchset进行tag操作
				{
					if (outbuffer->input_tag(input_port_pe[i], i, tag_counter))
						tag_counter_update();
				}
				else                                                             //！match时不需要tag
					outbuffer->input(input_port_pe[i], i);
			}
		}
	}
}


//getinput to buffer
void Loadstore_element::simStep1()
{
	//le
	if (attribution->ls_mode == LSMode::load)
	{ 
		leSimStep1();
	}
	//se
	else if (attribution->ls_mode == LSMode::store_addr || attribution->ls_mode == LSMode::store_data)
	{
		seSimStep1();
	}
}

//inbuffer向LSU或PE出数
void Loadstore_element::simStep2()
{
	if (system_parameter.attach_memory)
	{
		if (attribution->ls_mode == LSMode::load)
			leSimStep2();
		else if (attribution->ls_mode == LSMode::store_data)                       //SEA和SED的协同行为由SED完成
			sedSimStep2();
	}
	else if (!system_parameter.attach_memory)
	{
		if (attribution->ls_mode == LSMode::load)
			leSimStep2NoMem();
		else if (attribution->ls_mode == LSMode::store_data)                       //SEA和SED的协同行为由SED完成
			sedSimStep2NoMem();
	}
}

void Loadstore_element::tag_counter_update()
{
	tag_counter++;

	switch (attribution->size)
	{
	case BufferSize::small:
		if (tag_counter == 32)
			tag_counter = 0;
		break;

	case BufferSize::middle:
		if (tag_counter == 64)
			tag_counter = 0;
		break;

	case BufferSize::large:
		if (tag_counter == 128)
			tag_counter = 0;
		break;
	}
}

void Loadstore_element::leSimStep2()
{
	//输出到lsu
	if (nextlsu_bp)
	{
		inbuffer->reset_head();
		if (attribution->match)
			tag_counter_update();
	}

	Port_inout inbuffer_out;
	inbuffer->output(inbuffer_out, 0);

	if (!inbuffer_out.last && inbuffer_out.condition)
	{
		output_port_2lsu.valid = inbuffer_out.valid;
		output_port_2lsu.value_addr = inbuffer_out.value_data;
		output_port_2lsu.rdwr = false;
		output_port_2lsu.dae = attribution->dae;
		output_port_2lsu.tag = tag_counter;
		lsu->Addtranscation(output_port_2lsu);
	}
	else
	{
		if (outbuffer->input_tag(inbuffer_out, 0, tag_counter))
		{
			inbuffer->reset_head();
			if (attribution->match)
				tag_counter_update();//////////////////////////////////////////////////////在所有情况都需要tag_counter_update()///////////////
		}
	}
	

	//输出到pe
	if (nextpe_bp[0])
	{
		if (!attribution->match)
			outbuffer->output(output_port_2array, 0);
        //tag match, matchset的仿真是多LE级的，适合放在更高层次执行
		//不放在le函数内执行
	}
		
}

void Loadstore_element::leSimStep2NoMem()
{
	Port_inout inbuffer_out;
	inbuffer->output(inbuffer_out, 0);

	if (!inbuffer_out.last && inbuffer_out.condition)
	{
		output_port_2lsu.valid = inbuffer_out.valid;
		output_port_2lsu.value_addr = inbuffer_out.value_data;
		output_port_2lsu.rdwr = false;
		output_port_2lsu.dae = attribution->dae;
		output_port_2lsu.tag = tag_counter;

		//lsu->Addtranscation(output_port_2lsu);
		readNoMemory();
		inbuffer->reset_head();////输出时好像会reset
		if (attribution->match)
			tag_counter_update();///////////////没有用outbuffer输出
	}
	else
	{
		if (outbuffer->input_tag(inbuffer_out, 0, tag_counter))
		{
			inbuffer->reset_head();
			if (attribution->match)
				tag_counter_update();///////////////////////////////////这个情况需要update吗(不该出数的情况)
		}
	}


	//输出到pe
	if (nextpe_bp[0])
	{
		if (!attribution->match)
			outbuffer->output(output_port_2array, 0);//这里输出了
		//tag match, matchset的仿真是多LE级的，适合放在更高层次执行
		//不放在le函数内执行
	}
}

void Loadstore_element::sedSimStep2NoMem()
{
	//对于写请求来说，需要实现一个匹配的过程
	vector<uint> validDataIndex;
	vector<uint> validAddrIndex;
	vector<vector<uint>> vecs;
	outbuffer->getTagMatchIndex(validDataIndex, 0);
	couple_se->outbuffer->getTagMatchIndex(validAddrIndex, 0);
	vecs.push_back(std::move(validDataIndex));
	vecs.push_back(std::move(validAddrIndex));

	const auto match_tag = Util::findSameValueInVectors<uint>(vecs);

	if (match_tag != UINT_MAX)
	{
		Port_inout temp;
		Port_inout temp2;
		outbuffer->output_ack(temp, match_tag);
		CoupleSeOutput(temp2, match_tag);

		if (!temp.last && temp.condition)
		{
			//output_port_2lsu赋值
			//lsu->Addtranscation(output_port_2lsu);
			MemoryData::getInstance()->write(temp.value_data, temp2.value_data);
			sended_tag = match_tag;
			outbuffer->reset(sended_tag);
			coupleSeReset(sended_tag);
		}
		else if (!temp.condition)                  //false分支处理
		{
			outbuffer->reset(match_tag);
			coupleSeReset(match_tag);
		}
		else if (temp.last)                     //last处理
		{
			output_port_2array.valid = true;
			output_port_2array.value_bool = true;
			output_port_2array.last = true;
			outbuffer->reset(match_tag);
			coupleSeReset(match_tag);
		}
	}
}

void Loadstore_element::readNoMemory()
{
	if (output_port_2lsu.valid)
	{
		fake_lsu.push_back(output_port_2lsu);

		input_port_lsu[0] = fake_lsu.front();
		input_port_lsu[0].value_data = MemoryData::getInstance()->read(fake_lsu.front().value_addr);
	}
}

void Loadstore_element::sedSimStep2()
{
	if (nextlsu_bp)
	{
		outbuffer->reset(sended_tag);
		coupleSeReset(sended_tag);
	}
	//对于写请求来说，需要实现一个匹配的过程
	vector<uint> validDataIndex;
	vector<uint> validAddrIndex;
	vector<vector<uint>> vecs;
	outbuffer->getTagMatchIndex(validDataIndex,0);
	couple_se->outbuffer->getTagMatchIndex(validAddrIndex, 0);
	vecs.push_back(std::move(validDataIndex));
	vecs.push_back(std::move(validAddrIndex));

	const auto match_tag = Util::findSameValueInVectors<uint>(vecs);

	if (match_tag != UINT_MAX)
	{
		Port_inout temp;
		Port_inout temp2;
		outbuffer->output_ack(temp, match_tag);
		CoupleSeOutput(temp2, match_tag);

		if (!temp.last && temp.condition)
		{
			//output_port_2lsu赋值
			lsu->Addtranscation(output_port_2lsu);
			sended_tag = match_tag;
		}
		else if (!temp.condition)                  //false分支处理
		{
			outbuffer->reset(match_tag);
			coupleSeReset(match_tag);
		}
		else if (temp.last)                     //last处理
		{
			output_port_2array.valid = true;
			output_port_2array.value_bool = true;
			output_port_2array.last = true;
			outbuffer->reset(match_tag);
			coupleSeReset(match_tag);
		}
	}
}

void Loadstore_element::simBp()
{
	if (attribution->ls_mode == LSMode::load)
	{
		for (uint i = 0; i < this_bp.size(); ++i)
			this_bp[i] = inbuffer_bp->getBp(i);
	}
	else
	{
		for (uint i = 0; i < this_bp.size(); ++i)
			this_bp[i] = outbuffer_bp->getBp(i);
	}
}

void Loadstore_element::coupleSeReset(uint tag)
{
	couple_se->resetByCouple(tag);
}

bool Loadstore_element::isCoupleSeNotEmpty()
{
	return couple_se->isOutBufferNotEmpty();
}

void Loadstore_element::CoupleSeOutput(Port_inout& out, uint port)
{
	couple_se->CoupledSeOutput(out, port);
}

bool Loadstore_element::isOutBufferNotEmpty()
{
	return outbuffer->isBufferNotEmpty(0);
}

void Loadstore_element::CoupledSeOutput(Port_inout& out, uint port)
{
	outbuffer->output_ack(out, 0);
}

void Loadstore_element::resetByCouple(uint tag)
{
	outbuffer->reset(tag);
}

void Loadstore_element::getInput(uint port, Port_inout input)  //port之间来源不同，解耦合；但inbuffer的写入仍保持耦合
{
	input_port_pe[port] = input;
}

void Loadstore_element::getBp(uint port, bool input)
{
	nextpe_bp[port] = input;
}

void Loadstore_element::seAttach(Loadstore_element* couple)
{
	couple_se = couple;
}


Match_set::Match_set(const Simulator::Preprocess::ArrayPara para, map<uint, Loadstore_element*> ls_map)
{
	system_parameter = para;

	for (auto iter = ls_map.begin(); iter != ls_map.end(); ++iter)
	{
		if (iter->second->attribution->match && iter->second->attribution->ls_mode == LSMode::load)
			lse_set.push_back(iter->second);
	}
}

void Match_set::update()
{
	vector<vector<uint>> valid_tag_set;
	for (uint i = 0; i < lse_set.size(); i++)
	{
		vector<uint> temp;
		lse_set[i]->outbuffer->getTagMatchIndex(temp, 0);
		valid_tag_set[i] = std::move(temp);
	}
	const uint match_tag = Util::findSameValueInVectors<uint>(valid_tag_set);

	bool joined_bp = true;
	for (auto& i : lse_set)
		joined_bp &= i->nextpe_bp[0];

	if (match_tag != UINT_MAX && joined_bp)
	{
		for (auto& i : lse_set)
		{
			if (i->attribution->ls_mode == LSMode::load)
				i->outbuffer->output(i->output_port_2array, 0);
		}
	}
}

