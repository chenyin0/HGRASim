#pragma once

#include "../../define/define.hpp"
#include "../inout.h"
#include "../../preprocess/preprocess.h"



namespace Simulator::Array
{
	class Module
	{
	public:
		//virtual void print() = 0;
		virtual ~Module() = default;
	};

	class Alu:public Module
	{
	public:
		virtual ~Alu() = default;
	};

	struct Alutrans
	{
		PEOpcode opcode;
		int data1;
		int data2;
		int data3;
		uint cycle;
		uint delay;
		bool rs_cd;
		bool valid;
	};

	

	//需要支持每周期根据指令而变化的扩展
	class Pealu:public Alu
	{
	private:
		Alutrans newtrans;
		vector<Alutrans> pipeline;
		uint depth;
		//PEOpcode opcode;
		//uint delay;
		//uint mask;

	public:
		friend class Processing_element;
		Pealu(PEOpcode opcode);
		Pealu();
		~Pealu() = default;
		void update();
		void print();//想看pipeline
		vector<Alutrans> getpipeline() { return pipeline; };
		bool canReceiveInput();
		bool input(vector<Port_inout> alu_in, PEOpcode opin,bool nolast,bool rs_cd);
		static uint getDelay(PEOpcode opin);
		bool output(Port_inout& out); 
		//void getOperation(uint op);           //future interface for instuction style
		void compute(Port_inout& out);
		bool empty() const;
		void pop();
	};


	class Instructionbuffer
	{
	public:
		void update(){};
	};

	class Localreg:public Module
	{
	private:
		//int value;
		//bool reg_v;
	public:
		Localreg() = default;
		bool reg_v;
		int value;
		void input(Port_inout input);
		void input_transout(Port_inout input);
		void output(Port_inout& output);
		void setvalue(int value);
		void reset();
		void print(std::ofstream& file) const;
	};
	
}

