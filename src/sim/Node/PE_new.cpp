#include "PE.h"
#include <iomanip>

using namespace Simulator::Array;

Processing_element::Processing_element(const Simulator::Preprocess::ArrayPara para, uint index) :Node(para)
{
	this->index = index;
	auto dict = Preprocess::DFG::getInstance()->getDictionary();
	auto pe_vec = dict.find(Simulator::NodeType::pe)->second;
	attribution = dynamic_cast<const Preprocess::DFGNode<Simulator::NodeType::pe>*>(pe_vec[index]);

	alu = new Pealu(attribution->opcode);

	//通过传systempara信息，由工厂类获得各个模块的指针
	inbuffer = Buffer_factory::createInBuffer(para);
	outbuffer = Buffer_factory::createOutBuffer(para);
	//alu = Alu_factory::createAlu()
	in_num = system_parameter.data_inport_breadth + system_parameter.bool_inport_breadth;
	input_port.resize(in_num);
	local_reg.resize(in_num);
	for (auto& lr : local_reg)
		lr = new Localreg();
//	aluin.resize(in_num);
//	alu_mask.resize(system_parameter.data_inport_breadth + system_parameter.bool_inport_breadth);
//	ib2alu_mask.resize(system_parameter.data_inport_breadth + system_parameter.bool_inport_breadth);
	//buffer_reset.resize(system_parameter.data_inport_breadth + system_parameter.bool_inport_breadth);
	break_state.resize(in_num);
	thispe_bp.resize(in_num);
	reg_out.resize(in_num);
	ctrl_index = in_num - 1;
	next_bp.resize(system_parameter.data_outport_breadth + system_parameter.bool_outport_breadth);
	output_port.resize(system_parameter.data_outport_breadth + system_parameter.bool_outport_breadth);
//	in_num = system_parameter.data_inport_breadth + system_parameter.bool_inport_breadth;
	switch (attribution->opcode)
	{
		case PEOpcode::null:
	//		DEBUG_ASSERT(false);
			alu_num = 1;
			break;
		case PEOpcode::add:
			alu_num = 2;
			break;
		case PEOpcode::mul:
			alu_num = 2;
			break;
		case PEOpcode::sub:
			alu_num = 2;
			break;
		case PEOpcode::div:
			alu_num = 2;
			break;
		case PEOpcode::mod:
			alu_num = 2;
			break;
		case PEOpcode::lt:
			alu_num = 2;
			break;
		case PEOpcode::eq:
			alu_num = 2;
			break;
		case PEOpcode::mux:
			alu_num = 3;
			break;
		case PEOpcode::mac:
			alu_num = 3;
			break;
		case PEOpcode::add1:
			alu_num = 1;//
			break;
		case PEOpcode::leshift:
			alu_num = 1;//
			break;
		case PEOpcode::rishift:
			alu_num = 1;
			break;
		case PEOpcode::bit_and:
			alu_num = 2;
			break;
		case PEOpcode::bit_or:
			alu_num = 2;
			break;
		case PEOpcode::cos:
			alu_num = 2;
			break;
		case PEOpcode::sin:
			alu_num = 2;
			break;
		case PEOpcode::bit_xor:
			alu_num = 2;
			break;
		case PEOpcode::smux:
			alu_num = 2;
			break;
		case PEOpcode::neq:
			alu_num = 2;
			break;
		case PEOpcode::lte:
			alu_num = 2;
			break;
		case PEOpcode::bit_not:
			alu_num = 2;//
			break;
		case PEOpcode::merge:
			alu_num = 2;
			break;
		default:
			DEBUG_ASSERT(false);
			alu_num = 0;
			break;
	}
	//if (attribution->control_mode == ControlMode::loop_activate) {
	//	if (attribution->opcode != PEOpcode::mac){ DEBUG_ASSERT(false); }
	//	else { alu_num++; }
	//}
	aluin.resize(alu_num==1? 2: alu_num);
	for (auto&& i : break_state)
		i = false;

	inbuffer_out.resize(system_parameter.data_inport_breadth + system_parameter.bool_inport_breadth);

	pebp = Bp_factory::createPeBp(para, this);

	for(auto& i : input_port)
		i = Port_inout();

	for (auto& i : output_port)
		i = Port_inout();

	for (auto& i : inbuffer_out)
		i = Port_inout();

	for (auto& i : aluin)
		i = Port_inout();

	for (auto& i : next_bp)
		i = true;

	for(auto &i:reg_out)
		i = Port_inout();

	alu_out = Port_inout();

	
	parameterSet();
	//Mpr::SimQueryTable::callBackPEBpRegister(getBp, index);
	//Mpr::SimQueryTable::callBackPEInputRegister(getInput, index);
}

Processing_element::~Processing_element()
{
	delete inbuffer;
	delete outbuffer;
	delete pebp;
	delete alu;
	for(auto &lr:local_reg)
		delete lr;
	delete instruction_buffer;
}

void Processing_element::parameterSet()
{
	if (attribution->reg.size() == 1) {
		if (attribution->reg[0] != INT_MAX)
		{
			local_reg[1]->reg_v = true;
			local_reg[1]->setvalue(attribution->reg[0]);
		}
	}
	else {
		DEBUG_ASSERT(attribution->reg.size() == 3)
			for (uint i = 0; i < attribution->reg.size() ;i++) {
				if (attribution->reg[i] != INT_MAX)
				{
					local_reg[i]->reg_v = true;
					local_reg[i]->setvalue(attribution->reg[i]);
				}
			}
	}
}

