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
		//uint tag;
		Port_inout_lsu() { value_addr = 0; value_data = 0; valid = false; tag = 0; rdwr = false; dae = false; }
		void reset() { value_addr = 0; value_data = 0; valid = false; tag = 0; rdwr = false; dae = false; };
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

