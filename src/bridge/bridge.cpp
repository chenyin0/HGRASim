#include "bridge.h"

namespace Simulator
{
	void Bridge::BpCallBackFuns::registerFun(NodeType type_, NodeIndex node_index_, std::function<void(PortIndex, bool)> fun_)
	{
		_funs[type_][node_index_] = fun_;
	}

	void Bridge::BpCallBackFuns::operator()(NodeType type_, NodeIndex node_index_, PortIndex port_index_, bool bp_value_)
	{
		_funs[type_][node_index_](port_index_, bp_value_);
	}

	void Bridge::InputCallBackFuns::registerFun(NodeType type_, NodeIndex node_index_, std::function<void(PortIndex, Array::Port_inout)> fun_)
	{
		_funs[type_][node_index_] = fun_;
	}

	void Bridge::InputCallBackFuns::operator()(NodeType type_, NodeIndex node_index_, PortIndex port_index_, Array::Port_inout value_)
	{
		_funs[type_][node_index_](port_index_, value_);
	}

	Bridge::Location::Location(NodeType type_, uint node_index_, uint port_index_)
		: type(type_)
		, node_index(node_index_)
		, port_index(port_index_)
	{
	}

	bool Bridge::Location::operator==(const Location& b_) const
	{
		return type == b_.type && node_index == b_.node_index && port_index == b_.port_index;
	}

	std::size_t Bridge::LocationHash::operator()(const Location& b_) const
	{
		return std::hash<string>()(Simulator::NodeTypeConverter::toString(b_.type))
			^ std::hash<uint>()(b_.node_index)
			^ std::hash<uint>()(b_.port_index);
	}

	Bridge::Bridge(const AppGraph& app_graph_)
		: _app_graph(app_graph_)
	{
		initTable();
	}

	void Bridge::setNextInput(Array::Port_inout output_, NodeType type_, uint type_index_, uint port_index_)
	{
		Location self_location{ type_, type_index_, port_index_ };

		if (_next_table.find(self_location) == _next_table.end())
			return;

		vector<Location> target_locations = _next_table[self_location];
		for (auto& every_target_location : target_locations)
		{
			_input_funs(every_target_location.type, every_target_location.node_index, every_target_location.port_index, output_);
		}
	}

	void Bridge::setBp(bool bp_, NodeType type_, uint type_index_, uint port_index_)
	{
		Location self_location{ type_, type_index_, port_index_ };
		if (_pre_table.find(self_location) != _pre_table.end()) {
			Location source_location = _pre_table[self_location];
			// TODO bp机制有些bug -- 汪翔
			//if (_next_table.find(source_location) != _next_table.end())
			//	_bp_temporary_buffer[source_location] = _bp_temporary_buffer[source_location] && bp_;/
			//else
			//	_bp_temporary_buffer[source_location] = bp_;
			if (_next_table.find(source_location) != _next_table.end())
				_bp_temporary_buffer[source_location] = _bp_temporary_buffer[source_location] && bp_;
			else
				_bp_temporary_buffer[source_location] = true;

			if (bp_ == false) {
				stall_note[source_location].push_back(self_location);
			}
		}
	}

	vector<Simulator::Bridge::Location> Bridge::findNextNode(NodeType type_, uint type_index_, uint port_index_) {
		Location self_location{ type_, type_index_, port_index_};
		if (_next_table.find(self_location) != _next_table.end()) {
			return _next_table[self_location];
		}
		else {
			return vector<Simulator::Bridge::Location>();
		}
	}

	Simulator::Bridge::Location Bridge::findPreNode(NodeType type_, uint type_index_, uint port_index_) {
		Simulator::Bridge::Location self_location{ type_, type_index_, port_index_ };
		if (_pre_table.find(self_location) != _pre_table.end()) {
			return _pre_table[self_location];
		}
		else {
			return Location();
		}
	}

	void Bridge::InitBp()
	{
		auto map_it = _next_table.begin();
		while (map_it != _next_table.end()) {
			_bp_temporary_buffer[map_it->first] = true;
			++map_it;
		}
	}

	void Bridge::setAllBp()
	{
		for (auto[location, value] : _bp_temporary_buffer)
			_bp_funs(location.type, location.node_index, location.port_index, value);
		_bp_temporary_buffer.clear();
	}

	void Bridge::recvOneBp(Location location,bool value)
	{
		_bp_funs(location.type, location.node_index, location.port_index, value);
	}

	void Bridge::bpFunRegister(NodeType type_, NodeIndex node_index_, std::function<void(uint, bool)> fun_)
	{
		_bp_funs.registerFun(type_, node_index_, fun_);
	}

	void Bridge::inputFunRegister(NodeType type_, NodeIndex node_index_, std::function<void(uint, Array::Port_inout)> fun_)
	{
		_input_funs.registerFun(type_, node_index_, fun_);
	}

	auto Bridge::initTable() -> void
	{
		//const vector<AppNode>& app_nodes = _app_graph.getNodes();

		//for (auto& every_node : app_nodes)
		//{
		//	// set next table
		//	for (auto& every_next : every_node.next_node)
		//	{
		//		Location self_location{ every_node.type, every_node.type_num, every_next.source_port_index };

		//		uint target_vec_index = every_next.target_node_vec_index;
		//		uint target_port_index = every_next.target_port_index;

		//		_next_table[self_location].emplace_back
		//		(
		//			app_nodes[target_vec_index].type,
		//			app_nodes[target_vec_index].type_num, ///////////加上app_nodes之后有问题
		//			target_port_index
		//		);
		//	}

		//	// set per table
		//	for (auto& every_pre : every_node.pre_node)
		//	{
		//		Location self_location{ every_node.type, every_node.type_num, every_pre.target_port_index };

		//		uint source_vec_index = every_pre.source_node_vec_index;
		//		uint source_port_index = every_pre.source_port_index;

		//		_pre_table[self_location] = Location
		//		{ 
		//			app_nodes[source_vec_index].type,
		//			app_nodes[source_vec_index].type_num,
		//			source_port_index 
		//		};
		//	}
		//}
		
		const vector<AppNode>& app_nodes = _app_graph.getSimNodes();
		for (auto& every_node : app_nodes)
		{
			// set next table
			for (auto& every_next : every_node.next_node)
			{
				Location self_location{ every_node.type, every_node.type_num, every_next.source_port_index };

				uint target_vec_index = every_next.target_node_vec_index;
				uint target_port_index = every_next.target_port_index;

//				uint target_vec_index = Util::findIndex(app_nodes,every_next.target_node_vec_index);
//				uint target_port_index = Util::findIndex(app_nodes, every_next.target_port_index);

				_next_table[self_location].emplace_back
				(
					app_nodes[target_vec_index].type,
					app_nodes[target_vec_index].type_num, ///////////加上app_nodes之后有问题,所以这里只是把getNodes换成了getSimNodes//////////
					target_port_index
				);
			}

			// set per table
			for (auto& every_pre : every_node.pre_node)
			{
				Location self_location{ every_node.type, every_node.type_num, every_pre.target_port_index };

				uint source_vec_index = every_pre.source_node_vec_index;
				uint source_port_index = every_pre.source_port_index;

				_pre_table[self_location] = Location
				{ 
					app_nodes[source_vec_index].type,
					app_nodes[source_vec_index].type_num,
					source_port_index 
				};
			}
		}
	}
}