void Processing_element::update()
{
	simInstruction();
	simStep3();
	simStep2();
//	simStep1();

	//inbufferReset();        //INSPECTION
	simBp();
	print();
	wireReset();
}

void Processing_element::getInput(uint port, Port_inout input)
{
	input_port[port] = input;
}

void Processing_element::getBp(uint port, bool input)
{
	next_bp[port] = input;
}

void Processing_element::simStep1(uint i)
{
//	for (uint i = 0; i < (system_parameter.bool_inport_breadth + system_parameter.data_inport_breadth); ++i)
//	{
		if (break_state[i])
		{
			if (input_port[i].last && input_port[i].valid)
			{
				break_state[i] = false;       //当输入last信号时该端口的break状态结束
				//由于last携带的信号并没不携带数据所以不需要进入buffer
			}
			else if (!input_port[i].last && input_port[i].valid)
			{
				//input不进入inbuffer
				//bp仍返回   ack-入数成功/credit-可以入数
			}
		}
		else
		{
			if (attribution->input_bypass != InputBypass::bypass) {
				if (i == 1 && attribution->inbuffer_from[1] == InBufferFrom::aluin1) {
					if (!local_reg[1]->reg_v) {
						if (input_port[i].valid) {
							local_reg[1]->input(input_port[i]);
							inbuffer->input(input_port[i], i);
							//							local_reg[1]->reg_v = true;
							first_loop = false;
						}
					}
					else {
						if(alu_out.condition&&!alu_out.last&&alu_out.valid)
							inbuffer->input(this->alu_out, 1);//最后一个带last的信号不会进buffer，因为这时候last已经为true,reg_v已经被置位
					}
				} 
	//			else if (i == 1 && attribution->buffer_mode[1] == BufferMode::lr_out) {
	//	//			if (input_port[i].last&&!local_reg[i]->reg_v) {//这个输入后不会置reg_v，但需要reg_v为false才能进入
	//					local_reg[1]->input_transout(input_port[i]);
	//					//if (input_port[i].valid && input_port[i].condition)
	//					//	if (!input_port[i].last)
	//					//		inbuffer->input(input_port[i], i);
	//					//	else {
	//					//		inbuffer->back_head(0);
	//					//		local_reg[1]->reg_v = true;
	//					//	}
	////				}
	//			}
				else{
					if (input_port[i].valid) {
						if (i == 1) {
							if (attribution->control_mode == ControlMode::loop_activate) {
								if (!local_reg[1]->reg_v) {
									local_reg[1]->input(input_port[i]);
									inbuffer->input(input_port[1], 1);
									//local_reg[1]->reg_v = true;
									first_loop = false;
								}
								else {
									//inbuffer->input(input_port[0], 1);
									//								first_loop = false;
								}
							}
							else {
								if (i == 0 && attribution->control_mode == ControlMode::loop_activate) { 
									if (local_reg[1]->reg_v) {
										inbuffer->input(input_port[0], 1);
									}
								}
								else {
									if (attribution->inbuffer_from[1] == InBufferFrom::in1 && !local_reg[1]->reg_v && (attribution->buffer_mode[1] == BufferMode::lr)) {
										local_reg[1]->input(input_port[i]);
									}
									else if (attribution->buffer_mode[1] == BufferMode::buffer) {
										if (input_port[i].valid)
											inbuffer->input(input_port[i], i);
									}
									else if (attribution->buffer_mode[1] == BufferMode::lr_out) {
										if (input_port[i].valid && input_port[i].last)
											inbuffer->input_nolast(input_port[i], i);
									}
								}
							}
						}
						else {
							//if (attribution->lr_from == LocalregFrom::in1 && i == 1&& !local_reg->reg_v)
							//	local_reg->input(input_port[i]);{
							if (i == 0 && attribution->inbuffer_from[0] == InBufferFrom::in0 && !local_reg[0]->reg_v && attribution->buffer_mode[0] == BufferMode::keep)
								local_reg[0]->input(input_port[i]);
							else if (i == 2 && attribution->inbuffer_from[2] == InBufferFrom::in2 && !local_reg[2]->reg_v && attribution->buffer_mode[2] == BufferMode::keep)
								local_reg[2]->input(input_port[i]);
							else if (attribution->buffer_mode[i] == BufferMode::buffer) {
								inbuffer->input(input_port[i], i);
							}
							else if (attribution->buffer_mode[i] == BufferMode::lr_out) {
								if (input_port[i].valid && input_port[i].last)
									inbuffer->input_nolast(input_port[i], i);
							}
							
						}
					}
				}
			}
		}

	instruction_buffer->update();
}

void Processing_element::loopControlParse()
{
	//此时bool端口信号单独生效
	//if (!controlBufferCanBeReset())
		//return;
	//if (attribution->control_mode == ControlMode::loop_activate)
	//{
	//	//控制信号解读
	//	//if (control_value.valid)
	//	//{
	//	//	if (control_value.value_bool)     //代表循环开始
	//	//	{
	//	//		if (local_reg[1]->reg_v)
	//	//		{
	//	//			local_reg[1]->output(reg_out[1]);
	//	//			reg_out[1].last = control_value.last;
	//	//			reg_out[1].last = control_value.last;
	//	//		}
	//	//	}
	//	//	else
	//	//	{
	//	//		local_reg[1]->reset();
	//	//		inbuffer->reset();
	//	//	}
	//	//}
	//}
	//else    //在非loopcontrol情况下localreg主动输出
	//{
		//if (attribution->control_mode == ControlMode::loop_reset)
		//{
		//	if (control_value.valid)
		//	{ 
		//		if (!control_value.value_bool)     //代表循环开始
		//		{
		//			local_reg[1]->reset();
		//			inbuffer->reset();
		//		}
		//	}
		//}
//	if (attribution->output_from[0] == OutputFrom::alu && attribution->opcode == PEOpcode::null) {
		local_reg[0]->output(reg0_out);
		local_reg[1]->output(reg1_out);
		local_reg[2]->output(reg2_out);
		reg_out[0] = reg0_out;
		reg_out[1] = reg1_out;
		reg_out[2] = reg2_out;
	
	//}
}

