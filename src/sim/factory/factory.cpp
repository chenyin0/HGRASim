#include "factory.h"


using namespace Simulator::Array;
Buffer * Buffer_factory::createInBuffer(const Simulator::Preprocess::ArrayPara para)
{   
	return new Bufferpe_in(para);
}

Buffer * Buffer_factory::createOutBuffer(const Simulator::Preprocess::ArrayPara para)
{
	return new Bufferpe_out(para);
}

Buffer* Buffer_factory::createLcBuffer(Simulator::Preprocess::ArrayPara para)
{
	return new Buffer_lc(para);
}

Buffer* Buffer_factory::createLseInBuffer(Simulator::Preprocess::ArrayPara para)
{
	return new Bufferlse_in(para);
}

Buffer* Buffer_factory::createLseBuffer(Simulator::Preprocess::ArrayPara para, Simulator::BufferSize size,Simulator::LSMode lsmode)
{
	return new Bufferlse_out(para,size,lsmode);
}


Bufferbp* Bp_factory::createBufferBp(Simulator::Preprocess::ArrayPara para, Buffer* buffer)
{
	if (para.buffer_bp == BpType::credit)
		return new Creditbp_buffer(buffer);
	else if (para.buffer_bp == BpType::ack)
		return new Ackbp_buffer(buffer);
	else
	{
		DEBUG_ASSERT(false);
		return nullptr;
	}
}

Protocol* Bp_factory::createRegBp(Simulator::Preprocess::ArrayPara para, Bufferd *reg)//////必须用指针指向reg
{
	if (para.buffer_bp == BpType::credit)
		return new Creditbp_reg(reg);
	else if (para.buffer_bp == BpType::ack)
		return new Ackbp_reg(reg);
	else
	{
		DEBUG_ASSERT(false);
		return nullptr;
	}
}

Protocol* Bp_factory::createLrBp(Simulator::Preprocess::ArrayPara para, Localreg* lr)
{
	if (para.buffer_bp == BpType::credit)
		return new Creditbp_lr(lr);
	else if (para.buffer_bp == BpType::ack)
		return new Ackbp_lr(lr);
	else
	{
		DEBUG_ASSERT(false);
		return nullptr;
	}
}

Pebp* Bp_factory::createPeBp(Simulator::Preprocess::ArrayPara para, Processing_element* pe)
{
	return new Pebp(pe);
}

Lsebp* Bp_factory::createLseBp(Simulator::Preprocess::ArrayPara para, Loadstore_element* lse)
{
	return new Lsebp(lse);
}


