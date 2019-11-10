#include "LC.h"
#include <iomanip>
using namespace Simulator::Array;

LoopControl::LoopControl(const Simulator::Preprocess::ArrayPara para, uint index):Node(para), nextbp(system_parameter.lc_output_num, true)
{
	this->index = index;
	auto dict = Preprocess::DFG::getInstance()->getDictionary();
	auto lc_vec = dict.find(Simulator::NodeType::lc)->second;
	attribution = dynamic_cast<const Preprocess::DFGNode<Simulator::NodeType::lc>*>(lc_vec[index]);
	reg.resize(system_parameter.lc_regin_num);
	end_input.resize(system_parameter.lc_endin_num);
	reg_input.resize(system_parameter.lc_regin_num);
	lc_output.resize(system_parameter.lc_output_num);
	relay_input.resize(system_parameter.lc_endin_num);
	reg[1].valued = attribution->reg[0];
	reg[2].valued = attribution->reg[1];
	reg[3].valued = attribution->reg[2];
	reg[1].valid = true;
	reg[2].valid = true;
	reg[3].valid = true;

	end_reg.resize(system_parameter.lc_endin_num);
	muxout_buffer = Buffer_factory::createLcBuffer(para);
	first_loop = true;
	if (system_parameter.stall_mode == stallType::none) {
		#define stall
	}
	thisbp_end.resize(system_parameter.lc_endin_num);
	thisbp_reg.resize(system_parameter.lc_regin_num);
	//nextbp.resize(system_parameter.lc_output_num);
	reg_protocol.resize(reg.size());
	endreg_protocol.resize(end_reg.size());
	for (uint i = system_parameter.lc_regin_num; i < system_parameter.lc_endin_num; i++) {
		if (attribution->input_vec[i].source_node_type != NodeType::null)
			end_innum++;
	}
	end_flag.resize(end_innum);
	if (attribution->outermost) {
		last_flag = true;
	}
	DEBUG_ASSERT(end_innum > 0);
	for (uint i = 0; i < reg.size(); ++i)
		reg_protocol[i] = Bp_factory::createRegBp(para, &reg[i]);

	for (uint i = 0; i < end_reg.size(); ++i)
		endreg_protocol[i] = Bp_factory::createRegBp(para, &end_reg[i]);
}
LoopControl::~LoopControl()
{
	delete muxout_buffer;
	for (uint i = 0; i < reg.size(); ++i)
		delete reg_protocol[i];
	for (uint i = 0; i < end_reg.size(); ++i)
		delete endreg_protocol[i];
}
void LoopControl::update()
{
	simStep3();
	simStep2();
	simStep1();

	simBp();
//	print();
//	wireReset();
	
}

//使用debug类的打印方法进行打印
void LoopControl::wirePrint()
{
	const auto& debugPrint = Debug::getInstance();
	if (debugPrint->debug_level == 2) {
		debugPrint->linePrint("   this is step3");
		debugPrint->onePrint<Port_inout>(bufferout, "bufferout");
		debugPrint->onePrint<Port_inout>(stepout, "stepout");
		debugPrint->onePrint<Port_inout>(combout, "combout");
		Debug::getInstance()->getPortFile() << "lc" << attribution->index << " ";
		debugPrint->onePrint<Port_inout>(lc_output[1], "lc_output");
		Debug::getInstance()->getPortFile() << "lc" << attribution->index << " ";
		debugPrint->onePrint<Port_inout>(lc_output[3], "lc_output3");
		Debug::getInstance()->getPortFile() << "lc"<<attribution->index<<"df_cnt " << df_cnt << std::endl;
		debugPrint->linePrint("   this is step2");
		debugPrint->onePrint<Port_inout>(loopbegin, "loopbegin");
		debugPrint->onePrint<Port_inout>(muxout, "muxout");
		debugPrint->linePrint("   this is step1");
		debugPrint->vecPrint<Port_inout>(reg_input, "reg_input");
		debugPrint->vecPrint<Port_inout>(end_input, "end_input");
		debugPrint->vecPrint<Bool>(nextbp, "nextbp");
		debugPrint->vecPrint<Bool>(thisbp_end, "thisbp_end");
		debugPrint->vecPrint<Bool>(thisbp_reg, "thisbp_reg");

	}
	else if (debugPrint->debug_level <= 1) {
		Debug::getInstance()->getPortFile() << "lc" << attribution->index << " ";
		debugPrint->vecPrint<Port_inout>(reg_input, "reg_input");
		debugPrint->vecPrint<Port_inout>(end_input, "end_input");
		debugPrint->vecPrint<Port_inout>(lc_output, "lc_output");
//		debugPrint->linePrint("   this is step3");
	}
	else { ; }
}
void LoopControl::print()
{
	if (ClkDomain::getInstance()->getClk() >= Debug::getInstance()->print_file_begin && ClkDomain::getInstance()->getClk() < Debug::getInstance()->print_file_end) {
		//if (Debug::getInstance()->debug_level == 1) {
		//	Debug::getInstance()->getRegFile() << "--------LC " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
		//	muxout_buffer->print(Debug::getInstance()->getRegFile());
		//}
		Debug::getInstance()->getPortFile() << "--------LC " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
		wirePrint();
	}
}

