#pragma once
#include "Node.h"
#include "../debug.h"

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
		std::map<std::pair<NodeType, uint>, uint> index2order;
		map<uint, Simulator::Array::Loadstore_element*> lse_map;
		const Preprocess::ClusterGroupInterface &attribution;
		unordered_map<std::pair<NodeType, uint>, shared_ptr<Cluster>> clusters;
	public:
		ClusterGroup();
		int index2Id(NodeType nodetype_, uint index);
		Simulator::Array::Loadstore_element* getEle(NodeType nodetype_, uint lse_tag);
		bool exists(NodeType nodetype_, uint index);
		bool canRecv(NodeType nodetype_, uint index);
		void update();
		void print();
		void insert(std::map<std::pair<NodeType, uint>, uint> &index2order, map<uint, Simulator::Array::Loadstore_element*> &lse_map);
		void enable(NodeType nodetype_, uint cluster_id_);
	};
}