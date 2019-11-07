#include "sa_engine.h"
#if DEBUG_CONSOLE
#include <iostream>
#include <utility>
#endif

namespace Simulator::Mpr
{
	SimulateAnnealPara::SimulateAnnealPara(int inner_number_, double lambda_, double epsilon_, vector<uint> nodes_to_place_, function<double(double, double)> temperature_fun_, function<double(const vector<uint>&, const PlacementBoard&)> cost_fun_)
		: inner_number(inner_number_)
		, lambda(lambda_)
		, epsilon(epsilon_)
		, nodes_to_place(std::move(nodes_to_place_))
		, temperature_fun(std::move(temperature_fun_))
		, cost_fun(std::bind(cost_fun_, nodes_to_place, std::placeholders::_1))
	{
	}

	SimulateAnnealEngine::SimulateAnnealEngine(const AppGraph& app_graph_)
		: _app_graph(app_graph_)
		, _para(nullptr)
		, _placement(nullptr)
	{
	}

	auto SimulateAnnealEngine::goSimulatedAnneal() -> void
	{
		DEBUG_ASSERT(checkLoad());
#pragma region Init
		uint nodes_num = _para->nodes_to_place.size();
		randomGenerate();
		double init_cost = _para->cost_fun(*_placement);
		double standard_deviation = initAStandardDeviation();
		double temperature = 20 * standard_deviation;

		// 有些cost函数产生0的cost，直接返回当前布局，认为退火完成
		if (temperature == 0)
			return;

		int moves_per_t = static_cast<int>(static_cast<double>(_para->inner_number) * std::pow(_app_graph.getSize(), 4.0 / 3.0));
#pragma endregion 

#pragma region Assist Function
		auto is_over = [&](double cost_) -> bool
		{
			double limit = _para->epsilon * (cost_ / nodes_num);
			return temperature < limit;
		};
#pragma endregion 

#pragma region Main Loop
		double cost = init_cost;
		while (!is_over(cost))
		{
			PlacementBoard temp_place(*_placement);
			int accept_count = 0;
			for (int i = 0; i < moves_per_t; i++)
			{
				uint node_index = _para->nodes_to_place[Util::uRandom(0, nodes_num - 1)];
				temp_place.swapOnce(node_index);
				double temp_cost = _para->cost_fun(temp_place);

				if (temp_cost < cost)
				{
					accept_count++;
					std::swap(*_placement, temp_place);
					std::swap(cost, temp_cost);
				}
				else
				{
					int random_number = Util::uRandom(1, 100);
					int accept_threshold = static_cast<int>(100 * std::exp(-std::abs(cost - temp_cost) / temperature));
					if (random_number < accept_threshold)
					{
						accept_count++;
						std::swap(*_placement, temp_place);
						std::swap(cost, temp_cost);
					}
				}
			}
			double alpha = static_cast<double>(accept_count) / moves_per_t;
#if DEBUG_CONSOLE
			std::cout << "T = " << temperature << ", ";
			std::cout << "Cost = " << cost << ", ";
			std::cout << "Alpha = " << alpha << std::endl;
#endif

			temperature = _para->temperature_fun(temperature, alpha);
		}
#pragma endregion 
	}

	auto SimulateAnnealEngine::loadPara(SimulateAnnealPara* ptr_) -> void
	{
		_para = ptr_;
	}

	auto SimulateAnnealEngine::detachPara() -> void
	{
		_para = nullptr;
	}

	auto SimulateAnnealEngine::loadPlacement(PlacementBoard* ptr_) -> void
	{
		_placement = ptr_;
	}

	auto SimulateAnnealEngine::detachPlacement() -> void
	{
		_placement = nullptr;
	}

	auto SimulateAnnealEngine::checkLoad() const -> bool
	{
		return _para != nullptr && _placement != nullptr;
	}

	auto SimulateAnnealEngine::randomGenerate() -> void
	{
		_placement->clear(_para->nodes_to_place);
		const vector<AppNode>& app_nodes = _app_graph.getNodes();

		for_each(begin(_para->nodes_to_place), end(_para->nodes_to_place),
			[&](uint node_index_)
		{
			BlockType node_type = app_nodes[node_index_].type;
			_placement->randomPutNode(node_type, node_index_);
		});
	}


	double SimulateAnnealEngine::initAStandardDeviation() const
	{
		uint node_num = _para->nodes_to_place.size();
		vector<double> cost_list;
		PlacementBoard temp_placement(*_placement);
		for (uint i = 0; i < node_num; i++)
		{
			uint node_index = _para->nodes_to_place[Util::uRandom(0, node_num - 1)];
			temp_placement.swapOnce(node_index);
			cost_list.push_back(_para->cost_fun(temp_placement));
		}
		return Util::standardDeviation(cost_list);
	}
}