void Processing_element::getAluInput()
{
	//alu输入 其中aluin[2]用于mux选择端或用于控制
	if (allAluOperandsGot())
	{
		oprand_collected = true;
			DEBUG_ASSERT(!(attribution->control_mode == ControlMode::loop_activate && attribution->opcode == PEOpcode::null));
			if (attribution->control_mode == ControlMode::loop_activate) {
				if (attribution->input_bypass == InputBypass::inbuffer && (attribution->buffer_mode[2] == BufferMode::buffer)) {
					//if (alu->canReceiveInput() && !local_reg[1]->reg_v) {
					//	aluin[0] = reg_out[1];
					//	inbuffer->output(aluin[1], 2);
					//}
					if (alu->canReceiveInput() && local_reg[1]->reg_v) {
						DEBUG_ASSERT(alu_num < 2);
//						inbuffer->output(aluin[0], 1);
						inbuffer->output(aluin[1], 1);
					//	inbuffer->output(aluin[1], 2);
					}
				}
				else if (attribution->input_bypass == InputBypass::inbuffer && (attribution->buffer_mode[2] == BufferMode::keep || attribution->buffer_mode[2] == BufferMode::lr)) {
					//if (alu->canReceiveInput() && !local_reg[1]->reg_v) {
					//	aluin[0] = reg_out[1];
					//	aluin[1] = reg_out[2];
					//}
					if (alu->canReceiveInput() && local_reg[1]->reg_v) {
						inbuffer->output(aluin[1], 1);
						if (alu_num > 1)
							aluin[0] = reg_out[0];
					}
				}
			}

			else {
				for (uint i = 0; i < alu_num; ++i) {
					int search_index = alu_num == 1 ? 1 : i;
					if (attribution->opcode == PEOpcode::null) {
						if (outbuffer->isBufferNotFull(0) && attribution->input_bypass == InputBypass::inbuffer && (attribution->buffer_mode[i] == BufferMode::buffer || attribution->inbuffer_from[i] == InBufferFrom::aluin1))//alu来源于inbuffer
							inbuffer->output(aluin[search_index], search_index);
						else if (outbuffer->isBufferNotFull(0) && attribution->input_bypass == InputBypass::inbuffer && (attribution->buffer_mode[i] == BufferMode::keep || attribution->buffer_mode[i] == BufferMode::lr ) && attribution->inbuffer_from[i] != InBufferFrom::aluin1)//alu来源于lr
							aluin[search_index] = reg_out[search_index];
						else if (outbuffer->isBufferNotFull(0) && attribution->input_bypass == InputBypass::bypass)//alu来源于inport
							aluin[search_index] = input_port[search_index];
					}
					else {
						if (alu->canReceiveInput() && attribution->input_bypass == InputBypass::inbuffer && (attribution->buffer_mode[i] == BufferMode::buffer|| attribution->inbuffer_from[i] == InBufferFrom::aluin1))//alu来源于inbuffer
							inbuffer->output(aluin[search_index], search_index);
						else if (alu->canReceiveInput() && attribution->input_bypass == InputBypass::inbuffer && (attribution->buffer_mode[i] == BufferMode::keep || attribution->buffer_mode[i] == BufferMode::lr )&& attribution->inbuffer_from[i] != InBufferFrom::aluin1)//alu来源于lr
							aluin[search_index] = reg_out[search_index];
						else if (alu->canReceiveInput() && attribution->input_bypass == InputBypass::bypass)//alu来源于inport
							aluin[search_index] = input_port[search_index];
					}
				}
			}
			//else if (alu->canReceiveInput() && attribution->alu_in_from[i] == AluInFrom::first_loop)//alu来源于first_loop
			//{
			//	aluin[i].value_bool = first_loop;
			//	aluin[i].valid = true;
			//}


		if (attribution->control_mode == ControlMode::cb|| attribution->control_mode == ControlMode::cinvb|| attribution->control_mode == ControlMode::bind|| attribution->control_mode == ControlMode::transout || attribution->control_mode == ControlMode::loop_activate)
		{
			if (attribution->opcode != PEOpcode::null) {
				if (alu->canReceiveInput()) {
					if (attribution->input_bypass == InputBypass::inbuffer) {
						if (attribution->buffer_mode[ctrl_index] == BufferMode::buffer)
							inbuffer->output(control_value, ctrl_index);
						if (attribution->buffer_mode[ctrl_index] == BufferMode::keep || attribution->buffer_mode[ctrl_index] == BufferMode::lr)
							local_reg[2]->output(control_value);
					}
					else if (attribution->input_bypass == InputBypass::bypass) {
						control_value = input_port[2];
					}
				}
			}
			else {
				if (attribution->output_from[0] == OutputFrom::outbuffer) {
					if (outbuffer->isBufferNotFull(0)) {
						if (attribution->input_bypass == InputBypass::inbuffer) {
							if (attribution->buffer_mode[ctrl_index] == BufferMode::buffer)
								inbuffer->output(control_value, ctrl_index);
							if (attribution->buffer_mode[ctrl_index] == BufferMode::keep || attribution->buffer_mode[ctrl_index] == BufferMode::lr)
								local_reg[2]->output(control_value);
						}
						else if (attribution->input_bypass == InputBypass::bypass) {
							control_value = input_port[2];
						}
					}
				}
			}
			DEBUG_ASSERT(alu_num < in_num);
			if (attribution->control_mode == ControlMode::cb) {
				if (control_value.valid)
				{
					//if (!control_value.value_bool)
					if (!control_value.value_data)
						rs_cd = true;
				}
			}
			else if (attribution->control_mode == ControlMode::cinvb) {
				if (control_value.valid)
				{
					//if (control_value.value_bool)
					if (control_value.value_data)
						rs_cd = true;
				}
			}
			else if (attribution->control_mode == ControlMode::bind) {
				if (control_value.valid)
				{
					//if (control_value.value_bool)
					if (!control_value.value_data)
						alu_flag = true;
				}
				if (control_value.last&&control_value.valid)
					oprand_collected = false;
				else {
					for (auto& k : aluin) {
						if (k.valid && k.last) oprand_collected = false;
					}
				}
			}

		}
		
		//if (attribution->control_mode == ControlMode::transout) {
		//	if (attribution->output_from[0] == OutputFrom::alu) {
		//		if (next_bp[0])
		//		{
		//			if (attribution->input_bypass == InputBypass::inbuffer) {
		//				inbuffer->output(aluin[1], 1);
		//				if (attribution->buffer_mode[ctrl_index] == BufferMode::buffer)
		//					inbuffer->output(control_value, ctrl_index);
		//				else if (attribution->buffer_mode[ctrl_index] == BufferMode::keep)
		//					local_reg[2]->output(control_value);
		//				else { DEBUG_ASSERT(false);}
		//				if (control_value.valid && control_value.condition) {
		//					if (control_value.last != true) { oprand_collected = false; }
		//					else { //这个时候lr的值也应该是last的/////////////////
		//						//DEBUG_ASSERT()
		//						oprand_collected = true; 
		//						local_reg[1]->reg_v = false; 
		//					}
		//				}
		//				
		//			}
		//		}				
		//	}
			//else if(attribution->output_from[0] == OutputFrom::outbuffer){
			//	if (outbuffer->isBufferNotFull(0))
			//	{
			//		if (attribution->input_bypass == InputBypass::inbuffer) {
			//			inbuffer->output(aluin[0], 1);
			//			if (attribution->buffer_mode[ctrl_index] == BufferMode::buffer)
			//				inbuffer->output(control_value, ctrl_index);
			//			else if (attribution->buffer_mode[ctrl_index] == BufferMode::keep)
			//				local_reg[2]->output(control_value);
			//			else { DEBUG_ASSERT(false); }
			//			if (control_value.valid && control_value.condition) {
			//				if (control_value.last != true) { oprand_collected = false; }
			//				else { //这个时候lr的值也应该是last的/////////////////
			//					//DEBUG_ASSERT()
			//					oprand_collected = true;
			//					local_reg[1]->reg_v = false;
			//				}
			//			}

			//		}
			//	}
			//}
			//else { DEBUG_ASSERT(false); }
	//	}
		//else if(attribution->control_mode == ControlMode::cinvb)
		//{
		//	if (attribution->opcode != PEOpcode::null) {
		//		if (alu->canReceiveInput()) {
		//			if (attribution->input_bypass == InputBypass::inbuffer) {
		//				if (attribution->buffer_mode[ctrl_index] == BufferMode::buffer)
		//					inbuffer->output(control_value, ctrl_index);
		//				if (attribution->buffer_mode[ctrl_index] == BufferMode::keep || attribution->buffer_mode[ctrl_index] == BufferMode::lr)
		//					local_reg[2]->output(control_value);
		//			}
		//			else if (attribution->input_bypass == InputBypass::bypass) {
		//				control_value = input_port[2];
		//			}
		//		}
		//	}
		//	else {
		//		if (outbuffer->isBufferNotFull(0)) {
		//			if (attribution->input_bypass == InputBypass::inbuffer) {
		//				if (attribution->buffer_mode[ctrl_index] == BufferMode::buffer)
		//					inbuffer->output(control_value, ctrl_index);
		//				if (attribution->buffer_mode[ctrl_index] == BufferMode::keep || attribution->buffer_mode[ctrl_index] == BufferMode::lr)
		//					local_reg[2]->output(control_value);
		//			}
		//			else if (attribution->input_bypass == InputBypass::bypass) {
		//				control_value = input_port[2];
		//			}
		//		}
		//	}
		//	DEBUG_ASSERT(alu_num < in_num);
		//	if (control_value.valid)
		//	{
		//		for (auto& i : aluin) {
		//			if (i.condition == false)
		//				rs_cd = true;
		//				break;
		//		}
		//		//if (control_value.value_bool)
		//		if (control_value.value_data)
		//			rs_cd = true;
		//	}
		//}
			
		for (auto& i : aluin) {
			if (i.valid) {
				if (i.condition == false) {//清condition
					rs_cd = true;
					break;
				}
			}
		}
		if (control_value.valid && !control_value.condition) {
			rs_cd = true;
		}
		if (attribution->control_mode!=ControlMode::transout&& attribution->control_mode != ControlMode::trans) {
			//for (auto& aluinOne : aluin) {
	//		if (alu_in[i].valid) {
			//for (uint k = 0; k < aluin.size(); k++) {
			//	if (k == 1 && attribution->control_mode == ControlMode::loop_activate) {
			//		if (aluin[1].last) {
			//			for (uint i = 0; i < in_num; i++) {
			//				if ((attribution->buffer_mode[i] == BufferMode::keep || attribution->buffer_mode[i] == BufferMode::lr) && attribution->inbuffer_from[i] != InBufferFrom::flr)
			//					local_reg[i]->reg_v = false;
			//			}
			//			break;
			//		}
			//	}
			//	else if ((aluin[k].valid && aluin[k].last) || (control_value.valid && control_value.last)) {
			//		alu_flag = true;
			//		for (uint i = 0; i < in_num; i++) {
			//			if ((attribution->buffer_mode[i] == BufferMode::keep || attribution->buffer_mode[i] == BufferMode::lr) && attribution->inbuffer_from[i] != InBufferFrom::flr)
			//				local_reg[i]->reg_v = false;
			//		}
			//		break;
			//	}
			//}
			for (auto& aluinOne : aluin) {
				if ((aluinOne.valid && aluinOne.last) || (control_value.valid && control_value.last)) {
					alu_flag = true;
					for (uint i = 0; i < in_num; i++) {
						if ((attribution->buffer_mode[i] == BufferMode::keep || attribution->buffer_mode[i] == BufferMode::lr) && attribution->inbuffer_from[i] != InBufferFrom::flr)
							local_reg[i]->reg_v = false;
					}
					break;
				}
			}
			//if (attribution->control_mode == ControlMode::loop_activate) {
			//	for (auto& aluinOne : aluin) {
			//		if (aluinOne.valid && aluinOne.last) {
			//			alu_flag = true;
			//			for (uint i = 0; i < in_num; i++) {
			//				if ((attribution->buffer_mode[i] == BufferMode::keep|| attribution->buffer_mode[i] == BufferMode::lr) && attribution->inbuffer_from[i] != InBufferFrom::flr)
			//					local_reg[i]->reg_v = false;
			//			}
			//			break;
			//		}
			//	}
			//}
			//else {
			//	if (aluin[0].valid && attribution->buffer_mode[0] == BufferMode::keep && attribution->inbuffer_from[0] == InBufferFrom::in0 && (aluin[0].last == true || control_value.last)) {
			//		local_reg[0]->reg_v = false;
			//		alu_flag = true;
			//	}
			//	
			//	if (aluin[1].valid && attribution->buffer_mode[1] == BufferMode::lr && attribution->inbuffer_from[1] == InBufferFrom::in1 && (aluin[1].last == true || control_value.last)) {
			//		local_reg[1]->reg_v = false;
			//		alu_flag = true;
			//	}
			//	if (aluin[2].valid && attribution->buffer_mode[2] == BufferMode::keep && attribution->inbuffer_from[2] == InBufferFrom::in2 && (aluin[2].last == true || control_value.last)) {
			//		local_reg[2]->reg_v = false;
			//		alu_flag = true;
			//	}
			//}
//					first_loop = true;
	//		}
		//	}//清local_reg
		
			if (control_value.valid) {
				if(control_value.last)
					alu_flag = true;
			}
		}
	}
}