void LoopControl::buffer_print()
{
	if (ClkDomain::getInstance()->getClk() >= Debug::getInstance()->print_file_begin && ClkDomain::getInstance()->getClk() < Debug::getInstance()->print_file_end) {
		if (Debug::getInstance()->debug_level == 2) {
			Debug::getInstance()->getRegFile() << "--------LC " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
			Debug::getInstance()->getRegFile() << "--------reg endreg " << "--------" << std::endl;
			Debug::getInstance()->vecPrint<Bufferd>(reg, "reg");
			Debug::getInstance()->vecPrint<Bufferd>(end_reg, "end_reg");
			muxout_buffer->print(Debug::getInstance()->getRegFile());
		}
		else if (Debug::getInstance()->debug_level <= 1) {
			Debug::getInstance()->getRegFile() << "--------LC " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
			muxout_buffer->print(Debug::getInstance()->getRegFile());
		}
	}
}
void LoopControl::wireReset()
{
	for (auto& i : reg_input)
		i.reset();
	for (auto& i : end_input)
		i.reset();
	//for (auto& i : lc_output)
		//i.reset();

	loopbegin.reset();
	muxout.reset();
	stepout.reset();
	bufferout.reset();
	combout.reset();
	for (auto& i : lc_output)
		i.reset();
}

//在reg_input中将first_loop置为了true，但后面如果再有有效的输出将会使first_loop保持为true，因此需要将output清空(应在send完之后做)//////////
//void LoopControl::outReset() {
//	for (auto& i : lc_output)
//	i.reset();
//}
void LoopControl::regInput(Port_inout input, uint port)
{
	if (input.valid)
	{
//		DEBUG_ASSERT(!reg[port].valid); //需要去掉注释
		reg[port].valid = true;
		reg[port].valued = input.value_data;
		reg[port].last = input.last;
		reg[port].condition = input.condition;
		reg[port].valueb = input.value_bool;
		if (port == 0) {
			first_loop = true;
		//	if (!attribution->outermost && !reg[port].last) { df_cnt++; }
			if (!attribution->outermost) { df_cnt++; }
		}
	}
}

void LoopControl::endInput(Port_inout input, uint port)
{
	if (input.valid)
	{
//		DEBUG_ASSERT(!end_reg[port].valid);
		end_reg[port].valid = true;
		end_reg[port].valued = input.value_data;
		end_reg[port].last = input.last;
		if (!attribution->outermost&&input.last&&port==0) {
			df_cnt--;
		//	DEBUG_ASSERT(!(df_cnt < 0));
		}
	}
}

void LoopControl::simStep1()
{
	for (uint i = 0; i < reg_input.size(); i++) {
		if (reg_input[i].valid)
			regInput(reg_input[i], i);
	}
	for (uint i = 0; i < end_input.size(); i++) {
		if (end_input[i].valid) {//只有last的时候才能进end_reg
			endInput(end_input[i], i);
		}
	}

}

void LoopControl::simStep1(uint i)
{
	if (i < reg_input.size()) {
		if (reg_input[i].valid)
			regInput(reg_input[i], i);
	}
	else {
#ifdef stall
		if (relay_input[i - reg_input.size()].valid) {//只有last的时候才能进end_reg
			endInput(relay_input[i - reg_input.size()], i - reg_input.size());
		}
		relay_input[i - reg_input.size()] = end_input[i - reg_input.size()];
#endif // stall

#ifndef stall
		if (end_input[i - reg_input.size()].valid) {
			endInput(end_input[i - reg_input.size()], i - reg_input.size());
		}
#endif // !stall

	}

}

//void LoopControl::simall()
//{
//	for (uint i = 0; i < reg_input.size(); i++)
//	{
//		if (reg_input[i].valid)
//			regInput(reg_input[i], i);
//	}
//
//	for (uint i = 0; i < end_input.size(); i++)
//	{
//		if (end_input[i].valid) {
//			endInput(end_input[i], i);
//		}
//		relay_input[i] = end_input[i - reg_input.size()];
//	}
//}

//lc内部由于没有inbuffer，使用ack机制，muxbuffer接收到数后进行一定的清数行为
void LoopControl::simStep2()
{
	//loopbegin
	//if (reg[0].valid&&!reg[0].last)
	if (reg[0].valid )
		loopbegin.valid = true;
	//else if (attribution->outermost)
	//{
	//	loopbegin.valid =              //全局begin节点的输入！   //INSPECTION
	//}

	loopbegin.value_bool = first_loop;

	if (loopbegin.valid)
	{
		if (loopbegin.value_bool && reg[1].valid)
		{
			muxout.value_data = reg[1].valued;
			muxout.valid = true;
//			muxout.condition = true;
			muxout.value_bool = false;
			muxout.last = false;
			if (reg[0].condition == false||reg[0].last) {
			//if (reg[0].condition == false ) {
				muxout.condition = false;
				muxout.last = true;
			}
			else {
				muxout.condition = true;
				muxout.last = false;
			}
		}
		else if (!loopbegin.value_bool && stepout.valid)
		{
			muxout = stepout;
		}
	}

	if (muxout_buffer->isBufferNotFull(0))
	{
	//	if (muxout.valid && muxout.last)         //当combine输出为10时代表循环结束
	//		reg[0].valid = false;                         //i值无效化

		if(muxout_buffer->input(muxout, 0))
			first_loop = false;
	}
}

