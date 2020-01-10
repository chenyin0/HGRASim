#pragma once
#include "iostream"                        
#include "stdlib.h"                       
#include "math.h"                         //pay attention to ACK
#include <vector>
#include <map>
#include "MultiChannelMemorySystem.h"
#include "Cache.h"
#include "../inout.h"
//#include "../Node/LSE.h" 
#include "../Node/Node.h"

class Simulator::Array::Loadstore_element;
class Simulator::Array::Spm;

typedef struct comb_offset {
	bool valid;
	short pointer;
	short tag;
}Comb_Offset;

typedef struct comb_offset_4 {
	bool valid;
	short mask;
	short TAG_;
	short tag;
}Comb_Offset_4;

typedef struct vec_msg {
	short pointer;
	short step;
	short size;
}Vec_Msg;

typedef struct offset_line {
	std::vector <Comb_Offset> offset;
//	bool round;
	bool valid;
}Offset_Line;

typedef struct bus_tran {
	uint64_t addr;
	bool rdwr;
	short cycle;
}Bus_Tran;

namespace DRAMSim
{
#define OUTPUT_LOG
#define PRINTERRORM(str)  { Simulator::Array::Debug::getInstance()->getlsuFile()<<str<<std::endl; std::cout <<str<<std::endl;}
#define PRINTERRORMN(str)  { Simulator::Array::Debug::getInstance()->getlsuFile()<<str; std::cout <<str;}
#ifdef OUTPUT_LOG //输出到文件还是屏幕
	#define PRINTM(str)  { Simulator::Array::Debug::getInstance()->getlsuFile()<<str<<std::endl; }
	#define PRINTMN(str) {  Simulator::Array::Debug::getInstance()->getlsuFile() <<str; }
#else
	#define PRINTM(str)  if(SHOW_SIM_OUTPUT) { std::cout <<str<<std::endl; }
	#define PRINTMN(str) if(SHOW_SIM_OUTPUT) { std::cout <<str; }
#endif
	class TabLine
	{
	private:


	public:
		uint32_t valid;
		bool rdwr;
		TabLine();
		~TabLine() {};
//		bool pe_round;
		uint32_t ADDR_;
		bool bypass;
		uint32_t entry;
		uint32_t TAG_;   //pointer
		uint32_t pe_tag;
		bool complete;
		bool pref;
		int transid;
		bool vec;
		vector <Comb_Offset> offset;                 //mask通过位置表示
		vector <Comb_Offset_4> offset_4;
		void reset();
	};


	class ArbitratorLine :public Simulator::Array::Node
	{
	private:
		//void*data_; 
		//uint32_t counter;

	public:
		Simulator::Array::Loadstore_element* lse_;
//		Simulator::Array::Loadstore_element* se_;
		uint32_t TAG_;
		uint32_t ADDR_;
		uint bankId;
		ArbitratorLine(const Simulator::Preprocess::ArrayPara para, Simulator::Array::Loadstore_element* lse, uint32_t TAG);
		ArbitratorLine(const Simulator::Preprocess::ArrayPara para, uint32_t TAG);
		~ArbitratorLine() {};
		uint32_t valid;
		bool rdwr;
		bool bypass;
		uint32_t pe_tag;
//		bool pe_round;                      //和tag一样全程保持不变返回
											//uint32_t round_counter;
		bool pref;
		bool AddTrans(Simulator::Array::Port_inout_lsu input, uint32_t TAG,bool bypass);
		void returnACK();
		void getInput(uint port, Simulator::Array::Port_inout input) override {};
		void getBp(uint port, bool input) override {};
	};


	class Arbitrator :public Simulator::Array::Node
	{
	private:
	//	uint32_t lsize;
	//	uint32_t ssize;
		uint32_t lseSize;

	public:
		uint32_t pointer;
		std::map<uint, uint> lse2relse;
	//	bool print_screen;
		vector<ArbitratorLine*> ArbitratorLines;
		Arbitrator(const Simulator::Preprocess::ArrayPara para, map<uint, Simulator::Array::Loadstore_element*> lse_map);
		~Arbitrator();
		bool AddTrans(Simulator::Array::Port_inout_lsu input, uint32_t TAG,bool bypass);
		void getInput(uint port, Simulator::Array::Port_inout input) override {};
		void getBp(uint port, bool input) override {};
	};


