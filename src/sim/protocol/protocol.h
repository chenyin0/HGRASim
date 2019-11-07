#pragma once
#include "../../define/define.hpp"
#include "../Buffer/buffer.h"
//#include "../Node/LSE.h"
//#include "../Node/PE.h"


namespace Simulator::Array
{
	class Processing_element;
	class Loadstore_element;

	//描述simulator中的协议
	class Protocol
	{
	public:
		virtual bool getBp() = 0;
		virtual bool getBp(uint port) = 0;
//		virtual bool getBp(uint port,uint tag) = 0;
		virtual ~Protocol() = default;
	};

	//由buffer空或buffer入数而产生的bp行为
	class Bufferbp : public Protocol
	{
	public:
		virtual bool getBp(uint port) override { DEBUG_ASSERT(false); return false; }
		virtual ~Bufferbp() = default;
	};

	class Creditbp_buffer : public Bufferbp
	{
		Buffer* buffer;
	public:
		Creditbp_buffer(Buffer* buffer) { this->buffer = buffer; }
		bool getBp(uint port) override;
		bool getBp() override{ DEBUG_ASSERT(false); return false; }
	};

	class Ackbp_buffer: public Bufferbp
	{
	private:
		Buffer* buffer;
	public:
		Ackbp_buffer(Buffer* buffer) { this->buffer = buffer; }
		bool getBp(uint port) override;
		bool getBp() override { DEBUG_ASSERT(false); return false; }
	};

	class Creditbp_reg : public Protocol
	{
	private:
		Bufferd *reg;
	public:
		Creditbp_reg(Bufferd *reg) { this->reg = reg; }//必须用指针，否则只是复制了初始值
		bool getBp() override;
		bool getBp(uint port) override { DEBUG_ASSERT(false); return false; }
	};

	class Ackbp_reg : public Protocol
	{
	private:
		Bufferd *reg;
	public:
		Ackbp_reg(Bufferd *reg) { this->reg = reg; }
		bool getBp() override;
		bool getBp(uint port) override { DEBUG_ASSERT(false); return false; }
	};

	class Creditbp_lr : public Protocol
	{
	private:
		Localreg * lr;
	public:
		Creditbp_lr(Localreg* lr) { this->lr = lr; }
		bool getBp() override;
		bool getBp(uint port) override { DEBUG_ASSERT(false); return false; }
	};

	class Ackbp_lr : public Protocol
	{
	private:
		Localreg * lr;
	public:
		Ackbp_lr(Localreg* lr) { this->lr = lr; }
		bool getBp() override;
		bool getBp(uint port) override { DEBUG_ASSERT(false); return false; }
	};

	class Pebp:public Protocol
	{
	private:
		Processing_element * pe;
	public:
		Pebp(Processing_element* pe) { this->pe = pe; }
		bool getBp(uint port) override;
		bool getBp() override
		{
			DEBUG_ASSERT(false);
			return true;
		};
	};
	
	class Lsebp :public Protocol
	{
	private:
		Loadstore_element* lse;
	public:
		//friend class Loadstore_element;
		Lsebp(Loadstore_element* lse) { this->lse = lse; }
		bool getBp(uint port) override;
		bool getBpTag(uint port, uint tag);
		bool getBp() override
		{
			DEBUG_ASSERT(false);
			return true;
		};
	};
}


