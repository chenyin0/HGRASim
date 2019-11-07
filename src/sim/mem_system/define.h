//本文件包含各种程序中需要初始化的量
//每一项都要 拟器更改思路，生成最大数量的每种节点
//for no tag
#define outbuffer_depth 2		//outputBuffer的深度
#define inbuffer_depth 2		//inBuffer的深度
//for tag
#define TAGBITS   2
#define LEROUNDS  100                     //LE中允许有几个轮次 1 << LEROUNDS个
#define outtablebuffer_depth 4	            //outTableBuffer的深度
#define intablebuffer_depth 1<<TAGBITS 	//intablebuffer的深度
#define lebuffer_depth 32

//lebuffer的深度
//unit num
#define peNums 128				//PE总数
#define leNums 32				//LE总数
#define seNums 32				//SE总数
#define finegrainNums 32        //FG总数
#define LCNums 2                //LC总数

#define lbeginNums 5			//loop begin总数
#define lendNums 5				//loop end总数
#define joinNums 5				//join节点总数
#define switchNums 5				//switch节点总数
#define breakNums 5				//break节点总数
#define lendsNums 5				//loop end2节点总数
#define joinbpNums 30			//joinbp节点总数
#define lsunitConfig  1 

//#define taNums 2				//ta节点的数量
#define fgNums (lbeginNums+lendNums+joinNums+switchNums+breakNums+lendsNums+joinbpNums)				//总的fg节点的数量

#define unitTotalNums (peNums+leNums+seNums+fgNums+lsunitConfig+finegrainNums+LCNums)//+taNums)		//总的单元个数

#define joininportNums 10			//第三类细粒度模块（join节点）的输入端口数量
#define shitinportNums 0			//第三类细粒度模块（shit节点）的输入端口数量
#define memoryDepth 1000000			//保存输入数据memory的大小
#define memory2depth 2000000			//保存输出数据memory的大小

//with memory or not
#define ATTACH_MEMORY 	1		//仿真器加上或者不加上memory

//累加寄存器的边界值，下一次循环开始需要写入初始值的边界，超过这个值就要写入初值
//#define THRESHOLD 10000
#define END_CYCLE 569519

//LSU defination
#define BUS_ENABLE      1
#define BUS_DELAY       1
#define FIFOLINE_NUM    20
#define TABLINE_NUM     20
#define IN_NUM          16
#define OUT_NUM         16
//#define ADDR_COMB       0                      //表示硬件地址合并
//#define OFFSET_4        0
#define OFFSET_DEPTH    4
#define POST_OFFSET_DEPTH   8
#define PREF_HISTORY    16
#define CACHE_ALLHIT    0
//#define COMB_RANGE      60

//ALU_delay defination
#define MUL_Delay       3
#define ADD_Delay       1
#define MUX_Delay       1
#define SUB_Delay       1
#define DIV_Delay       3
#define MOD_Delay       1
#define COMB_Delay      1
#define SHIFT_Delay     1
#define LOGIC_Delay     1
#define COS_Delay       1
#define SIN_Delay       1


//若需要动态选择部分打印，将相应部分注释并在main函数中声明同名变量，对该变量进行操作
#define PRINT_SCREEN_BEGIN_CYCLE    1
#define PRINT_SCREEN_END_CYCLE      2
#define PRINT_DEBUG_BEGIN_CYCLE     0
#define PRINT_DEBUG_END_CYCLE       3000
#define PROFILING        1
#define PRINT_BUS        0



//VLE行为选项
#define V_TABLELINE_NUM 10     //每个VLE对应的v_table的深度
#define V_TABLE_DELAY   3      //每个请求需要在v_table中等待合并2的时间
#define V_OFFSET_DEPTH  8      //至少为8    ?
#define V_TABLE_CHANNEL 1      //一个v_table一个cycle内能往table发送几个数据
#define V_ROUND         8

