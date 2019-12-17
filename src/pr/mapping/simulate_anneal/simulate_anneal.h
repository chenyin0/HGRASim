#pragma once

#include "engine/sa_engine.h"
#include "sa_cost_function.hpp"
#include "sa_temperature_function.hpp"

namespace Simulator::Mpr
{
	enum class SimulateAnnealMethod
	{
		Normal,
		Stage
	};

	class SimulateAnnealBase
	{
	public:
		explicit SimulateAnnealBase(const AppGraph& app_graph_);

		vector<Cord> getPlacementResult() const;

		void savePlacementResult(const string& addr_) const;

	protected:
		const AppGraph& _app_graph;
		PlacementBoard _result;
		CostFunDataBase _cost_fun_database;
		TemperatureFunDataBase _temperature_fun_database;
	};
	
	class SimulateAnnealStage : public SimulateAnnealBase
	{
	public:
		enum class Stage
		{
			LS,
			other
		};
		
	private:
		struct StagePara
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
		explicit SimulateAnnealStage(const AppGraph& app_graph_);

	public:
		void goSimulateAnneal();

		void setParaNumber(Stage stage_, int inner_number_, double lambda_, double epsilon_);

		void setParaCostFun(Stage stage_, CostFunType cost_fun_type_);

		void setParaTemperatureFun(Stage stage_, TemperatureFunType temperature_fun_type_);

	private:
		void initStage();
		
	private:
		StagePara _ls_stage;
		StagePara _other_stage;
	};

	class SimulateAnnealOverall : public SimulateAnnealBase
	{
	private:
		struct Para
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
		explicit SimulateAnnealOverall(const AppGraph& app_graph_);

	public:
		void goSimulateAnneal();

		void setParaNumber(int inner_number_, double lambda_, double epsilon_);

		void setParaCostFun(CostFunType cost_fun_type_);

		void setParaTemperatureFun(TemperatureFunType temperature_fun_type_);

	private:
		void initPara();

	private:
		Para _para;
	};
}