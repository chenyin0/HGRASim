#pragma once
#include "../define/define.hpp"

namespace Simulator::Array
{
	class Port_inout
	{
	public:
		bool value_bool;
		int value_data;
		bool valid;
		bool condition;
		bool last;
		//uint tag;
		Port_inout() { value_bool = false; value_data = 0; valid = false; condition = true; last = false; }//初始情况下condition为true
		Port_inout(bool valueb, int valued, bool valid_, bool cond, bool last_)
		: value_bool(valueb), value_data(valued), valid(valid_), condition(cond), last(last_){}
		//Port_inout& operator=(Port_inout& rhs) {
		//	this->value_bool = rhs.value_bool;
		//	this->value_data = rhs.value_data;
		//	this->valid = rhs.valid;
		//	this->condition = rhs.value_bool;
		//	this->last = rhs.last;
		//	return *this;
		//}
		void reset() { value_bool = false; value_data = 0; valid = false; condition = true; last = false; };
	};

	class Port_inout_lsu
	{
	public:
		int value_data;
		uint value_addr;
		bool valid;
		short tag;
		bool rdwr;
		bool dae;

		//bool isNewContext;  // identify it is a new context when switch the context
		uint lseId_virtual;  // LSE 实际的硬件ID，从配置文件里读出

		MemAccessMode _memAccessMode;  // add memory access mode
		DaeMode _daeMode;

		bool lastData; // 表示最后一次循环的数据，用来指导SPM里的schedule切换配置
		// bind with each load request, send the data back to SPM according to these two Id
		uint bankId;
		uint rowId;
		bool inflight; // indicate this addr is send to memory, but hasn't been sent back;

		bool cond; // for branch
		bool dataReady; // indicate the load request has been sent back, data load is ready

		bool occupy;  // used in branch, to occupy

		bool bypassCache;  // set to 1, signify bypass cache and send to DRAM directly

		uint contextId;  // 不用配置，用作SPM callbackAck4Lse

		//uint tag;

		Port_inout_lsu() 
		{
			value_addr = 0; 
			value_data = 0; 
			valid = false; 
			tag = 0;
			rdwr = false; 
			dae = false;

			_memAccessMode = MemAccessMode::temp;
			_daeMode = DaeMode::none; 
			/*isNewContext = 0;*/  
			lseId_virtual = 0;
			lastData = 0;
			
			bankId = 0;  
			rowId = 0;

			inflight = 0;

			cond = 0;
			dataReady = 0;

			occupy = 0;

			bypassCache = 0;

			contextId = 0;
		}

		void reset() 
		{ 
			value_addr = 0; 
			value_data = 0;
			valid = false; 
			tag = 0; 
			rdwr = false;
			dae = false; 
			_memAccessMode = MemAccessMode::temp;
			_daeMode = DaeMode::none; 
			/*isNewContext = 0;*/
			lastData = 0;
			bypassCache = 0;
		}
	};

	/*
	class Port_bp
	{
	public:
		bool value;
		void reset() { value = false; }
	};
	*/

	/*
	class Data32_notag: public Port_inout
	{
	public:
		uint value;
		bool valid;

		Data32_notag() = default;
		~Data32_notag() = default;
	};

	class Databool_notag: public Port_inout
	{
	public:
		bool value;
		bool valid;

		Databool_notag() = default;
		~Databool_notag() = default;
	};

	class Port_factory
	{
	public:
		virtual Port_inout create_port() = 0;
		virtual ~Port_factory() = default;
	};

	class Data32_notag_factory: public Port_factory
	{
	public:
		virtual Port_inout create_port()
		{
			return new Data32_notag();
		}
	};

	class Databool_notag_factory : public Port_factory
	{
	public:
		virtual Port_inout * create_port()
		{
			return new Databool_notag();
		}
	};
	*/
}