	class Lsu :public Simulator::Array::Node
	{
	private:
		Arbitrator* arbitrator;
		uint32_t completetran;
		vector<int> config_reg;
		bool NotSameBlock(uint32_t addr_,bool bypass);
		bool NotSameBlock(uint32_t addr_);
		int IsVecTran(uint32_t pointer);
		bool IsPrefTran(uint32_t pointer);
		bool IsRTran(uint32_t pointer);
		list<uint> last_pref;                               //用于记录之前的pref请求，深度与tagbits对应
		vector<uint> pref_pointer;                            //记录预取的端口
		vector<uint> r_pointer;
		int send_pointer;    //用于指向下一条要发送的请求
		uint lseSize;
		bool print_enable;
		bool PostNotFull();
	public:
		Lsu(const Simulator::Preprocess::ArrayPara para, map<uint, Simulator::Array::Loadstore_element*> lse_map);
		~Lsu() ;
		Cache* cache;
		Simulator::Array::Spm* spm;
		uint conflict_times;
		uint add_times;
		uint seek_bank();
		uint receive_enable(uint transnum);
		void attachSpm(Simulator::Array::Spm *spm_);
		bool AddTrans(Simulator::Array::Port_inout_lsu input, uint TAG,bool bypass);
		bool returnACK(uint bankId);
		void AttachMem(DRAMSim::MultiChannelMemorySystem* memory);
		void write_hit_complete(uint32_t addr, uint32_t i);
		void read_hit_complete(uint32_t addr, uint32_t i);
		void update();     //synchronic
		void addpoped_addr(int addr, bool bypass);
		void config_in(map<uint, Simulator::Array::Loadstore_element*> lse_map);
		void read_miss_complete(uint32_t addr);            //被动式miss，完成后通知table进行访问cache（需占用cache port并将cache state拉高）
		void write_miss_complete(uint32_t addr);
		void read_hit_complete(uint32_t addr);             //主动式hit，完成后进入reg搜索
		void write_hit_complete(uint32_t addr);

		DRAMSim::MultiChannelMemorySystem* mem;
		//vector<TabLine *> table;
		vector <vector<TabLine*>> pre_fifo;
		vector <TabLine*> post_table;
		vector <vector<Offset_Line>> post_offset;
		bool channel_occupy[32];
		short bank_round = 0;
		bool profiling = 1;
		uint pref_history = 16;
		std::ofstream cachefile;
		std::ofstream overlapfile_1;
		uint64_t ClockCycle;
		uint32_t finished_trans;
		uint32_t total_trans;
		uint32_t finished_rd;
		uint32_t finished_wr;
		uint32_t finished_pref;
		uint32_t total_wr;
		uint32_t total_rd;
		int transid;                                          //唯一标识！
		vector<Vec_Msg> vec_pointer;                          //记录每个向量端口配置的index、step、size信息
		unordered_map<uint32_t, Simulator::RecallMode>poped_addr;                              //表示尚未完成的miss块
		list <Bus_Tran> bus;
		void bus_update();
		void config_in(vector<int> config);

		uint32_t cache_rd_reg;
		vector <TabLine*> inflight_reg;                              //若为pipeline_cache则是寄存器组而不是一个寄存器
		vector<TabLine> pending_inflight;
		bool addTrans2post(TabLine* line);
		bool addreserve2post(TabLine* line);
		//bool RegMshrConflict();
		void release_poped_addr(uint32_t addr);
//		bool channel_occupy[leNums];
		//vector<Comb_Offset> now_miss_offset;
		//vector<Comb_Offset> next_miss_offset;
		//short next_index;
		//bool next_index_v;
		//short this_index;
		//bool this_index_v;
		//bool fifo_pause_signal;

		short MSHR_STATE;
		uint miss_index;
		uint miss_finished_cnter;
		bool state3_reg;


		double lsu_out = 0;
		double lsu_fifo = 0;
		double lsu_fifo_max = 0;
		double lsu_mshr = 0;
		double lsu_mshr_max = 0;
		double read_prf = 0;
		double dram_port = 0;
		double invalid_pref = 0;
//		double non_pref_miss = 0;


		//void print() override{};
		void getInput(uint port, Simulator::Array::Port_inout input) override{};
		void getBp(uint port, bool input) override{};
	};
}