//进行控制判断后输入alu
//进行break_state的置位
void Processing_element::gatherOperands()
{
//	lastCheck();
	if (!oprand_collected)
		return;

//	lastCheck();//////////////////////////////////////////////////////////////放这里可以吗？？？？？？////////////
	
	if (attribution->control_mode == ControlMode::break_pre)
	{
	//	if (aluin[2].valid && aluin[2].value_bool)
		if (aluin[2].valid && aluin[2].value_data)
		{
			if (alu->input(aluin, attribution->opcode,true, rs_cd))
			{
				breakoccur();
				inbuffer->reset();       //进入break状态是清空inbuffer
			}
		}
		//else if (aluin[2].valid && !aluin[2].value_bool)
		else if (aluin[2].valid && !aluin[2].value_data)
		{
			alu->input(aluin, attribution->opcode,true, rs_cd);
		}
		else
			DEBUG_ASSERT(false);
	}
	else if (attribution->control_mode == ControlMode::break_post)
	{
		//if (aluin[2].valid && aluin[2].value_bool)
		if (aluin[2].valid && aluin[2].value_data)
		{
			breakoccur();
			inbuffer->reset();       //进入break状态是清空inbuffer
		}
		//else if (aluin[2].valid && !aluin[2].value_bool)
		else if (aluin[2].valid && !aluin[2].value_data)
		{
			alu->input(aluin,attribution->opcode,true, rs_cd);
		}
		else
			DEBUG_ASSERT(false);
	}
	else if (attribution->control_mode == ControlMode::continue_)
	{
		//if (aluin[2].valid && aluin[2].value_bool)
		if (aluin[2].valid && aluin[2].value_data)
		{
			//do nothing
		}
		//else if (aluin[2].valid && !aluin[2].value_bool)
		else if (aluin[2].valid && !aluin[2].value_data)
		{
			alu->input(aluin,attribution->opcode,true, rs_cd);
		}
		else
		    DEBUG_ASSERT(false);
	}
	//else if (attribution->control_mode == ControlMode::cb)
	//{
	//	if()
	//}
	else      //其他情况下，bool口不需要控制alu的行为
	{
		if (attribution->opcode != PEOpcode::null) {
			if (alu->canReceiveInput()) {
		//		bool alu_flag = false;
				for (auto& alun : aluin) {
					if (alun.last) { alu_flag = true; break; }
				}
				if (alu_flag)//如果带last信号，这个trans也加入alu pipeline队列
				{
					alu->input(aluin, attribution->opcode, false, rs_cd);//为了保持信号的顺序，还是要进alu的
					alu_flag = false;
				}
				else {
					alu->input(aluin, attribution->opcode, true, rs_cd);
				}
			}
		}
		else{
	//		bool alu_flag = false;
			for (auto& alun : aluin) {
				if (alun.valid&&alun.last) { alu_flag = true; break; }
			}
			if (aluin[1].valid) {
				if ( (rs_cd || alu_flag)) {
					if (rs_cd) {
						alu_out.value_data = aluin[1].value_data;
						alu_out.valid = true;
						alu_out.condition = false;
					}
					if (alu_flag) {
						alu_out.value_data = aluin[1].value_data;
						alu_out.valid = true;
						alu_out.last = true;
						alu_flag = false;
					}
				}
				else {
					alu_out.value_data = aluin[1].value_data;
					//alu_out.value_bool = aluin[0].value_bool;
					alu_out.valid = true;
				}
			}
		}
	}
	if (attribution->opcode == PEOpcode::null) { outbuffer->input(alu_out, 0); }
}

