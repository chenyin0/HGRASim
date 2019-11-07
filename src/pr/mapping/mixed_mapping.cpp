#include "mixed_mapping.h"
#ifdef DEBUG_CONSOLE
#include <iostream>
#endif

namespace Simulator::Mpr
{
	bool MixedMapping::SimulatedAnnealPara::check() const
	{
		return cost_fun != nullptr && temperature_fun != nullptr;
	}

	MixedMapping::MixedMapping(const ResourceGraph& r_graph_, const AppGraph& app_graph_)
		: _app_graph(app_graph_)
		, _placement_engine(app_graph_)
		, _routing_engine(r_graph_, app_graph_)
		, _placement(app_graph_)
	{
		_para.cost_fun = std::bind(&MixedMapping::costFunction, &*this, std::placeholders::_1, std::placeholders::_2);
		initPara();
	}

	void MixedMapping::goMapping()
	{
		// prepare
		DEBUG_ASSERT(_para.check());

		// placement
		_placement_engine.loadPlacement(&_placement);

		SimulateAnnealPara para(
			_para.inner_number,
			_para.lambda,
			_para.epsilon,
			_para.nodes_to_place,
			_para.temperature_fun,
			_para.cost_fun
		);

		_placement_engine.detachPara();
		_placement_engine.loadPara(&para);
		_placement_engine.goSimulatedAnneal();

		// mapping
		vector<Cord> placement_cord = _placement.generateVectorCord();

		_routing_engine.loadPlacement(&placement_cord);
		_routing_engine.goRouting();
	}

	void MixedMapping::setParaNumber(int inner_number_, double lambda_, double epsilon_)
	{
		_para.inner_number = inner_number_;
		_para.lambda = lambda_;
		_para.epsilon = epsilon_;
	}

	void MixedMapping::setParaTemperatureFun(TemperatureFunType temperature_fun_type_)
	{
		_para.temperature_fun = _temperature_fun_database.getFunction(temperature_fun_type_);
	}

	double MixedMapping::costFunction(const vector<uint>& nodes_, const PlacementBoard& placement_board_)
	{
		vector<Cord> placement_cord = placement_board_.generateVectorCord();
		
		_routing_engine.loadPlacement(&placement_cord);
		uint conflict_size = _routing_engine.routingOnceAndGetConflictSize();

		return static_cast<double>(conflict_size);
	}

	void MixedMapping::initPara()
	{
		uint index = 0;
		const vector<AppNode>& app_nodes = _app_graph.getNodes();
		for_each(begin(app_nodes), end(app_nodes), [&](const AppNode& app_node_)
		{
			BlockType type = app_node_.type;
			_para.nodes_to_place.push_back(index);
			index++;
		});
	}
}
