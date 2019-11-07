#pragma once

#include "engine/placement_board.h"
#include "engine/graph_analysis.h"

namespace Simulator::Mpr
{
	enum class CostFunType
	{
		ChessBoard,
		PunishedDistance
	};

	class CostFunDataBase
	{
	public:
		explicit CostFunDataBase(const AppGraph& app_graph_)
			: _app_nodes(app_graph_.getNodes())
			, _analysis_graph(app_graph_)
		{
		}
		
		auto getFunction(CostFunType type_) ->function<double(const vector<uint>&, const PlacementBoard&)>
		{
			if (type_ == CostFunType::ChessBoard)
				return std::bind(&CostFunDataBase::funPureChessBoard, &*this, std::placeholders::_1, std::placeholders::_2);
			else if (type_ == CostFunType::PunishedDistance)
				return std::bind(&CostFunDataBase::funPunishedChessBoard, &*this, std::placeholders::_1, std::placeholders::_2);;
			return nullptr;
		}

	private:
		double funPureChessBoard(const vector<uint>& nodes_, const PlacementBoard& place_)
		{
			double cost = 0;
			for_each(begin(nodes_), end(nodes_),
				[&](int index_)
			{
				const vector<AppLink>& all_next = _app_nodes[index_].next_node;
				for_each(begin(all_next), end(all_next),
					[&](const AppLink& link_)
				{
					uint target_node_index = link_.target_node_vec_index;

					if (!place_.queryExist(target_node_index))
						return;

					Cord source_cord = place_.queryCord(index_);
					Cord target_cord = place_.queryCord(target_node_index);

					cost += Cord::chessboardDist(source_cord, target_cord);
				});
			});

			return cost;
		}

		double funPunishedChessBoard(const vector<uint>& nodes_, const PlacementBoard& place_)
		{
			double cost = 0;
			for (uint i = 0; i < nodes_.size(); i++)
			{
				for (uint j = i; j < nodes_.size(); j++)
				{
					uint source_node_index = nodes_[i];
					Cord source_cord = place_.queryCord(source_node_index);
					uint target_node_index = nodes_[j];
					Cord target_cord = place_.queryCord(target_node_index);
					uint dist = _analysis_graph.getDistance(source_node_index, target_node_index);
					cost +=  Cord::chessboardDist(source_cord, target_cord) * dist * dist;
				}
			}

			return cost;
		}

	private:
		const vector<AppNode>& _app_nodes;
		AnalysisAppGraph _analysis_graph;
	};
	
}