void Processing_element::breakoccur()
{
	for (auto& i : break_state)
	{
		DEBUG_ASSERT(!i);
		i = true;
	}
}

//void Processing_element::lastCheck()
//{
//	//if (aluin[0].last || aluin[1].last || aluin[2].last)
//	//{
//	//	lastflag = true;
//	//	//for (auto& i : aluin)
//	//		//DEBUG_ASSERT(i.last);//something wrong
//	//	/*
//	//	if (attribution->dfg_end)                        //数据流终点
//	//	{
//	//		if (alu->empty() && outbuffer->empty())
//	//		{
//	//			output_port[2].valid = true;
//	//			output_port[2].last = true;
//	//			inbufferReset();
//	//		}
//	//	}
//	//	*/
//	//	
//	//	if (lastflag && alu->empty() && outbuffer->empty())
//	//	{
//	//		for (uint i = 0; i < attribution->ob_from.size(); ++i)
//	//		{
//	//			if (attribution->ob_from[i] == OutBufferFrom::alu)
//	//			{
//	//				alu_out.valid = true;
//	//				alu_out.last = true;
//	//			}
//	//			//其他情况下，outbuffer的last信号能从其来源获得
//	//		}
//	//		first_loop = true;
//	//		lastflag = false;
//	//	}
//	//}
//	//信号带last可能与alu为空的情况不能同时存在，所以设置lastflag记录last信号的产生，将上一段注释了
//	if (aluin[0].last || aluin[1].last || aluin[2].last)
//	{
//		lastflag = true;
//	}
//
//		
////		if (lastflag && alu->empty() && outbuffer->empty())
//	if (lastflag && alu->empty() && outbuffer->empty())
//	{
//		//for (uint i = 0; i < attribution->ob_from.size(); ++i)
//		//{
//		//	if (attribution->ob_from[i] == OutBufferFrom::alu)
//		//	{
//		//		alu_out.valid = true;//输入信号last时就把alu_out置为true
//		//		alu_out.last = true;
//		//	}
//		//	//其他情况下，outbuffer的last信号能从其来源获得
//		//}
//		first_loop = true;
//		lastflag = false;
//	}
//	
//}
/*
void Processing_element::inbufferReset()
{
	for (uint i = 0; i < buffer_reset.size(); ++i)
	{
		if (buffer_reset[i])
			inbuffer->reset(i);
	}
}
*/
void Processing_element::aluUpdate()
{
	if (attribution->opcode != PEOpcode::null) {
		alu->compute(alu_out);
		if (outbuffer->isBufferNotFull(0)) {
			alu->update();      //alu流水更新以及输出
		}
	}
}


