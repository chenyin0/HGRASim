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
	//wire signalÿ�������´ӼĴ�������
	for (auto& i : input_port_pe)
		i.reset();

	for (auto& i : input_port_lsu)
		i.reset();

	output_port_2lsu.reset();
	output_port_2array.reset();
	//output_port_2lc.reset();
	//vector<Bool> this_bp;         //bp������ÿ���ڸ�����û��validλ������Ҫreset
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
				break_state[i] = false;       //������last�ź�ʱ�ö˿ڵ�break״̬����
											  //last��Я���������ݣ��������buffer       //INSPECTION
			}
			else if (!input_port_pe[i].last && input_port_pe[i].valid)
			{
				//input������inbuffer
				//bp�Է���   ack-�����ɹ�/credit-��������
			}
		}
		else
		{
			if (input_port_pe[i].valid)
				inbuffer->input(input_port_pe[i], i);
		}
	}

	//get input from lsu�����ܱ�breakӰ�죬��Ϊ֮ǰ�����Ķ�����Ч�ķ�������
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
				break_state[i] = false;       //������last�ź�ʱ�ö˿ڵ�break״̬����
											  //last��Я���������ݣ��������buffer       //INSPECTION
			}
			else if (!input_port_pe[i].last && input_port_pe[i].valid)
			{
				//input������inbuffer
				//bp�Է���   ack-�����ɹ�/credit-��������
			}
		}
		else
		{
			if (input_port_pe[i].valid)
			{
				if (attribution->match)                                         //matchʱ��Ҫ����matchset����tag����
				{
					if (outbuffer->input_tag(input_port_pe[i], i, tag_counter))
						tag_counter_update();
				}
				else                                                             //��matchʱ����Ҫtag
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

//inbuffer��LSU��PE����
void Loadstore_element::simStep2()
{
	if (system_parameter.attach_memory)
	{
		if (attribution->ls_mode == LSMode::load)
			leSimStep2();
		else if (attribution->ls_mode == LSMode::store_data)                       //SEA��SED��Эͬ��Ϊ��SED���
			sedSimStep2();
	}
	else if (!system_parameter.attach_memory)
	{
		if (attribution->ls_mode == LSMode::load)
			leSimStep2NoMem();
		else if (attribution->ls_mode == LSMode::store_data)                       //SEA��SED��Эͬ��Ϊ��SED���
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
	//�����lsu
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
				tag_counter_update();//////////////////////////////////////////////////////�������������Ҫtag_counter_update()///////////////
		}
	}
	

	//�����pe
	if (nextpe_bp[0])
	{
		if (!attribution->match)
			outbuffer->output(output_port_2array, 0);
        //tag match, matchset�ķ����Ƕ�LE���ģ��ʺϷ��ڸ��߲��ִ��
		//������le������ִ��
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
		inbuffer->reset_head();////���ʱ�����reset
		if (attribution->match)
			tag_counter_update();///////////////û����outbuffer���
	}
	else
	{
		if (outbuffer->input_tag(inbuffer_out, 0, tag_counter))
		{
			inbuffer->reset_head();
			if (attribution->match)
				tag_counter_update();///////////////////////////////////��������Ҫupdate��(���ó��������)
		}
	}


	//�����pe
	if (nextpe_bp[0])
	{
		if (!attribution->match)
			outbuffer->output(output_port_2array, 0);//���������
		//tag match, matchset�ķ����Ƕ�LE���ģ��ʺϷ��ڸ��߲��ִ��
		//������le������ִ��
	}
}

void Loadstore_element::sedSimStep2NoMem()
{
	//����д������˵����Ҫʵ��һ��ƥ��Ĺ���
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
			//output_port_2lsu��ֵ
			//lsu->Addtranscation(output_port_2lsu);
			MemoryData::getInstance()->write(temp.value_data, temp2.value_data);
			sended_tag = match_tag;
			outbuffer->reset(sended_tag);
			coupleSeReset(sended_tag);
		}
		else if (!temp.condition)                  //false��֧����
		{
			outbuffer->reset(match_tag);
			coupleSeReset(match_tag);
		}
		else if (temp.last)                     //last����
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
	//����д������˵����Ҫʵ��һ��ƥ��Ĺ���
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
			//output_port_2lsu��ֵ
			lsu->Addtranscation(output_port_2lsu);
			sended_tag = match_tag;
		}
		else if (!temp.condition)                  //false��֧����
		{
			outbuffer->reset(match_tag);
			coupleSeReset(match_tag);
		}
		else if (temp.last)                     //last����
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

void Loadstore_element::getInput(uint port, Port_inout input)  //port֮����Դ��ͬ������ϣ���inbuffer��д���Ա������
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

