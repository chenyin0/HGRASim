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
		DRAMSim::MultiChannelMemorySystem* mem;
		vector<type_index> config_order;
		uint clk;
		//Debug debug;

		bool sendOutput(Simulator::NodeType type, uint index);
		void sendBpOut(Simulator::NodeType type, uint index);

	public:
		HgraArray(const AppGraph& app_graph);
		~HgraArray();
		void run();
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