void Processing_element::controlBlock()
{
	loopControlParse();    //当控制信号单独作用时用该函数实现
	getAluInput();     //alu获取输入，包括作为操作数的控制输入
	gatherOperands();  //包括一般数据流操作 以及 控制信号作为操作数实现
}


void Processing_element::simStep2()
{
	//controlBlock();    //get the inbuffer_out for control check
	//aluUpdate();       //alu compute
	//toOutBuffer();
	//应该反过来操作
//	toOutBuffer();
	aluUpdate();
	toOutBuffer();//因为是ack机制，所以后面的要清掉,而且按照正常来说，这一周期出数，这一周期就应该收数,如果不收,下周期alu进不了数.而且aluupdate和tooutbuffer实际上是在一个寄存器周期里面做的
	controlBlock();//从inbuffer进alu
}

void Processing_element::aluPop()
{
	alu->pop();
}

void Processing_element::outReset()
{
	for (auto& i : output_port)///////////////////////////////加了这个之后输出就无效了，不能把out清空//
	i.reset();
}
void Processing_element::toOutBuffer()
{
	if (attribution->opcode != PEOpcode::null) {
		for (uint i = 0; i < system_parameter.bool_outport_breadth + system_parameter.data_outport_breadth; ++i)
		{
			if (attribution->output_from[i] == OutputFrom::outbuffer) {
				if (outbuffer->input(alu_out, i) && alu->getpipeline().size() != 0)
					aluPop();
			}
			else if (attribution->output_from[i] == OutputFrom::alu) { 
				if (next_bp[i] && alu->getpipeline().size() != 0) {
					output_port[i] = alu_out;//考虑到alu是nop的情况，只有在pipeline 非空的情况才会pop出去
					if(alu->getpipeline().size() != 0){ aluPop(); }
				}
			}
		}
	}
}

