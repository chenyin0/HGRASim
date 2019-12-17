#pragma once

#include "../../../../app_graph/app_graph.h"

namespace Simulator::Mpr
{
	struct AnalysisAppGraphNode
	{
		BlockType type;
		uint type_num;
		vector<uint> next;

		AnalysisAppGraphNode();

		AnalysisAppGraphNode(BlockType type_, uint type_num_);

		bool operator== (const AnalysisAppGraphNode& node_) const;
	};
	
	class AnalysisAppGraph
	{
	public:
		explicit AnalysisAppGraph(const AppGraph& app_graph_);

	public:
		uint getDistance(uint node_index1_, uint node_index2_) const;
		
	private:
		void init(const AppGraph& app_graph_);
		
		uint dijkstra(uint source_, uint end_) const;

	private:
		vector<AnalysisAppGraphNode> _nodes;

		vector<vector<uint>> _dist;
	};
	
}