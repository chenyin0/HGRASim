//封装在类内的cache，无数据   注意cache地址以字为单位，dram地址以字节为单位,所以发送地址时要移位处理
//cache 占用了太多内存，需要优化
#ifndef CACHE
#define CACHE
#include <stdint.h>
#include <math.h>
#include <iostream>
//#include "../typedef.h"
#include "../debug.h"
#include "../Node/Node.h"
//extern bool PRINT_SCREEN;
typedef struct cache_trans {
	uint32_t addr;
	bool rdwr;
	short cycle;
	bool valid;
}Cache_Trans;
namespace DRAMSim
{
#define CACHE_BANK      8
#define CACHE_BANK_BIT  3

	//可配置工作模式    cache分为v个组，每组k路,v*k=m
	//#define CACHE_MODE      1        //1 2 3分表组关联、全关联、直接映射  配置，v=m,ways=1为直接映射，v=1,ways=m为关联映射，在中间的为组关联
#define WRITE_MODE      1        //0 1 分别表示write_through 和 write_back，与write_back次数有关，若有write_back延时则有影响
#define REPLACE_MODE    1        //0 1 2分别表示随机替换、最近最少使用（每组ways越多，硬件代价越大）、FIFO， 影响hit rate,   the function of 1 2 have not been tested!!
	//#define FIFO_DEPTH      5
#define DATA_CACHE_SIZE         32*1024      // 8KB
#define DATA_LINE_SIZE          8           // 一块几个字
#define CACHE_WAYS_NUM          4
#define CACHE_LINE_EACH_WAY     (DATA_CACHE_SIZE/(CACHE_BANK*DATA_LINE_SIZE*CACHE_WAYS_NUM*sizeof(uint32_t)))   //sizeof表示字节数  8KB组数64


	//延时定义                               从table出数->cache中默认已经需要1个cycle，若需要一个cycle出一个数，hit_delay应该为0
	//#define LOOKUP_DELAY            0
#define HIT_DELAY               1               //至少为1
#define LOOKUP_DELAY            1
	//#define REPLACE_DELAY           2
#define WRITEBACK_DELAY         0     //并行？
#define WRITETHROUGH_DELAY      0
#define READHIT_DELAY           0
#define WRITEHIT_DELAY          0
	//#define READ_DELAY           1
	//#define WRITE_DELAY          1


	// cacheline bit masks
#define     CACHE_LINE_SIZE         (DATA_LINE_SIZE)      // 一个cache行的大小 words
#define     UPDATE_BIT               (1 << 0)              //UPDATE位用于替换时判断写回
#define     VALID_BIT                (1 << 1)              //标注起始cache内数据not valid 
	//#define     COUNTER_BIT              (0x3 << 2)              //三、四位用于replace的计算,若超过4路组关联需要更多的位数

	// address bit masks
#define     ADDR_INDEX              3 + CACHE_BANK_BIT
#define     INDEX_BITS              ((int)(CACHE_LINE_EACH_WAY - 1) << ADDR_INDEX)   
#define     ADDR_TAGS               (int)((log(CACHE_LINE_EACH_WAY)/log(2)) +  ADDR_INDEX)
#define     TAGS_BITS               (((1 << (32 - ADDR_TAGS)) - 1) << ADDR_TAGS)                   //注意该项和寻址空间有关！
#define     ADDR_WORD               0
#define     WORD_BITS               (0x7 << ADDR_WORD)
#define     ADDR_BANK               3
#define     BANK_BITS               (((1 << CACHE_BANK_BIT) - 1) << ADDR_BANK)
	//#define     ADDR_BYTE               (0x3 << 0)

	//UINT32 CacheTopoFindWord[CACHE_WAYS_NUM][CACHE_LINE_EACH_WAY][CACHE_LINE_SIZE];    //拓扑逻辑定位words 路-组-字，用于存放数据


	/* address bit fields:      28位地址寻址空间2^28words
	* bits[31:27] = useless
	* bits[27:9] = Tag
	* bits[8:5] = Set(=index)
	* bits[4:3] = Bank
	* bits[2:0] = Word
	*/


#define     DRAM_SIZE       0xfffffff     // dram size = 2^28 words

	//#define     DRAM_BASE       0x0000      // dram address = DRAM_BASE + offset in DramTopology
	//UINT32 DramTopology[DRAM_SIZE];   //DRAM 拓扑逻辑


	//class FIFOLINE
	//{
	//private:
	//   uint32_t addr;
	//   uint32_t rdwr;
	//   uint32_t valid;
	//public:
	//   FIFOLINE();
	//   ~FIFOLINE();
	//   bool addtranscation(uint32_t adr, uint32_t rw);
	//   //void shift();
	//}

	using namespace std;

	class Lsu;


	class Cache :public Simulator::Array::Node
	{
	private:
		Lsu* lsunit;
		bool print_enable;
		//uint32_t line;
		//uint32_t data;    //数据 读和写  
		//vector<FIFOLINE *> fifo;
		Cache_Trans lookup_trans[CACHE_BANK];
		//vector <Cache_Trans> missfifo;
		vector <Cache_Trans> hitfifo[CACHE_BANK];



	public:
		Cache(const Simulator::Preprocess::ArrayPara para);
		~Cache();
		int misscounter;
		int hitcounter;
		void getInput(uint port, Simulator::Array::Port_inout input) override {};
		void getBp(uint port, bool input) override {};
		bool addtranscation(uint32_t adr, uint32_t rw);
		short counter[CACHE_BANK][CACHE_WAYS_NUM][CACHE_LINE_EACH_WAY];  // priority
		uint32_t CacheTopology[CACHE_BANK][CACHE_WAYS_NUM][CACHE_LINE_EACH_WAY];   //用于存放标记、UPDATE位、valid位 
																	   //uint32_t ReplaceWay;
																	   //uint32_t lineWord;   //字偏移量 
																	   //uint32_t lineIndex;
																	   //uint32_t lineTag;
																	   //uint32_t lineWay;
		uint32_t state[CACHE_BANK] = { 0 };     //idle=0,busy=1
		uint32_t addr_in[CACHE_BANK] = { 0 };
		bool rdwr_in[CACHE_BANK] = { 0 };
		bool trans_v[CACHE_BANK] = { 0 };
		vector <uint32_t> WB_table;                          //用于收回writeback和writethrough产生的callback
															 //void update();
															 //void attachlsu();
		void attach(Lsu* lsu);
		void do_replacement(uint32_t addr);
		void do_writeback(uint32_t ReplaceWay, uint32_t lineIndex, uint32_t bank);
		void do_writethrough(uint32_t addr);
		void do_readhit(uint32_t addr);
		void do_writehit(uint32_t addr);
		bool do_lookup(uint32_t addr);
		bool addtranscation(uint32_t adr, uint32_t rw, uint32_t bank);
		bool print_screen;
		bool bus_enable;
		bool cache_allhit;
		uint non_pref_miss;
		//void do_read(uint32_t ReplaceWay);
		//void do_write(uint32_t ReplaceWay);
		void mem_read_complete(unsigned id, uint64_t address, uint64_t clock_cycle);
		void mem_write_complete(unsigned id, uint64_t address, uint64_t clock_cycle);
		void update();
	};
}
#endif