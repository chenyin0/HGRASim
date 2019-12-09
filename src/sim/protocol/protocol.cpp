#include "protocol.h"
#include "../Node/PE.h"
#include "../Node/LSE.h"

using namespace Simulator::Array;

bool Creditbp_buffer::getBp(uint port)
{
	if (buffer->isBufferNotFull(port))
		return true;
	else
		return false;
}

bool Ackbp_buffer::getBp(uint port)
{
	//if 入数成功
	//return true
	return (buffer->isInputSuccess(port));
}

bool Creditbp_reg::getBp()
{
	if (!reg->valid)
		return true;
	else
		return false;
}

bool Ackbp_reg::getBp()
{
	//return reg->isInputSuccess();
	DEBUG_ASSERT(true);
	return false;
}

bool Creditbp_lr::getBp()
{
	if (!lr->reg_v)
		return true;
	else
		return false;
}

bool Ackbp_lr::getBp()
{
	//return lr->isInputSuccess();
	DEBUG_ASSERT(true);
	return false;
}

bool Pebp::getBp(uint port)
{
	uint clk = ClkDomain::getInstance()->getClk();
	if (port == 0) {
		if (pe->attribution->control_mode == ControlMode::loop_activate|| pe->attribution->control_mode == ControlMode::calc_activate) {
			//if (pe->attribution->control_mode == ControlMode::calc_activate) { return true; }
			//else if (pe->inbuffer->isBufferNotFull(1)) { return true; }
			//else { return false; }
			return true;
		}
		else if (pe->attribution->input_bypass==InputBypass::inbuffer) {/////注意这里是alu来源于inbuffer的情况，即input需要进inbuffer
//			if (pe->attribution->ob_from[0] == OutBufferFrom::alu) {
			if (pe->attribution->buffer_mode[0] == BufferMode::buffer || pe->attribution->buffer_mode[0] == BufferMode::lr_out) {
				if (Preprocess::Para::getInstance()->getArrayPara().stall_mode == stallType::none) {
					if (pe->inbuffer->isBufferNotFull(port)) { return true; }
					else { return false; }
				}
				else if (Preprocess::Para::getInstance()->getArrayPara().stall_mode == stallType::inbuffer_stall) {
					if (pe->inbuffer->isBufferNotFull(port)) {
						return true;
					}
					else if (pe->alu->depth != 0 && pe->alu->canReceiveInput() && pe->allAluOperandsGot()) { return true; }
					else if (pe->alu->depth == 0 && pe->outbuffer->isBufferNotFull(0)) { return true; }
					else if (pe->alu->depth != 0 && pe->allAluOperandsGot() && pe->outbuffer->isBufferNotFull(0) && !pe->alu->canReceiveInput() && pe->attribution->opcode == PEOpcode::mul) {
						return true;
					}
					else { return false; }
				}
				
			}
			else if (pe->attribution->buffer_mode[0] == BufferMode::keep) {
				return !(pe->local_reg[0]->reg_v);
			}
//			}
		}
		else if (pe->attribution->input_bypass == InputBypass::bypass) {
			if(pe->alu->depth!=0)
				return(pe->alu->canReceiveInput());
			else {
				if (pe->attribution->output_from[0] == OutputFrom::outbuffer)
					return(pe->outbuffer->isBufferNotFull(0));
				else if(pe->attribution->output_from[0] == OutputFrom::alu)
					return(pe->next_bp[0]);
			}
		}
//		else if(pe->inbuffer->isBufferNotFull(port)) { return true; }
		else { return false; } 
	 //这里写alu来源于inbuffer，ob来源于alu,out来源于ob
		//else {
		//	Debug::getInstance()->getPortFile() << "here buffer is full";
		//}
		//if (pe->bufferCanBeEmpty(port))
		//	return true;
	}
	else if (port == 1) {
		 if (pe->attribution->input_bypass == InputBypass::inbuffer) {
			if (pe->attribution->inbuffer_from[1] == InBufferFrom::aluin1) {
				return !(pe->local_reg[1]->reg_v);
			}
			else if (pe->attribution->control_mode == ControlMode::loop_activate || pe->attribution->control_mode == ControlMode::calc_activate) {
				return !(pe->local_reg[1]->reg_v);
			}
			else if (pe->attribution->buffer_mode[1] == BufferMode::lr) {
				//		Debug::getInstance()->getPortFile() << ""
				return !(pe->local_reg[1]->reg_v);//总是要给lr进值，只要有效就不能进
			}
			else if (pe->attribution->buffer_mode[1] == BufferMode::buffer || pe->attribution->buffer_mode[port] == BufferMode::lr_out) {/////注意这里是alu来源于inbuffer的情况，即input需要进inbuffer
				if (Preprocess::Para::getInstance()->getArrayPara().stall_mode == stallType::none) {
					if (pe->inbuffer->isBufferNotFull(port)) { return true; }
					else { return false; }
				}
				else if (Preprocess::Para::getInstance()->getArrayPara().stall_mode == stallType::inbuffer_stall) {
					if (pe->inbuffer->isBufferNotFull(port)) {
						return true;
					}
					else if (pe->alu->depth != 0 && pe->alu->canReceiveInput() && pe->allAluOperandsGot()) { return true; }
					else if (pe->alu->depth == 0 && pe->outbuffer->isBufferNotFull(0)) { return true; }
					else if (pe->alu->depth != 0 && pe->allAluOperandsGot() && pe->outbuffer->isBufferNotFull(0) && !pe->alu->canReceiveInput() && pe->attribution->opcode == PEOpcode::mul) {
						return true;
					}
					else { return false; }
				}
			}
		}
		else if (pe->attribution->input_bypass == InputBypass::bypass) {
			 if (pe->alu->depth != 0)
				 return(pe->alu->canReceiveInput());
			 else {
				 if (pe->attribution->output_from[0] == OutputFrom::outbuffer)
					 return(pe->outbuffer->isBufferNotFull(0));
				 else if (pe->attribution->output_from[0] == OutputFrom::alu)
					 return(pe->next_bp[0]);
			 }
		}
		else { return false; }//还需要写alu直接来自inport的情况，此时bp由alu决定
	}
	else if (port == 2) {
		if (pe->attribution->input_bypass == InputBypass::inbuffer) {
			if (pe->attribution->buffer_mode[2] == BufferMode::keep) {
				//		Debug::getInstance()->getPortFile() << ""
				return !(pe->local_reg[2]->reg_v);//总是要给lr进值，只要有效就不能进
			}
			else if (pe->attribution->buffer_mode[port] == BufferMode::buffer|| pe->attribution->buffer_mode[port] == BufferMode::lr_out) {/////注意这里是alu来源于inbuffer的情况，即input需要进inbuffer
				if (Preprocess::Para::getInstance()->getArrayPara().stall_mode == stallType::none) {
					if (pe->inbuffer->isBufferNotFull(port)) { return true; }
					else { return false; }
				}
				else if (Preprocess::Para::getInstance()->getArrayPara().stall_mode == stallType::inbuffer_stall) {
					if (pe->inbuffer->isBufferNotFull(port)) {
						return true;
					}
					else if (pe->alu->depth != 0 && pe->alu->canReceiveInput() && pe->allAluOperandsGot()) { return true; }
					else if (pe->alu->depth == 0 && pe->outbuffer->isBufferNotFull(0)) { return true; }
					else if (pe->alu->depth != 0 && pe->allAluOperandsGot() && pe->outbuffer->isBufferNotFull(0) && !pe->alu->canReceiveInput() && pe->attribution->opcode == PEOpcode::mul) {
						return true;
					}
					else { return false; }
				}
			}
		}
		else if (pe->attribution->input_bypass == InputBypass::bypass) {
			if (pe->alu->depth != 0)
				return(pe->alu->canReceiveInput());
			else {
				if (pe->attribution->output_from[0] == OutputFrom::outbuffer)
					return(pe->outbuffer->isBufferNotFull(0));
				else if (pe->attribution->output_from[0] == OutputFrom::alu)
					return(pe->next_bp[0]);
			}
		}
		else { return false; } 
	}
}

bool Lsebp::getBpTag(uint port,uint tag)
{
	if (lse->getAttr()->ls_mode == LSMode::load) {
		if (lse->inbuffer->isBufferNotFull(port))
			return true;
	}
	else {
		if (lse->getOutbuffer()->canReceiveInput(port,tag))
			return true;
	}
//	if (lse->bufferCanBeEmpty(port))
//		return true;
	return false;
}

bool Lsebp::getBp(uint port)
{
	if (lse->getAttr()->ls_mode == LSMode::load) {
		if (lse->inbuffer->isBufferNotFull(port))
			return true;
	}
	else {
		if (lse->getOutbuffer()->isBufferNotFull(port))
			return true;
	}
	//	if (lse->bufferCanBeEmpty(port))
	//		return true;
	return false;
}
