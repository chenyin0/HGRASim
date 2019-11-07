#pragma once

#include "simulate_anneal/engine/sa_engine.h"
#include "routing_engine/routing_engine.h"
#include "simulate_anneal/sa_temperature_function.hpp"

namespace Simulator::Mpr
{
	enum class TemperatureFunType;
	// TODO finish this junk
	class MixedMapping
	{
	private:
		struct SimulatedAnnealPara
		{
			int inner_number = 30;
			double lambda = 0.7;
			double epsilon = 0.01;

			// store nodes needed to be placed, every element is the index in app_graph
			vector<uint> nodes_to_place;

			//funs
			function<double(const vector<uint>&, const PlacementBoard&)> cost_fun = nullptr;
			function<double(double, double)> temperature_fun = nullptr;

			bool check() const;
		};
		
	public:
		MixedMapping(const ResourceGraph& r_graph_, const AppGraph& app_graph_);

	public:
		void goMapping();

		void setParaNumber(int inner_number_, double lambda_, double epsilon_);

		void setParaTemperatureFun(TemperatureFunType temperature_fun_type_);
		
	private:
		double costFunction(const vector<uint>& nodes_, const PlacementBoard& placement_board_);

		void initPara();
		
	private:
		const AppGraph& _app_graph;
		TemperatureFunDataBase _temperature_fun_database;
		
		SimulatedAnnealPara _para;
		SimulateAnnealEngine _placement_engine;
		RoutingEngine _routing_engine;

		PlacementBoard _placement;
		
	};
	
}
