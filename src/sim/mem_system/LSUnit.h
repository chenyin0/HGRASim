#pragma once
//暂时使用8offset形式，硬件中应使用按channel划分的offset

#include "iostream"                        
#include "stdlib.h"                       
#include "math.h"                         //pay attention to ACK
#include <vector>
#include <map>
#include "MultiChannelMemorySystem.h"
#include "Cache.h"
#include "../Node/LSE.h" 
//#include "define.h"

//#define TABLELINE_NUM  40
//#define OUT_NUM   2      暂时出口数为无限         
//#define IN_NUM   10                               //leNums+seNums
//#define BUSDELAY       30
using namespace DRAMSim;

typedef     unsigned char       UINT8;
typedef     unsigned short      UINT16;
//typedef     unsigned int        uint32_t;
//typedef     unsigned long       uint64_t;   

//extern bool PRINT_SCREEN;

using std::vector;

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
	bool round;
	bool valid;
}Offset_Line;

typedef struct bus_tran {
	uint64_t addr;
	bool rdwr;
	short cycle;
}Bus_Tran;
//using std::string;
namespace Simulator::Array
{
	class Loadstore_element;
}

namespace DRAMSim {
	class TabLine
	{
	private:


	public:
		uint32_t valid;
		uint32_t rdwr;
		TabLine();
		~TabLine() {};
		bool pe_round;
		uint32_t ADDR_;
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


	class ArbitratorLine
	{
	private:
		//void*data_; 
		//uint32_t counter;

	public:
		Simulator::Array::Loadstore_element* le_;
		Simulator::Array::Loadstore_element* se_;
		uint32_t TAG_;
		uint32_t ADDR_;
		ArbitratorLine(Simulator::Array::Loadstore_element* le, Simulator::Array::Loadstore_element* se, uint32_t TAG);
		~ArbitratorLine() {};
		uint32_t valid;
		uint32_t pe_tag;
		bool pe_round;                      //和tag一样全程保持不变返回
											//uint32_t round_counter;
		bool pref;
		bool AddTrans(uint32_t ADDR, uint32_t TAG, uint32_t V, uint32_t PE_TAG, bool PE_ROUND);
		void returnACK();
	};


	class Arbitrator
	{
	private:
		uint32_t lsize;
		uint32_t ssize;

	public:
		uint32_t pointer;
		vector<ArbitratorLine *> ArbitratorLines;
		Arbitrator(Simulator::Array::Loadstore_element** lep, Simulator::Array::Loadstore_element** sep);
		~Arbitrator();
		bool AddTrans(uint32_t ADDR, uint32_t TAG, uint32_t V, uint32_t PE_TAG, bool PE_ROUND);
	};


	class LSUnit
	{
	private:
		Arbitrator * arbitrator;
		uint32_t completetran;
		vector<int> config_reg;
		bool NotSameBlock(uint32_t addr_);
		int IsVecTran(uint32_t pointer);
		bool IsPrefTran(uint32_t pointer);
		list<uint> last_pref;                               //用于记录之前的pref请求，深度与tagbits对应
		vector<uint> pref_pointer;                            //记录预取的端口
		int send_pointer;                                     //用于指向下一条要发送的请求
		bool PostNotFull();


	public:
		LSUnit(map<uint, Loadstore_element*> lse_map, Simulator::Array::Loadstore_element** sep);//在构造函数中的Load**和Store**并没有使用
		~LSUnit();
		Cache *cache;
		bool AddTrans(uint32_t ADDR, uint32_t TAG, uint32_t V, uint32_t PE_TAG, bool PE_ROUND);
		void AttachMem(MultiChannelMemorySystem *memory);
		void update();     //synchronic

		void read_miss_complete(uint32_t addr);            //被动式miss，完成后通知table进行访问cache（需占用cache port并将cache state拉高）
		void write_miss_complete(uint32_t addr);
		void read_hit_complete(uint32_t addr);             //主动式hit，完成后进入reg搜索
		void write_hit_complete(uint32_t addr);

		MultiChannelMemorySystem *mem;
		//vector<TabLine *> table;
		vector <TabLine *> pre_fifo;
		vector <TabLine *> post_table;
		vector <vector<Offset_Line>> post_offset;

		ofstream cachefile;
		ofstream overlapfile_1;
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
		vector<uint32_t> poped_addr;                              //表示尚未完成的miss块
		list <Bus_Tran> bus;
		void bus_update();
		void config_in(vector<int> config);

		uint32_t cache_rd_reg;
		TabLine* inflight_reg;                                //若为pipeline_cache则是寄存器组而不是一个寄存器
		bool addTrans2post(TabLine* line);
		bool addreserve2post(TabLine* line);
		//bool RegMshrConflict();
		void release_poped_addr(uint32_t addr);

		//vector<Comb_Offset> now_miss_offset;
		//vector<Comb_Offset> next_miss_offset;
		//short next_index;
		//bool next_index_v;
		//short this_index;
		//bool this_index_v;
		//bool fifo_pause_signal;

		short MSHR_STATE;
		short miss_index;
		short miss_finished_cnter;
		bool state3_reg;

	};
}

