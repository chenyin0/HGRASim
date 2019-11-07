#include "graph_analysis.h"

namespace Simulator::Mpr
{
	AnalysisAppGraphNode::AnalysisAppGraphNode()
		: type(BlockType::null)
		, type_num(0)
		, next(vector<uint>{})
	{
	}

	AnalysisAppGraphNode::AnalysisAppGraphNode(BlockType type_, uint type_num_)
		: type(type_)
		, type_num(type_num_)
		, next(vector<uint>{})
	{
	}

	bool AnalysisAppGraphNode::operator==(const AnalysisAppGraphNode& node_) const
	{
		return type == node_.type && type_num == node_.type_num;
	}

	AnalysisAppGraph::AnalysisAppGraph(const AppGraph& app_graph_)
		: _nodes(app_graph_.getNodes().size())
		, _dist(vector<vector<uint>>(app_graph_.getNodes().size(), vector<uint>(app_graph_.getNodes().size(), 0)))
	{
		init(app_graph_);
	}

	uint AnalysisAppGraph::getDistance(uint node_index1_, uint node_index2_) const
	{
		return _dist[node_index1_][node_index2_];
	}

	void AnalysisAppGraph::init(const AppGraph& app_graph_)
	{
		const vector<AppNode>& app_nodes = app_graph_.getNodes();
		// build _nodes
		for (uint index = 0; index < app_nodes.size(); ++index)
		{
			_nodes[index].type = app_nodes[index].type;
			_nodes[index].type_num = app_nodes[index].type_num;

			for_each(begin(app_nodes[index].next_node), end(app_nodes[index].next_node),
				[&](const AppLink& link_)
			{
				uint target_index = link_.target_node_vec_index;
				_nodes[index].next.push_back(target_index);
				_nodes[target_index].next.push_back(index);
			}
			);
		}
		// build _dist, a half of matrix
		for (uint from = 0; from < app_nodes.size(); from++)
		{
			for (uint to = from; to < app_nodes.size(); to++)
			{
				if (to == from)
					_dist[from][to] = 0;
				else
					_dist[from][to] = dijkstra(from, to);
			}
		}
		// build _dist, another half
		for (uint i = 0; i < app_nodes.size(); i++)
		{
			for (uint j = 0; j < i; j++)
			{
				_dist[i][j] = _dist[j][i];
			}
		}
	}

	uint AnalysisAppGraph::dijkstra(uint source_, uint end_) const
	{
		vector<Bool> visited(_nodes.size(), false);
		vector<uint> dist(_nodes.size(), UINT_MAX);
		vector<uint> path(_nodes.size(), UINT_MAX);

		queue<uint> Q;

		visited[source_] = true;
		path[source_] = 0;
		dist[source_] = 0;
		Q.push(source_);

		bool is_success = false;
		while (!Q.empty())
		{
			uint cur_index = Q.front();
			Q.pop();
			
			visited[cur_index] = true;
			if (cur_index == end_)
			{
				is_success = true;
				break;
			}
				
			for (auto next : _nodes[cur_index].next)
			{
				if (visited[next])
					continue;

				if (dist[cur_index] + 1 < dist[next])
				{
					dist[next] = dist[cur_index] + 1;
					path[next] = cur_index;
				}

				Q.push(next);
			}
		}

		if (!is_success)
			DEBUG_ASSERT(false);

		// deal result
		stack<uint> s;
		uint temp = end_;
		while (temp != source_)
		{
			s.push(temp);
			temp = path[temp];
		}
		s.push(source_);

		return s.size();
	}
}