void Processing_element::outBufferSend()
{
	for (uint i = 0; i < system_parameter.data_outport_breadth + system_parameter.bool_outport_breadth; ++i)
	{
		if (next_bp[i])
			outbuffer->output(output_port[i], i);
	}
}

void Processing_element::simStep3()
{
	//toOutBuffer();
	outBufferSend();
}

void Processing_element::simBp()
{
	for (uint i = 0; i < thispe_bp.size(); i++)
		thispe_bp[i] = pebp->getBp(i);
	//给上一级outbuffer返回bp
}

//bool Processing_element::bufferCanBeEmpty(uint port)
//{
//
//	//if (inbuffer->isBufferNotFull(port)) {
//	////	Debug::getInstance()->getPortFile() << "here buffer is full";
//	//	return true;
//	//}
//	if (isBufferCanBeNotFull(port)) {
//		Debug::getInstance()->getPortFile() << "here buffer can be not full"<<std::endl;
//		return true;
//	}
//	return false;
//}

//bool Processing_element::isBufferCanBeNotFull(uint port)
//{
//	if (attribution->ob_from[0] == OutBufferFrom::inbuffer && port == 0)//当dummy时
//	{
//		if (outbuffer->isBufferNotFull(port))
//			return true;
//		else
//			return next_bp[0];
//	}
//	else if (attribution->alu_in_from[port] == AluInFrom::inbuffer)    //当去往alu时查询该通路是否有空                    
//	{
//		if (allAluOperandsGot())
//		{
//			if (alu->canReceiveInput())                               //alu not full
//				return true;
//			else if (attribution->ob_from[0] == OutBufferFrom::alu)
//			{
//				if (outbuffer->isBufferNotFull(0))   //ob not full
//					return true;
//				else if (next_bp[0])                      //next_pe not full
//					return true;
//			}
//			else if (attribution->output_from[0] == OutputFrom::alu)
//			{
//				return next_bp[0];
//			}
//		}
//	}
//	else if (port == 2 && attribution->alu_in_from[port] != AluInFrom::inbuffer) //control port
//	{
//		return (controlBufferCanBeReset());
//	}
//	return false;
//}

//判断是否集齐所有操作数
bool Processing_element::allAluOperandsGot()
{
		if (attribution->control_mode == ControlMode::loop_activate && attribution->input_bypass == InputBypass::inbuffer )
		{
//			if (first_loop)
//			{
			if (alu_num == 2) {
				DEBUG_ASSERT(attribution->buffer_mode[0] ==  BufferMode::keep&& attribution->inbuffer_from[0] == InBufferFrom::flr && local_reg[0]->reg_v);
			}
			if (attribution->inbuffer_from[2] != InBufferFrom::flr) {
				if (inbuffer->isBufferNotEmpty(1) && inbuffer->isBufferNotEmpty(2))
				{
					return true;
				}
				else
					return false;
			}
			else {
				if (inbuffer->isBufferNotEmpty(1))
				{
					return true;
				}
				else
					return false;
			}
			//else if (!first_loop)
			//{
			//	if (alu_num == 2) {
			//		if (inbuffer->isBufferNotEmpty(1) && inbuffer->isBufferNotEmpty(2))
			//			return true;
			//		else
			//			return false;
			//	}
			//	else if (alu_num == 1) {
			//		if (inbuffer->isBufferNotEmpty(1))
			//		{
			//			return true;
			//		}
			//		else
			//			return false;
			//	}
			//	else {
			//		DEBUG_ASSERT(false);
			//	}
			//}
		}
	//	else if (attribution->control_mode == ControlMode::transout && attribution->input_bypass == InputBypass::inbuffer)
	//	{
	//		if (!inbuffer->isBufferNotEmpty(2)) { return false; }
	//		if (!inbuffer->isBufferNotEmpty(1)) { return false; }
	////		if (!local_reg[1]->reg_v) { return false; }
	//	}
		else
		{			
				//			alu_mask[i] = true;
			uint clk = ClkDomain::getInstance()->getClk();
			if (attribution->input_bypass == InputBypass::inbuffer) {
				if (attribution->control_mode == ControlMode::bind||attribution->control_mode == ControlMode::cb || attribution->control_mode == ControlMode::cinvb || attribution->control_mode == ControlMode::break_pre || attribution->control_mode == ControlMode::break_post)
				{
					if (!inbuffer->isBufferNotEmpty(2))
						return false;
				}
				for (uint i = 0; i < alu_num; ++i) {
					int search_index= (alu_num == 1 ? 1 : i);
					if (attribution->input_bypass == InputBypass::inbuffer && (attribution->buffer_mode[i] == BufferMode::buffer || (attribution->inbuffer_from[1] == InBufferFrom::aluin1)))
					{
						if (!inbuffer->isBufferNotEmpty(search_index))
							return false;
					}
					if (attribution->input_bypass == InputBypass::inbuffer && (attribution->buffer_mode[i] == BufferMode::keep || attribution->buffer_mode[i] == BufferMode::lr|| attribution->buffer_mode[i] == BufferMode::lr_out)&&(attribution->inbuffer_from[1]!=InBufferFrom::aluin1))
					{
						if (!local_reg[search_index]->reg_v)
							return false;
					}
				}

			}

			else {
				if (attribution->control_mode == ControlMode::bind||attribution->control_mode == ControlMode::cb || attribution->control_mode == ControlMode::cinvb || attribution->control_mode == ControlMode::break_pre || attribution->control_mode == ControlMode::break_post)
				{
					if (!input_port[2].valid)
						return false;
				}
				for (uint i = 0; i < alu_num; ++i) {
					int search_index = (alu_num == 1 ? 1 : i);
					if (!input_port[search_index].valid)
						return false;
				}
			}//bypass情况不做getoprands判断
		}
		
	return true;
}

