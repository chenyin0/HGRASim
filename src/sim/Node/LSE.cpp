#include "LSE.h"
#include <iomanip>

using namespace Simulator::Array;

Loadstore_element::Loadstore_element(const Simulator::Preprocess::ArrayPara para, uint index, DRAMSim::Lsu* lsu):Node(para)
{
	this->lsu = lsu;
	this->index = index;
	inbuffer = Buffer_factory::createLseInBuffer(para);
	//outbuffer = Buffer_factory::createLseBuffer(para,attribution);
	tag_counter = 0;
	nextpe_bp.resize(system_parameter.le_dataout_breadth + system_parameter.le_boolout_breadth);
	input_port_pe.resize(system_parameter.lse_boolin_breadth + system_parameter.lse_datain_breadth);
	input_port_lsu.resize(1);
	this_bp.resize(system_parameter.lse_boolin_breadth + system_parameter.lse_datain_breadth);
	auto dict = Preprocess::DFG::getInstance()->getDictionary();
	auto ls_vec = dict.find(Simulator::NodeType::ls)->second;
	attribution = dynamic_cast<const Preprocess::DFGNode<Simulator::NodeType::ls>*>(ls_vec[index]);
	outbuffer = Buffer_factory::createLseBuffer(para, attribution->size,attribution->ls_mode);
	inbuffer_bp = Bp_factory::createLseBp(para, this);
	lastout = Port_inout();
	outbuffer_bp = Bp_factory::createLseBp(para, this);
#define order_force
	break_state.resize(system_parameter.lse_boolin_breadth + system_parameter.lse_datain_breadth);
	if (attribution->size == BufferSize::small)
		maintain_order.resize(system_parameter.le_outbuffer_depth_small);
	else if (attribution->size == BufferSize::middle)
		maintain_order.resize(system_parameter.le_outbuffer_depth_middle);
	else if (attribution->size == BufferSize::large)
		maintain_order.resize(system_parameter.le_outbuffer_depth_large);
	else
		;
	for (auto& i : maintain_order)
		i = true;

	for (auto& i : nextpe_bp)
		i = true;
}

Loadstore_element::~Loadstore_element()
{
	delete inbuffer;
	delete outbuffer;
	delete inbuffer_bp;
	delete outbuffer_bp;
}
void Loadstore_element::update()
{
	simStep2();
	simStep1();
	simBp();
//	print();
//	wireReset();
}

void Loadstore_element::attachLsu(Lsu* lsu_)
{
	this->lsu = lsu_;
}
void Loadstore_element::wireReset()
{
	//wire signal每周期重新从寄存器读出
	for (auto& i : input_port_pe)
		i.reset();
	inbuffer_out.reset();

//	for (auto& i : input_port_lsu)//////////这里需要上一周期的lsu的输入信号，因此在step1用完了这个信号之后再清空，这是其自己的内部信号
//		i.reset();
	for (auto& i : input_port_lsu)//////////这里需要上一周期的lsu的输入信号，因此在step1用完了这个信号之后再清空，这是其自己的内部信号
		i.reset();
	output_port_2lsu.reset();//////////////////////////////////////////////这个地方需要reset吗////////////
	output_port_2array.reset();
	//output_port_2array.reset();////////////////////不能把out reset，不然收不到数/////////////////////////////////////////////
	//output_port_2lc.reset();
	//vector<Bool> this_bp;         //bp线由于每周期更新且没有valid位，不需要reset
	//vector<Bool> nextpe_bp;
	//Bool nextlsu_bp;
	//uint sended_tag;
}
//void Loadstore_element::outReset() {
//	output_port_2array.reset();
//}

