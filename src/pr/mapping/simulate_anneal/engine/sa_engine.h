#pragma once

#include "graph_analysis.h"
#include "placement_board.h"
#include "../../../resource_graph/pr_res_graph.h"

namespace Simulator::Mpr
{
	struct SimulateAnnealPara
	{
	public:
		int inner_number;
		double lambda;
		double epsilon;

		// store nodes needed to be placed, every element is the index in app_graph
		vector<uint> nodes_to_place;

		// input: current temperature, alpha
		function<double(double, double)> temperature_fun;

		// input: source node, source node cord, target node, target node cord
		function<double(const PlacementBoard&)> cost_fun;

	public:
		SimulateAnnealPara(int inner_number_, double lambda_, double epsilon_,
			vector<uint> nodes_to_place_,
			function<double(double, double)> temperature_fun_,
			function<double(const vector<uint>&, const PlacementBoard&)> cost_fun_);
	};

	class SimulateAnnealEngine
	{
	public:
		SimulateAnnealEngine(const AppGraph& app_graph_);

	public:
		auto goSimulatedAnneal() -> void;

		auto loadPara(SimulateAnnealPara* ptr_) -> void;

		auto detachPara() -> void;
		
		auto loadPlacement(PlacementBoard* ptr_) -> void;

		auto detachPlacement() -> void;

	private:
		auto checkLoad() const -> bool;
		
		auto randomGenerate() -> void;

		auto initAStandardDeviation() const -> double;

	private:
		const AppGraph& _app_graph;
		
		SimulateAnnealPara* _para;

		PlacementBoard* _placement;
	};
}