//bool Processing_element::controlBufferCanBeReset()
//{
//	if (attribution->control_mode == ControlMode::loop_activate || attribution->control_mode == ControlMode::loop_reset)
//	{
//		if (local_reg->reg_v && inbuffer->isBufferNotEmpty(2))       //操作数集齐
//		{
//			if (attribution->ob_from[0] == OutBufferFrom::lr)
//			{
//				if (outbuffer->isBufferNotFull(0))   //ob not full
//					return true;
//				else if (next_bp[0])                      //next_pe not full
//					return true;
//			}
//			else if (attribution->alu_in_from[2] == AluInFrom::lr)     //lr->alu
//			{
//				if (attribution->ob_from[0] == OutBufferFrom::alu)     //alu->ob
//				{
//					if (outbuffer->isBufferNotFull(0))   //ob not full
//						return true;
//					else if (next_bp[0])                      //next_pe not full
//						return true;
//				}
//				else if (attribution->output_from[0] == OutputFrom::alu)
//				{
//					if (next_bp[0])                      //next_pe not full
//						return true;
//				}
//			}
//		}
//	}
//	return false;                //其他路径都需要操作数匹配
//}

void Processing_element::wireReset()
{
	//vector<Bool> thispe_bp;
	//vector<Bool> next_bp;
	for (auto& i : input_port)
		i.reset();
//	for (auto& i : output_port)///////////////////////////////加了这个之后输出就无效了，不能把out清空//
//		i.reset();
	for (auto& i : inbuffer_out)
		i.reset();
	for (auto& i : aluin)
		i.reset();
	for(auto& i:reg_out)
		i.reset();
	alu_out.reset();
	rs_cd = false;
	control_value.reset();
	oprand_collected = false;
}
void Processing_element::print_lr(std::ofstream& file) {
	file << "----LocalReg Element----" << std::endl;
	file << std::setw(20) << "valid control data bool" << std::endl;
	for (uint i = 0; i < local_reg.size(); i++)
	{
		file << "local_reg" << std::setw(2) << local_reg[i]->reg_v<< std::setw(4)<< local_reg[i]->value;
		file << "      ";
	}
	file << std::endl;
}
void Processing_element::bufferprint() {
	if (ClkDomain::getInstance()->getClk() >= Debug::getInstance()->print_file_begin && ClkDomain::getInstance()->getClk() < Debug::getInstance()->print_file_end) {
		if (Debug::getInstance()->debug_level >= 1) {
			Debug::getInstance()->getRegFile() << "--------PE " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
		//	for(auto &lr:local_reg)
			print_lr(Debug::getInstance()->getRegFile());
			inbuffer->print(Debug::getInstance()->getRegFile());
//			Debug::getInstance()->getRegFile() << "PE" << attribution->index<<" ";
			alu->print();
			outbuffer->print(Debug::getInstance()->getRegFile());
		}
//		Debug::getInstance()->getPortFile() << "--------PE " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
//		wirePrint(Debug::getInstance()->getPortFile());
	}
}
void Processing_element::print()
{
	if (ClkDomain::getInstance()->getClk() >= Debug::getInstance()->print_file_begin && ClkDomain::getInstance()->getClk() < Debug::getInstance()->print_file_end) {
		Debug::getInstance()->getPortFile() << "--------PE " << attribution->index << " clk " << ClkDomain::getInstance()->getClk() << "--------" << std::endl;
		wirePrint(Debug::getInstance()->getPortFile());
	}
}

void Processing_element::wirePrint(std::ofstream& file)
{
	//使用Debug的单例类进行调试，因此删掉了原打印方法
	const auto& debugPrint = Debug::getInstance();
	if (debugPrint->debug_level == 2) {
		file << std::setw(20) << "valid control data bool" << std::endl;
		debugPrint->linePrint("   this is step3");
		Debug::getInstance()->getPortFile() << "pe" << attribution->index << " ";
		debugPrint->vecPrint<Port_inout>(output_port, "output");
		debugPrint->linePrint("   this is step2");
	//	alu->print();
		debugPrint->onePrint<Port_inout>(control_value , "control");
		debugPrint->vecPrint<Port_inout>(reg_out, "regout");
		Debug::getInstance()->getPortFile() << "          pipeline size" << alu->getpipeline().size() << std::endl;
		Debug::getInstance()->getPortFile() << "          oprand_collected" << oprand_collected << std::endl;
		debugPrint->vecPrint<Port_inout>(aluin, "aluin");
		debugPrint->onePrint<Port_inout>(alu_out, "alu_out");
		Debug::getInstance()->getPortFile() << "          lastflag" << lastflag << std::endl;
		debugPrint->linePrint("   this is step1");
		debugPrint->vecPrint<Port_inout>(input_port, "input_port");
		debugPrint->vecPrint<Bool>(next_bp, "next_bp");
		debugPrint->vecPrint<Bool>(thispe_bp, "thispe_bp");
	}
	else if (debugPrint->debug_level <= 1) {
		file << std::setw(20) << "valid control data bool" << std::endl;
	//	debugPrint->onePrint<Port_inout>(output_port[0], "output0");
		debugPrint->vecPrint<Port_inout>(input_port, "input_port");
		debugPrint->onePrint<Port_inout>(output_port[0], "output0");
		debugPrint->vecPrint<Bool>(next_bp, "next_bp");
		debugPrint->vecPrint<Bool>(thispe_bp, "thispe_bp");
	}
	else { ; }
}