void Loadstore_element::lseReset() {
	for (auto& i : input_port_lsu)//////////这里需要上一周期的lsu的输入信号，因此在step1用完了这个信号之后再清空，这是其自己的内部信号
	i.reset();
}
void Loadstore_element::leSimStep1(uint i)
{
	//get input from pe
//	for (uint i = 0; i < input_port_pe.size(); ++i)
//	{
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
//	}

	//get input from lsu，不能被break影响，因为之前发出的都是有效的访问请求
	//for (uint i = 0; i < input_port_lsu.size(); ++i)
	//{
	//	if (input_port_lsu[i].valid && !((input_port_pe[0].valid) && (!input_port_pe[0].condition)))///////////需要考虑直连情况////////////////////////
	//	{
	//		if (outbuffer->input_tag_lsu(input_port_lsu[i], i, input_port_lsu[i].tag))
	//		{
	//			if (system_parameter.attach_memory)
	//				this_bp[0] = true;//index修改成了0
	//			else if (!system_parameter.attach_memory)
	//				fake_lsu.erase(fake_lsu.begin());///////////应该是front
	//		}
	//	}
	//}
}
void Loadstore_element::leSimStep1()
{
	for (uint i = 0; i < input_port_pe.size(); ++i)
		leSimStep1(i);
}
void Loadstore_element::seSimStep1(uint i)
{
	//get input from pe
//	for (uint i = 0; i < input_port_pe.size(); ++i)
//	{
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
				//if (attribution->match)                                         //match时需要参与matchset进行tag操作
				//{
				//	if (outbuffer->input_tag(input_port_pe[i], i, tag_counter))
				//		tag_counter_update();
				//}
				//else                                                             //！match时不需要tag
				//	outbuffer->input(input_port_pe[i], i);
				if (outbuffer->input_tag(input_port_pe[i], i, tag_counter))
					tag_counter_update();//都需要按tag进数和出数
			}
		}
//	}
}

void Loadstore_element::seSimStep1()
{
	for (uint i = 0; i < input_port_pe.size(); ++i)
		seSimStep1(i);
}
//getinput to buffer
void Loadstore_element::simStep1(uint i)
{
	if (attribution->ls_mode != LSMode::dummy) {
		if (attribution->ls_mode == LSMode::load)
		{
			leSimStep1(i);
		}
		//se
		else if (attribution->ls_mode == LSMode::store_addr || attribution->ls_mode == LSMode::store_data)
		{
			seSimStep1(i);
		}
	}
	else 
	{ 
		DEBUG_ASSERT(i == 0);
		outbuffer->input(input_port_pe[i], i);
	}

}

