#pragma once
/*
 *˵��
 *���ļ���Ϊ����ϵͳ�������Ա�ķ�װ
 *����������������������εķ�װ
 *1. �����м���ͨ��HgraArray���õ�����
 *2. �����м���ͨ��HgraMultiArray���ö�����
 *3. ϵͳ����ͨ��HgraSystem����ϵͳ
 */


#include "../define/define.hpp"
#include "../app_graph/app_graph.h"
#include "../bridge/bridge.h"
#include "memory/MemoryData.h"
#include "debug.h"
#include "Node/LC.h"
#include "Node/LSE.h"
#include "Node/PE.h"
#include "Node/Cluster.h"
#include "Node/SPM.h"
 //#include "mem_system/MultiChannelMemorySystem.h"
 //#include "../sim/mem_system/Lsu.h"

namespace Simulator::Array
{
	struct type_index
	{
		NodeType type;
		uint index;
		//		uint order;
		//		uint order;
	};

	class HgraArray
	{
	private:
		Bridge bridge;
		map<uint, Processing_element*> pe_map;
		map<uint, Loadstore_element*> lse_map;
		map<uint, LoopControl*> lc_map;
		Match_set* match_set;
		DRAMSim::Lsu* lsu;
		Simulator::Array::Spm *spm;
		DRAMSim::MultiChannelMemorySystem* mem;
		vector<type_index> config_order;
		uint clk;
		uint pe_ulti;
		uint pe_ulti_cnt;
		uint total_key_pe;
		ClusterGroup cluster_group= ClusterGroup();
		vector<Simulator::Bridge::Location> reserved_nodes;
		std::map<uint, std::pair<NodeType, uint>> order2index;//config_orderת����pe_map�е���pe_map��indexΪ������Ԫ��
		std::map< std::pair<NodeType, uint>,uint> ele2order;
		std::map<std::pair<NodeType, uint>, uint> index2order;//pe_map�е�Ԫ��ת����pe_map��index
		std::map<std::tuple<NodeType, uint,uint>, vector<std::tuple<uint,int,bool>>> data_flow;
		//Debug debug;

		bool sendOutput(Simulator::NodeType type, uint index);
		void sendBpOut(Simulator::NodeType type, uint index);

	public:
		HgraArray(const AppGraph& app_graph);
		~HgraArray();
		bool task_finish = false;
		bool update_flag = false;
		const char* maps[7] = { "pe","fg","ls","lv","lc","null","begin" };
		void run();
		void calc_data();
		uint number_key_pe();
		void pe_update(uint);
		void nextStep1(type_index type_in,uint port);
		void reverseStep1(type_index type_in, uint port);
		void printStall(type_index type_in, uint port);
		void initIndex();
		static void power_callback(double a, double b, double c, double d) {}
	};

	class HgraMultiArray
	{
	private:
		unordered_map<uint, Bridge> multi_bridge;
		unordered_map<uint, vector<Processing_element*>> multi_pe_vec;
		unordered_map<uint, vector<Loadstore_element*>> multi_lse_vec;
		unordered_map<uint, vector<LoopControl*>> multi_lc_vec;
		unordered_map<uint, Match_set*> multi_match_set;
		unordered_map<uint, DRAMSim::Lsu*> multi_lsu;

	public:
		//HgraMultiArray(const AppGraph& app_graph);
		//~HgraMultiArray();
		//void run();

	};
}