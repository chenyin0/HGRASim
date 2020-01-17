#pragma once
#include "Node.h"
#include "../debug.h"
#include "../ClkDomain.h"
#include "SPM.h"
namespace Simulator::Array
{
	//INSPECTION  break需要的循环结束以及全局结束的冲突
	class LoopControl: public Node
	{
	private:
		uint index;
		uint end_innum;
		vector<Bufferd> reg;                                  //0-iterator 1-initial 2-loopbound 3-step
		vector<Bufferd> end_reg; 
		vector<Bufferd> relay_reg;
		vector<Protocol*> reg_protocol;
		vector<Protocol*> endreg_protocol;
		Buffer* muxout_buffer;
		Spm* spm;

		bool first_loop;
		bool spmBp;
		int df_cnt;
		uint activate_cnt;
		uint end_cnt;
		int break_cnt;
		bool last_flag = false;
	    //wire signal
		//the order is reg_input first, end_input second
		Port_inout loopbegin;
		Port_inout muxout;
		Port_inout stepout;
		Port_inout bufferout;
		Port_inout combout;
		vector<Port_inout> relay_input;
	    //bp
		vector<Bool> nextbp;
		vector<Bool> end_flag;
//		void simStep1();                                       //reg get input
		void regInput(Port_inout input, uint port);
		void endInput(Port_inout input, uint port);

	public:
		LoopControl(const Preprocess::ArrayPara para, uint index);
		~LoopControl();
		bool stall_one;
		vector<Port_inout> reg_input;
		vector<Port_inout> end_input;
		void update();
		void attachSpm(Spm* spm_);
		void buffer_print();
		void activate();
		void getInput(uint port, Port_inout input) override;  //port之间来源不同，解耦合；但inbuffer的写入仍保持耦合
		void getBp(uint port, bool input) override;
		void simStep2();                                       //lc buffer get input
		void simStep3();                                       //lc buffer give output
		void simStep1(uint i);
		void simStep1();
		void simBp();
		void simall();
		void wireReset();
		void print();
		void wirePrint();
		void outReset();
		const Preprocess::DFGNode<NodeType::lc>* getAttr() { return attribution; };
		void onePrint(std::ofstream& file, const Port_inout& wire, string name);
		vector<Port_inout> lc_output;                        //0-i 1-j 2-loopsignal 3-endsignal
		const Preprocess::DFGNode<NodeType::lc>* attribution;     //config
		vector<Bool> thisbp_end;      //the order is reg_input first, end_input second
		vector<Bool> thisbp_reg;
	};
}