void Loadstore_element::simStep1()
{
	if (attribution->ls_mode != LSMode::dummy) {
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
	else
	{
	//	DEBUG_ASSERT(i == 0);
		for (uint i = 0; i < input_port_pe.size(); ++i)
			outbuffer->input(input_port_pe[i], i);
	}

}

//inbuffer向LSU或PE出数
void Loadstore_element::simStep2()
{
	if (attribution->ls_mode != LSMode::dummy) {
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
	else {
		if (nextpe_bp[0]) 
		{
			outbuffer->output(output_port_2array, 0);
		}
	}
}

void Loadstore_element::tag_counter_update()
{
	uint clk = ClkDomain::getInstance()->getClk();
	tag_counter++;

	switch (attribution->size)
	{
	case BufferSize::small:
		if (tag_counter == system_parameter.le_outbuffer_depth_small)
			tag_counter = 0;
		break;

	case BufferSize::middle:
		if (tag_counter == system_parameter.le_outbuffer_depth_middle)
			tag_counter = 0;
		break;

	case BufferSize::large:
		if (tag_counter == system_parameter.le_outbuffer_depth_large)
			tag_counter = 0;
		break;
	}
}
void Loadstore_element::LSEcallback(uint addr, uint64_t cycle, short tag)
{
	if (attribution->ls_mode == LSMode::load) {
		for (uint i = 0; i < input_port_lsu.size(); ++i) {
			input_port_lsu[i].value_addr = addr;
			input_port_lsu[i].valid = true;
			input_port_lsu[i].tag = tag;
			input_port_lsu[i].value_data = addr;
//			input_port_lsu[i].value_data = Simulator::Array::MemoryData::getInstance()->read(addr);//////////////////////////////////////////////此处可以更加完善///////////////////////////////
			input_port_lsu[i].rdwr = false;
		}
		for (uint i = 0; i < input_port_lsu.size(); ++i)
			if (input_port_lsu[i].valid) {
				if (outbuffer->input_tag_lsu(input_port_lsu[i], i, input_port_lsu[i].tag)) { ; }////////////////////这个地方是需要bp判断的吧
				else {
					uint clk = ClkDomain::getInstance()->getClk();
					Debug::getInstance()->getPortFile() << "outbuffer can no in kls" <<attribution->index<<" " <<ClkDomain::getInstance()->getClk()<< std::endl;
//					DEBUG_ASSERT(false); 
				}
//#ifdef order_force
//				maintain_order[input_port_lsu[i].tag] = true;
//#endif
			}
	}
	else
		DEBUG_ASSERT(false);

}
void Loadstore_element::leSimStep2()
{

	//for (uint i = 0; i < input_port_lsu.size(); ++i)
	//{
	//	if (input_port_lsu[i].valid)///////////需要考虑直连情况////////////////////////
	//	{
	//		if (outbuffer->input_tag_lsu(input_port_lsu[i], i, input_port_lsu[i].tag))
	//		{
	//			if (!system_parameter.attach_memory)
	//				fake_lsu.erase(fake_lsu.begin());///////////应该是front
	//		}
	//	}
	//}//这实际上是上周期就做了的事情了，不过就是这一周期先做好，就相当于上一周期发过来，就可以用了
	//输出到lsu
		//输出到pe
	if (nextpe_bp[0])
	{
		if (!attribution->match) {
#ifdef order_force
			if (outbuffer->entity[outbuffer->head_ptr[0]][0].valid)
			{
				maintain_order[outbuffer->head_ptr[0]] = true;
			}
#endif
			outbuffer->output(output_port_2array, 0);//这里输出了

		}
		//tag match, matchset的仿真是多LE级的，适合放在更高层次执行
		//不放在le函数内执行
		else {
#ifdef order_force
			if (outbuffer->entity[tag_counter][0].valid)
			{
				maintain_order[tag_counter] = true;
			}
#endif
			outbuffer->outputTag(output_port_2array, tag_counter);/////////这里是有问题的
		}
		//tag match, matchset的仿真是多LE级的，适合放在更高层次执行
		//不放在le函数内执行
	}
	//if (lastout.valid&& output_port_2array.valid&&lastout.last != true && output_port_2array.last != true)
	//	DEBUG_ASSERT(output_port_2array.value_data - lastout.value_data == 1);
	//if(!output_port_2array.last)
	//	lastout = output_port_2array;


	if (nextlsu_bp)
	{
//		inbuffer->reset_head();
		if (!wait_for_data)
			inbuffer->reset(0);
		if (inbuffer->output_ack(inbuffer_out, 0)) {
			wait_for_data = false;
		}
		else {
			wait_for_data = true;
		}
		if(inbuffer_out.valid){
			if (firstlsu) {
				firstlsu = false;
			}
			else {
				tag_counter_update();//第一次更新tag为0
//				firstlsu = false;
			}
		}
	//	addr_v = true;
//			tag_counter_update();//只有后一级返回ack时出新数才能更新tag_counter，否则不更新
		if(!inbuffer_out.last && inbuffer_out.condition&& inbuffer_out.valid)///////对应了非直通的情况
			nextlsu_bp = false;
	}
	else
	{
		inbuffer->output_ack(inbuffer_out, 0);
	}
//	inbuffer->output(inbuffer_out, 0);

	if (!inbuffer_out.last && inbuffer_out.condition && inbuffer_out.valid)//在last的时候就不要过数据流
	{
		output_port_2lsu.valid = inbuffer_out.valid;
		output_port_2lsu.value_addr = inbuffer_out.value_data;
		output_port_2lsu.rdwr = false;
		output_port_2lsu.dae = attribution->dae;
		output_port_2lsu.tag = tag_counter;
	//	tag_counter_update();
	//	if (addr_v) {

#ifdef order_force
		if (maintain_order[output_port_2lsu.tag]) {
#endif
			lsu->AddTrans(output_port_2lsu, index);
#ifdef order_force
			maintain_order[output_port_2lsu.tag] = false;

		}
#endif
	//		addr_v = false;
	//	}
	}
	else if(inbuffer_out.valid)
	{
//		if (inbuffer_out.valid) {
//			tag_counter_update();
#ifdef order_force
		if (maintain_order[tag_counter]) {
#endif
			if (outbuffer->input_tag(inbuffer_out, 0, tag_counter))////////////////暗含了inbuffer_out.valid信息/////////////
			{
				maintain_order[tag_counter] = false;
				//		tag_counter_update();
				nextlsu_bp = true;
				//			inbuffer->reset(0);
				//			if (attribution->match)
				//////////////////////////////////////////////////////在所有情况都需要tag_counter_update()///////////////
			}
			else { 
				//nextlsu_bp = false;
				Debug::getInstance()->getPortFile() << "outbuffer can no in ls" <<attribution->index<<" "<< ClkDomain::getInstance()->getClk() << std::endl; 
			//	DEBUG_ASSERT(false); 
			}
#ifdef order_force
		}
		else{ nextlsu_bp = false; }
#endif
//		}
	}
	//通过outbuffer的输出决定bp
	

		
}

void Loadstore_element::leSimStep2NoMem()
{
	//Port_inout inbuffer_out;
	if (nextpe_bp[0])
	{
		//DEBUG_ASSERT(false);
		if (!attribution->match)
			outbuffer->output(output_port_2array, 0);//这里输出了
		//tag match, matchset的仿真是多LE级的，适合放在更高层次执行
		//不放在le函数内执行
		else
			outbuffer->outputTag(output_port_2array, tag_counter);
	}

	inbuffer->output(inbuffer_out, 0);

	if (!inbuffer_out.last&&inbuffer_out.condition&& inbuffer_out.valid)//不应该是last来确定,last不要让这个数过数据流////
	{
		output_port_2lsu.valid = inbuffer_out.valid;
		output_port_2lsu.value_addr = inbuffer_out.value_data;
		output_port_2lsu.rdwr = false;
		output_port_2lsu.dae = attribution->dae;
		output_port_2lsu.tag = tag_counter;//用过了这个tag再累加

		//lsu->Addtranscation(output_port_2lsu);
		readNoMemory();
//		inbuffer->reset_head();////输出时好像会reset
		tag_counter_update();
//		if (attribution->match)
//			tag_counter_update();///////////////没有用outbuffer输出
		for (uint i = 0; i < input_port_lsu.size(); ++i)
		{
			if (input_port_lsu[i].valid )///////////需要考虑直连情况////////////////////////
			{
				if (outbuffer->input_tag_lsu(input_port_lsu[i], i, input_port_lsu[i].tag))
				{
					if (!system_parameter.attach_memory)
						fake_lsu.erase(fake_lsu.begin());///////////应该是front
				}
			}
		}
	}
	else
	{
	//	tag_counter_update();
		lsu_noin = true;//
		//用过了这个tag再累加
		if (outbuffer->input_tag(inbuffer_out, 0, tag_counter))//////////////bufferlse reset会使第一个输出变成invalid////////////////////////////////////////////
		{
//			inbuffer->reset_head();
			tag_counter_update();
//			if (attribution->match)
//				tag_counter_update();///////////////////////////////////这个情况需要update吗(不该出数的情况)
		}
	}



}

void Loadstore_element::sedSimStep2NoMem()
{
	//对于写请求来说，需要实现一个匹配的过程
	outbuffer->getTagMatchIndex(validDataIndex, 0,0);
	couple_se->outbuffer->getTagMatchIndex(validAddrIndex, 0,0);
	vecs.push_back(std::move(validDataIndex));
	vecs.push_back(std::move(validAddrIndex));
	uint clk = ClkDomain::getInstance()->getClk();
	match_tag = Util::findSameValueInVectors<uint>(vecs);
	vecs.clear();
	if (match_tag != UINT_MAX)
	{
		outbuffer->output_ack(output_port_2array, match_tag);
		CoupleSeOutput(couple_se->output_port_2array, match_tag);

		if (!couple_se->output_port_2array.last && couple_se->output_port_2array.condition)
		{
			//output_port_2lsu赋值
			//lsu->Addtranscation(output_port_2lsu);

			if (!couple_se->output_port_2array.last && couple_se->output_port_2array.condition&& !output_port_2array.last && output_port_2array.condition) {
				MemoryData::getInstance()->write(static_cast<uint>(couple_se->output_port_2array.value_data), output_port_2array.value_data);
			}
			sended_tag = match_tag;
			outbuffer->resetTag(sended_tag);
			coupleSeReset(sended_tag);
		}
		else if (!couple_se->output_port_2array.condition)                  //false分支处理
		{
	//		outbuffer->outputTag(output_port_2array, tag_counter);
			outbuffer->resetTag(match_tag);
			coupleSeReset(match_tag);
		}
		else if (couple_se->output_port_2array.last)                     //last处理
		{
			output_port_2array.valid = true;
			output_port_2array.value_bool = true;
			output_port_2array.last = true;
			outbuffer->resetTag(match_tag);
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
//		input_port_lsu[0].value_data = MemoryData::getInstance()->read(fake_lsu.front().value_addr);///////////////////这个里面memory还没有赋值///////////
		input_port_lsu[0].value_data = fake_lsu.front().value_addr;
		input_port_lsu[0].tag=tag_counter;
	}
}

void Loadstore_element::callbackACK()
{
	nextlsu_bp = true;
}

void Loadstore_element::printBuffer()
{
	const auto& debugPrint = Debug::getInstance();
	if (Debug::getInstance()->debug_level >= 1) {
		if (ClkDomain::getInstance()->getClk() >= Debug::getInstance()->print_file_begin && ClkDomain::getInstance()->getClk() < Debug::getInstance()->print_file_end) {
			if (attribution->ls_mode == LSMode::load) {
				Debug::getInstance()->getLseFile() << "--------LE " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
				inbuffer->print(Debug::getInstance()->getLseFile());
				outbuffer->print_valid(Debug::getInstance()->getLseFile());
				//			Debug::getInstance()->getPortFile() << "--------LE " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
				//			wirePrint();
	//			Debug::getInstance()->getPortFile() << "--------LE " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
	//			debugPrint->onePrint<Bool>(nextlsu_bp, "nextlsu_bp");
			}
			else {
				Debug::getInstance()->getLseFile() << "--------SE " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
				inbuffer->print(Debug::getInstance()->getLseFile());
				outbuffer->print_valid(Debug::getInstance()->getLseFile());
				//			Debug::getInstance()->getPortFile() << "--------SE " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
				//			wirePrint();
			}
		}
	}
}

void Loadstore_element::print()
{
	if (ClkDomain::getInstance()->getClk() >= Debug::getInstance()->print_file_begin && ClkDomain::getInstance()->getClk() < Debug::getInstance()->print_file_end) {
		if (attribution->ls_mode == LSMode::load) {
//			Debug::getInstance()->getLseFile() << "--------LE " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
//			inbuffer->print(Debug::getInstance()->getLseFile());
//			outbuffer->print(Debug::getInstance()->getLseFile());
			Debug::getInstance()->getPortFile() << "--------LE " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
			wirePrint();
		}
		else {
//			Debug::getInstance()->getLseFile() << "--------SE " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
//			inbuffer->print(Debug::getInstance()->getLseFile());
//			outbuffer->print(Debug::getInstance()->getLseFile());
			Debug::getInstance()->getPortFile() << "--------SE " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
			wirePrint();
		}
	}
}
void Loadstore_element::wirePrint()
{
	const auto& debugPrint = Debug::getInstance();
	if (debugPrint->debug_level == 2) {
		if (attribution->ls_mode == LSMode::load)
		{
			debugPrint->linePrint("   this is step2");
			debugPrint->onePrint<Bool>(nextlsu_bp, "nextlsu_bp");
			debugPrint->onePrint<Port_inout>(inbuffer_out, "inbuffer_out");
			Debug::getInstance()->getPortFile() << "lse" << index<<" ";
			debugPrint->onePrint<Port_inout_lsu>(output_port_2lsu, "output_port_2lsu");
		//	debugPrint->onePrint<Bool>(maintain_order[output_port_2lsu.tag], "output_port_2lsu.order");
			Debug::getInstance()->getPortFile() << "lse" << index<< "output_port_2lsu.tag" << output_port_2lsu.tag << " "<<std::endl;
			Debug::getInstance()->getPortFile() << "lse" << index<<" ";
			debugPrint->onePrint<Port_inout_lsu>(input_port_lsu[0], "input_port_lsu");
			Debug::getInstance()->getPortFile() << "lse" << index << " ";
			debugPrint->onePrint<Port_inout>(output_port_2array, "output_pe");
			debugPrint->linePrint("   this is step1");
			debugPrint->vecPrint<Port_inout>(input_port_pe, "input_pe");
			debugPrint->vecPrint<Bool>(maintain_order, "maintain_order");
			debugPrint->vecPrint<Bool>(nextpe_bp, "nextpe_bp");
			debugPrint->vecPrint<Bool>(this_bp, "this_bp");
		}
		else {
			debugPrint->linePrint("   this is step2");
			debugPrint->onePrint<Bool>(nextlsu_bp, "nextlsu_bp");
			debugPrint->vecPrint<uint>(validAddrIndex, "validAddrIndex");
			debugPrint->vecPrint<uint>(validDataIndex, "validDataIndex");
			Debug::getInstance()->getPortFile() << "     match_tag       "<<match_tag<<std::endl;
//			debugPrint->onePrint<Port_inout>(couple_se->output_port_2array, "outport");
			Debug::getInstance()->getPortFile() << "lse" << index << " ";
			debugPrint->onePrint<Port_inout>(output_port_2array, "outport2");
			debugPrint->linePrint("   this is step1");
			Debug::getInstance()->getPortFile() << "lse" << index << " ";
			debugPrint->vecPrint<Port_inout>(input_port_pe, "input_pe");
			debugPrint->linePrint("   this is bp");
			debugPrint->vecPrint<Bool>(nextpe_bp, "nextpe_bp");
			debugPrint->vecPrint<Bool>(this_bp, "this_bp");
		}
	}
	else if (debugPrint->debug_level <= 1) {
		if (attribution->ls_mode == LSMode::load)
		{
			debugPrint->onePrint<Port_inout_lsu>(input_port_lsu[0], "input_port_lsu");
			debugPrint->onePrint<Port_inout_lsu>(output_port_2lsu, "outputport_lsu");
			debugPrint->vecPrint<Port_inout>(input_port_pe, "input_pe");
			debugPrint->onePrint<Port_inout>(output_port_2array, "output_pe");
		}
		else {
			debugPrint->vecPrint<Port_inout>(input_port_pe, "input_pe");
			debugPrint->onePrint<Port_inout>(output_port_2array, "outport2");
		}
	}
}
void Loadstore_element::value_update(uint &update_tag,const uint &last_tag)
{
	update_tag = last_tag + 1;

	switch (attribution->size)
	{
	case BufferSize::small:
		if (last_tag == system_parameter.le_outbuffer_depth_small-1)
			update_tag = 0;
		break;

	case BufferSize::middle:
		if (last_tag == system_parameter.le_outbuffer_depth_middle-1)
			update_tag = 0;
		break;

	case BufferSize::large:
		if (last_tag == system_parameter.le_outbuffer_depth_large-1)
			update_tag = 0;
		break;
	}
}
void Loadstore_element::sedSimStep2()
{
	uint clk = ClkDomain::getInstance()->getClk();
	if (nextlsu_bp)
	{
		if (!firstlsu) {
			outbuffer->resetTag(sended_tag);
			coupleSeReset(sended_tag);
			nextlsu_bp = false;
			value_update(first_seektag,sended_tag);
		}
	}
	//对于写请求来说，需要实现一个匹配的过程
	outbuffer->getTagMatchIndex(validDataIndex, 0, first_seektag);
	couple_se->outbuffer->getTagMatchIndex(validAddrIndex, 0, first_seektag);
	vecs.push_back(std::move(validDataIndex));
	vecs.push_back(std::move(validAddrIndex));

	match_tag = Util::findSameValueInVectors<uint>(vecs);

	vecs.clear();
	if (match_tag != UINT_MAX)
	{
//		if (nextlsu_bp)
//		{
			if (firstlsu) {
				firstlsu = false;/////////////////////////应该在找到这个tag后才置firstlsu为false，没找到的时候不能瞎清////////
			}
//			nextlsu_bp = false;//找到了之后才能发，这时候才能置这个bp为false
//		}
		outbuffer->output_ack(output_port_2array, match_tag);
		CoupleSeOutput(couple_se->output_port_2array, match_tag);
		
		if (!couple_se->output_port_2array.condition || !output_port_2array.condition)
		{
			outbuffer->resetTag(match_tag);
			coupleSeReset(match_tag);
			value_update(first_seektag, match_tag);
		}
		else if (couple_se->output_port_2array.last || output_port_2array.last)
		{
			output_port_2array.valid = true;
			output_port_2array.value_bool = true;
			output_port_2array.last = true;
			outbuffer->resetTag(match_tag);
			coupleSeReset(match_tag);
			value_update(first_seektag, match_tag);
		}
		else
		{
			//output_port_2lsu赋值
			output_port_2lsu.valid = couple_se->output_port_2array.valid && output_port_2array.valid;
			output_port_2lsu.value_addr = couple_se->output_port_2array.value_data;
			output_port_2lsu.value_data = output_port_2array.value_data;
			output_port_2lsu.rdwr = true;
			output_port_2lsu.dae = attribution->dae;
			output_port_2lsu.tag = match_tag;
			lsu->AddTrans(output_port_2lsu, index);////////////给output_port_2lsu赋值////////////////
			MemoryData::getInstance()->write(output_port_2lsu.value_addr, output_port_2lsu.value_data);
			sended_tag = match_tag;
		}
//		if (!couple_se->output_port_2array.last && couple_se->output_port_2array.condition)
//		{
//			//output_port_2lsu赋值
//			output_port_2lsu.valid = couple_se->output_port_2array.valid && output_port_2array.valid;
//			output_port_2lsu.value_addr = couple_se->output_port_2array.value_data;
//			output_port_2lsu.value_data = output_port_2array.value_data;
//			output_port_2lsu.rdwr = true;
//			output_port_2lsu.dae = attribution->dae;
//			output_port_2lsu.tag = tag_counter;
//			lsu->AddTrans(output_port_2lsu,index);////////////给output_port_2lsu赋值////////////////
//			MemoryData::getInstance()->write(output_port_2lsu.value_addr, output_port_2lsu.value_data);
//			sended_tag = match_tag;
//		}
//		else if (!couple_se->output_port_2array.condition)                  //false分支处理
//		{
////			outbuffer->outputTag(output_port_2array, tag_counter);
//			outbuffer->resetTag(match_tag);
//			coupleSeReset(match_tag);
//		}
//		else if (couple_se->output_port_2array.last)                     //last处理
//		{
//			output_port_2array.valid = true;
//			output_port_2array.value_bool = true;
//			output_port_2array.last = true;
//			outbuffer->resetTag(match_tag);
//			coupleSeReset(match_tag);
//		}
	}
}

void Loadstore_element::simBp()
{
	if (attribution->ls_mode == LSMode::dummy) {
		for (uint i = 0; i < this_bp.size(); ++i)
			this_bp[i] = outbuffer->isBufferNotFull(0);
	}
	else {
		if (attribution->ls_mode == LSMode::load)
		{
			for (uint i = 0; i < this_bp.size(); ++i)
				this_bp[i] = inbuffer_bp->getBpTag(i, tag_counter);
		}
		else
		{
			for (uint i = 0; i < this_bp.size(); ++i)
				this_bp[i] = outbuffer_bp->getBpTag(i, tag_counter);
		}
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

void Loadstore_element::CoupleSeOutput(Port_inout& out, uint tag)
{
	couple_se->CoupledSeOutput(out, tag);
}

bool Loadstore_element::isOutBufferNotEmpty()
{
	return outbuffer->isBufferNotEmpty(0);
}

void Loadstore_element::CoupledSeOutput(Port_inout& out, uint tag)
{
	outbuffer->output_ack(out, tag);
}

void Loadstore_element::resetByCouple(uint tag)
{
	outbuffer->resetTag(tag);
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
		lse_set[i]->outbuffer->getTagMatchIndex(temp, 0,0);
		valid_tag_set[i] = std::move(temp);
	}
	uint match_tag = Util::findSameValueInVectors<uint>(valid_tag_set);

	bool joined_bp = true;
	for (auto& i : lse_set)
		joined_bp &= i->nextpe_bp[0];

	if (match_tag != UINT_MAX && joined_bp)
	{
		for (auto& i : lse_set)
		{
			if (i->attribution->ls_mode == LSMode::load)
				i->outbuffer->output(i->output_port_2array, 0);//应该改成outputTag，并且是在所有match的lse仿真之前做(),另外要把PE中的这个输出部分删掉
		}
	}
}

