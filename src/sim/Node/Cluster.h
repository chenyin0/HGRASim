#pragma once
#include "Node.h"
#include "../mem_system/Lsu.h"
#include "../debug.h"
#include "../memory/MemoryData.h"
#include "../debug.h"
#include "../ClkDomain.h"

namespace Simulator::Array {
	class Cluster
	{
	private:
		const shared_ptr<Preprocess::ClusterInterface> attribution;
		uint counter;
		bool update_en;
	public:
		friend class ClusterGroup;
		Cluster(shared_ptr<Preprocess::ClusterInterface> attr);
		void enable();
		void update();
	};
	class ClusterGroup
	{
	private:
		const Preprocess::ClusterGroupInterface &attribution;
		unordered_map<std::pair<NodeType, uint>, shared_ptr<Cluster>> clusters;
	public:
		ClusterGroup();
		int index2Id(NodeType nodetype_, uint index);
		bool exists(NodeType nodetype_, uint index);
		bool canRecv(NodeType nodetype_, uint index);
		void update();
		void enable(NodeType nodetype_, uint cluster_id_);
	};
}