#pragma once
#include "Cluster.h"
#include "../mem_system/Lsu.h"
#include "../memory/MemoryData.h"
#include "../ClkDomain.h"

namespace Simulator::Array
{
	//LSE与LSU之间互相传输都使用ack机制
	class Loadstore_element:public Node
	{
	private:
		friend class Match_set;
		const Preprocess::DFGNode<NodeType::ls>* attribution;
		Buffer* inbuffer;
		Buffer* outbuffer;
		bool have_recv;
		Lsebp* inbuffer_bp;
		Lsebp* outbuffer_bp;
		vector<Bool> break_state;
		vector<Port_inout_lsu> fake_lsu;
		DRAMSim::Lsu* lsu;
		ClusterGroup& cluster_group;
		Loadstore_element* couple_se;
		uint tag_counter;
		uint sended_tag;
		bool lsu_noin;
		bool wait_for_data;
		uint index;
		uint match_tag;
		uint first_seektag;
		bool addr_v = true;
		Port_inout outport;
		Port_inout lastout;
		Port_inout outport2;
		//wire signal
		vector<Port_inout> input_port_pe;
		vector<Port_inout_lsu> input_port_lsu;
		vector<uint> validDataIndex;
		vector<uint> validAddrIndex;
		vector<Bool> maintain_order;
		vector<vector<uint>> vecs;
		vector<Bool> nextpe_bp;
		Port_inout inbuffer_out;
		Bool nextlsu_bp=true;
		Bool firstlsu = true;
		bool ack;
		Simulator::Array::Spm spm;
		void leSimStep1(uint i);
		void leSimStep1();
		void seSimStep1(uint i);
		void seSimStep1();

		void leSimStep2();
		void sedSimStep2();
		void leSimStep2NoMem();
		void readNoMemory();
		void sedSimStep2NoMem();
		void tag_counter_update();


		//function to call couple se
		bool isCoupleSeNotEmpty();
		void CoupleSeOutput(Port_inout& out, uint port);
		void coupleSeReset(uint tag);
		//function for achieve coupled se
		bool isOutBufferNotEmpty();
		void CoupledSeOutput(Port_inout& out, uint port);
		void resetByCouple(uint tag);

	public:
		Loadstore_element(const Preprocess::ArrayPara para, uint index, DRAMSim::Lsu* lsu, ClusterGroup& cluster_group);
		friend class Lsebp;
		friend class Lsu;
		~Loadstore_element();
		void simStep2();
		bool Ack2spm();
		void spm2lse_temp(Port_inout_lsu data);
		void simStep1();
		void simStep1(uint i);
		void simBp();
		bool existsInGroup();
		void LSEcallback(Port_inout_lsu inp);
		void print();
		void value_update(uint &update_tag,const uint &last_tag);
		void printBuffer();
		void lseReset();
		void wireReset();
		void update();     //需要包括lse对t/f; end信号的处理
		void getInput(uint port, Port_inout input) override;  //port之间来源不同，解耦合；但inbuffer的写入仍保持耦合
		void getBp(uint port, bool input) override;
		void seAttach(Loadstore_element* couple);
		void wirePrint();
		void callbackACK();
		void outReset();
		void attachLsu(Lsu* lsu_);
		void LSEcallback(uint addr, uint64_t cycle, short tag);
		const Preprocess::DFGNode<NodeType::ls>*getAttr() {return attribution;}
		//void print() override;
		Bufferlse_in *getInbuffer() { return static_cast<Simulator::Array::Bufferlse_in*>(inbuffer); }
		Bufferlse_out *getOutbuffer() { return static_cast<Simulator::Array::Bufferlse_out*>(outbuffer); }
//		Bufferlse_in *getInbuffer() { return inbuffer; }
		Port_inout_lsu output_port_2lsu;    //second
		Port_inout_lsu getData();
		Port_inout output_port_2array;      //first
		//Port_inout output_port_2lc;         //last
		vector<Bool> this_bp;                   //0-pe_addr 1-lsu/pe_data
	};

	//负责执行全局LE的tag匹配
	class Match_set
	{
	private:
		Preprocess::ArrayPara system_parameter;
		vector<Loadstore_element*> lse_set;
	public:
		Match_set(const Preprocess::ArrayPara para, map<uint, Loadstore_element*> ls_map);
		~Match_set() = default;
		void update();
	};
}
