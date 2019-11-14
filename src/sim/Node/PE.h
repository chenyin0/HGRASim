#pragma once
#include "Node.h"
#include "../debug.h"
#include "../ClkDomain.h"

namespace Simulator::Array
{
	//��Ҫд��ÿ���ڿɱ��
	//����PE�ڲ�ģ��֮�����������
	//inbuffer->alu��Ȼ�ȳ�������
	class Processing_element : public Node
	{
	private:
		friend class Pebp;
		uint index;                  //logic index
		uint alu_num;
		bool alu_flag;
		uint ctrl_index;
		uint in_num;
		bool rs_cd;
		vector<Bool> break_state;
		const Preprocess::DFGNode<NodeType::pe>* attribution;

		Buffer* inbuffer;
		Buffer* outbuffer;
		Pebp* pebp;
		//Bufferbp* out_bufferbp;
		Pealu* alu;
//		Localreg* local_reg;
		vector<Localreg*> local_reg;
		Instructionbuffer* instruction_buffer;
		bool first_loop=true;

		//alu�Ĳ�����mask
		vector<Bool> alu_mask;
		vector<Bool> ib2alu_mask;

		//ģ��֮��������źţ�ֵȡ���ڼĴ�����������Ҫÿ����reset
		vector<Bool> next_bp;
//		vector<Port_inout> input_port;
		vector<Port_inout> inbuffer_out;
		Port_inout reg1_out;
		Port_inout reg0_out;
		Port_inout reg2_out;
		vector<Port_inout> reg_out;
		Port_inout control_value;
		Port_inout alu_out;
		vector<Port_inout> aluin;
		bool oprand_collected;
		bool lastflag;
//		void simStep1(); //input2inbuffer     break���λ��step1

//		void simStep2(); //inbuffer2alu2outbuffer    breakstate����λ��step2
		//{
		/**/void controlBlock();
		/*--*/void loopControlParse();
		/*--*/void getAluInput();
		/*--*/void gatherOperands();
		/*----*/void lastCheck();
		/*----*/void breakoccur();
		/**/void aluUpdate();
		/**/void toOutBuffer();
		void onePrint(std::ofstream& file, const Port_inout& wire, string name);
		//}

//		void simStep3(); //outbuffer2output
		/*--*/void aluPop();
		/**/void outBufferSend();
		/**///void output2Next();

		void simInstruction(){};//future interface for change instruction
//		void wireReset();

//		void simBp();
		/**/bool bufferCanBeEmpty(uint port);   //for credit
		/*--*/bool isBufferCanBeNotFull(uint port);
		/*----*/bool allAluOperandsGot();
		/*----*/bool controlBufferCanBeReset();       //preprocess for control buffer reset

//		void print();
		void wirePrint(std::ofstream& file);
	public:
		friend class Pebp;
	//	friend class Pro;
		bool stall_one;
		Processing_element(const Preprocess::ArrayPara para, uint index);
		~Processing_element();
		void update();
		void simStep3();
		void bufferprint();
		void simStep2();
		void simStep1(uint i);
		void simStep1();
		void print();
		vector<Port_inout> input_port;
		void simBp();
		void wireReset();
		void parameterSet();
		//using returnBp= bool(Processing_element::*)();
		//enum class threeBp{inbuffer,alu,outbuffer};
		//bool everybp(threeBp);
		//static returnBp Menu[];
		bool isSpecial() {
			return attribution->control_mode == ControlMode::cb||attribution->inbuffer_from[1]==InBufferFrom::aluin1
				|| attribution->control_mode == ControlMode::cinvb || attribution->control_mode == ControlMode::break_pre
				|| attribution->control_mode == ControlMode::break_post || attribution->control_mode == ControlMode::loop_activate
				|| std::find(attribution->buffer_mode.begin(), attribution->buffer_mode.end(), BufferMode::lr_out) != attribution->buffer_mode.end()
				|| attribution->control_mode == ControlMode::trans|| attribution->control_mode == ControlMode::continue_
				;
		}
		bool except;
		void print_lr(std::ofstream& file);
		void outReset();
		//�����������ڽڵ����ã����������ڽڵ��ڲ�update��Ϊ����������һ���stepc
		//port����������port��Ϊһ�֣�data��ǰ��bool�ں�
		void getInput(uint port, Port_inout input) override;  //port֮����Դ��ͬ������ϣ���inbuffer��д���Ա������
		void getBp(uint port, bool input) override;
//		auto isSpecial();
		Bufferpe_in* getInbuffer() { return static_cast<Simulator::Array::Bufferpe_in*>(inbuffer); }
		Bufferpe_out* getOutbuffer() { return static_cast<Simulator::Array::Bufferpe_out*>(outbuffer); }
//		Buffer* getInbuffer() { return inbuffer; };
		const Preprocess::DFGNode<NodeType::pe>* getAttr(){ return attribution; };
		vector<Port_inout> output_port;
		vector<Bool> thispe_bp;

	};
}