void LoopControl::simStep3()
{
	bool bpok = true;
	for (auto& i : nextbp)
	{
		bpok = bpok & i;
	}

	if (bpok)
		muxout_buffer->output(bufferout, 0);

    //step calculate
	if (reg[3].valid)
	{
		if (!bufferout.last&&bufferout.value_data < reg[2].valued) {//大于临界值的数就不要再算了
			stepout.valid = bufferout.valid;
			stepout.value_data = bufferout.value_data + reg[3].valued;
		}
		else {
			stepout.valid = false;
		}
	}
	if (bufferout.value_data >= reg[2].valued||bufferout.last) {//一旦reg[0].last就永远清不了了
		reg[0].valid = false;
		reg[0].valued = 0;
		reg[0].valueb = false;
		reg[0].last = false;//需要这个last来查看是否结束
		reg[0].condition = true;
	}
	//compare calculate
	if (reg[2].valid)
	{
		combout.valid = bufferout.valid;
		combout.value_bool = bufferout.value_data < reg[2].valued;//stepout和combout是否可以合并，last应该是muxout信号上进行判断
/*		if (!combout.value_bool)
			stepout.last = true;*/
	}

	//i
//	if(bufferout.last)
//	lc_output[0].valid = combout.valid & combout.value_bool & reg[0].valid;
	lc_output[0].valid = false;
	lc_output[0].value_data = reg[0].valued;
	lc_output[0].last = bufferout.last ? true: !combout.value_bool;
	//if(reg[0].valid&&reg[0].condition==false)
	//	lc_output[0].last = true;
	lc_output[0].value_bool = false;
//	lc_output[0].condition = combout.value_bool;
	lc_output[0].condition = bufferout.condition;
	//j
	lc_output[1].valid = bufferout.valid;
	lc_output[1].value_data = bufferout.value_data;
//	lc_output[1].last = bufferout.last;
	lc_output[1].last = bufferout.last ? true: !combout.value_bool;
	//if (reg[0].valid && reg[0].condition == false)
	//	lc_output[1].last = true;
	lc_output[1].value_bool = false;
//	lc_output[1].condition = combout.value_bool;
	lc_output[1].condition = bufferout.condition;
	//loopsignal
//	lc_output[2].valid = combout.valid;
	lc_output[2].valid = false;
	lc_output[2].value_bool = combout.value_bool;
	lc_output[2].last = bufferout.last ? true: !combout.value_bool;
//	lc_output[2].value_bool = false;
//	lc_output[2].condition = combout.value_bool;
	lc_output[2].condition = bufferout.condition;

	//endreg sim
	//bool end_loop = end_flag[0];
//	bool getvalid = false;
//	for (uint i = 0; i < end_reg.size(); i++)
//		if (end_reg[i].valid)
////			getvalid = true;
////			end_loop &= (end_reg[i].last && end_reg[i].valid);
//			end_loop = end_loop || end_reg[i].last;

	for (uint i = 0; i < end_reg.size(); i++) {
		if (end_reg[i].valid && end_reg[i].last) {
			end_flag[i] = true;
//			end_loop = end_loop || end_flag[i];
		}
	}
	bool end_loop = end_flag[0];
	for(auto &i:end_flag){end_loop= end_loop && i;}

	if (reg[0].last && reg[0].valid)
	{
		last_flag = true;
		//reg[0].valid = false;
		//reg[0].valued = 0;
		//reg[0].valueb = false;
		//reg[0].last = false;//需要这个last来查看是否结束
		//reg[0].condition = true;
	}
	if (last_flag &&end_loop&&df_cnt==0)
	{
		lc_output[3].valid = true;
		lc_output[3].value_bool = true;
		lc_output[3].condition = true;
		lc_output[3].last = true;
		last_flag = false;
		for (auto& i : end_flag) { i = false; }
	}
}

void LoopControl::simBp()
{
	for (uint i = 0; i < reg_protocol.size(); ++i)
		thisbp_reg[i] = reg_protocol[i]->getBp();

	for (uint i = 0; i < endreg_protocol.size(); ++i)
//		thisbp_end[i] = endreg_protocol[i]->getBp();
		thisbp_end[i] = true;
}

void LoopControl::getInput(uint port, Port_inout input)
{
	if (port < reg_input.size())
		reg_input[port] = input;
	else
		end_input[port - reg_input.size()] = input;
}

void LoopControl::getBp(uint port, bool input)
{
	nextbp[port] = input;
}





