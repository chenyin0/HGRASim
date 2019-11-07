#include "routing_engine.h"
#ifdef DEBUG_CONSOLE
#include <iostream>
#include <iomanip>
#endif

namespace Simulator::Mpr
{
	RoutingEngine::AppNetRoutingResult::AppNetRoutingResult(const AppNet& app_net_, const vector<Path>& path_)
		: source_node_vec_index(app_net_.source_node_vec_index)
		, source_port_index(app_net_.source_port_index)
		, targets(app_net_.targets)
		, target_paths(path_)
	{
	}

	auto RoutingEngine::AppNetRoutingResult::clearPaths() -> void
	{
		for (auto& target_path : target_paths)
			target_path.clear();
	}

	auto RoutingEngine::ConflictPool::updateMaxInfo() -> void
	{
		_current_max_conflict = 0;
		_conflict_resources.clear();
		for (auto [resource_index, conflict_size] : _conflict_pool)
		{
			if (conflict_size == _current_max_conflict)
				_conflict_resources.insert(resource_index);
			if (conflict_size > _current_max_conflict)
			{
				_current_max_conflict = conflict_size;
				_conflict_resources.clear();
				_conflict_resources.insert(resource_index);
			}
		}
	}

	auto RoutingEngine::ConflictPool::checkAllConflict() const -> bool
	{
		return _conflict_pool.empty();
	}

	auto RoutingEngine::ConflictPool::getPoolSize() const -> uint
	{
		return _conflict_pool.size();
	}

	auto RoutingEngine::ConflictPool::getOneMaxConflictResourceIndex() const -> uint
	{
		uint random_num = Util::uRandom(0, _conflict_resources.size() - 1);
		auto itr = _conflict_resources.begin();
		while (random_num--)
			++itr;
		return *itr;
	}

	auto RoutingEngine::ConflictPool::reduceResource(uint res_index_) -> void
	{
		auto itr = _conflict_pool.find(res_index_);
		if (itr == _conflict_pool.end())
			return;
		
		if (itr->second == 2)
		{
			_conflict_pool.erase(itr);
		}
		else
			itr->second--;
	}

	auto RoutingEngine::ConflictPool::addResource(uint res_index_) -> void
	{
		auto itr = _conflict_pool.find(res_index_);
		if (itr == _conflict_pool.end())
			_conflict_pool[res_index_] = 2;
		else
			++_conflict_pool[res_index_];
	}

	auto RoutingEngine::ConflictPool::getConflictPool() const -> const unordered_map<ResNodeIndex, ConflictSize>&
	{
		return _conflict_pool;
	}

	auto RoutingEngine::ConflictPool::clear() -> void
	{
		_conflict_pool.clear();
		_current_max_conflict = 0;
		_conflict_resources.clear();
	}

	RoutingEngine::Monitor::Monitor(const vector<Resource*>& resource_, const RGraphMonitor& r_graph_monitor_)
		: _r_graph_monitor(r_graph_monitor_)
		, _state(vector<State>(resource_.size(), State{ list<uint>{}, 1 }))
		, _conflict_pool(ConflictPool{})
	{
	}

	bool RoutingEngine::Monitor::checkAllConflict() const
	{
		return _conflict_pool.checkAllConflict();
	}

	auto RoutingEngine::Monitor::getConflictPoolSize() const -> uint
	{
		return _conflict_pool.getPoolSize();
	}

	auto RoutingEngine::Monitor::getNowConflictNet() const -> uint
	{
		uint conflict_res_index = _conflict_pool.getOneMaxConflictResourceIndex();
		return _state[conflict_res_index].occupy_net.front();
	}

	auto RoutingEngine::Monitor::popNet(uint net_num_, AppNetRoutingResult& net_result_) -> void
	{
		Path all_path;
		for (auto& path : net_result_.target_paths)
			all_path.insert(all_path.end(), path.begin(), path.end());
		Util::eraseDuplicate(all_path);
		for (auto node_index : all_path)
		{
			list<uint>& res_list = _state[node_index].occupy_net;
			auto itr = std::find(res_list.begin(), res_list.end(), net_num_);
			res_list.erase(itr);

			_conflict_pool.reduceResource(node_index);
		}
	}

	double RoutingEngine::Monitor::queryWeight(uint num_) const
	{
		return _state[num_].weight;
	}

	auto RoutingEngine::Monitor::updateWeight() -> void
	{
		const auto& pool = _conflict_pool.getConflictPool();
		for (auto[res_index, conflict_size] : pool)
		{
			if (conflict_size == 1)
				continue;
			DEBUG_ASSERT(conflict_size < 0xfffff000)
			_state[res_index].weight += conflict_size;
		}
			
	}

	auto RoutingEngine::Monitor::queryResourceOccupy(uint num_) const -> bool
	{
		return !_state[num_].occupy_net.empty();
	}

	auto RoutingEngine::Monitor::updateState(uint net_num_, AppNetRoutingResult& net_result_) -> void
	{
		Path all_path;
		for (auto& path : net_result_.target_paths)
			all_path.insert(all_path.end(), path.begin(), path.end());
		Util::eraseDuplicate(all_path);
		
		for (auto every_res_index : all_path)
		{
			_state[every_res_index].occupy_net.push_back(net_num_);
			if (_state[every_res_index].occupy_net.size() >= 2)
				_conflict_pool.addResource(every_res_index);
		}
	}

	auto RoutingEngine::Monitor::updateConflictPool() -> void
	{
		_conflict_pool.updateMaxInfo();
	}

	auto RoutingEngine::Monitor::clear() -> void
	{
		for (auto& every_state : _state)
		{
			every_state.occupy_net.clear();
			every_state.weight = 1;
		}
		_conflict_pool.clear();
	}

	RoutingEngine::RoutingEngine(const ResourceGraph& r_graph_, const AppGraph& app_graph_)
		: _resource(*r_graph_.getResources())
		, _r_graph_monitor(*r_graph_.getMonitor())
		, _app_nodes(app_graph_.getNodes())
		, _all_nets(initAppNetRoutingResult(app_graph_))
		, _placement(nullptr)
		, _monitor(Monitor{ _resource, _r_graph_monitor })
	{
	}

	auto RoutingEngine::goRouting() -> void
	{
		_monitor.clear();
		// init routing
		for (uint net_index = 0; net_index < _all_nets.size(); net_index++)
			routingOneNet(net_index);

		// main loop
		while (!_monitor.checkAllConflict())
		{
			_monitor.updateConflictPool();
			_monitor.updateWeight();

			uint net_index = _monitor.getNowConflictNet();
			_monitor.popNet(net_index, _all_nets[net_index]);
			_all_nets[net_index].clearPaths();

			routingOneNet(net_index);

#ifdef DEBUG_CONSOLE
			std::cout << "Current Conflict Net Index: " << std::setw(4) << net_index << "       ";
			std::cout << "Current Conflict Pool Size: " << std::setw(6) << _monitor.getConflictPoolSize() << std::endl;
#endif
		}

		// generate result
		generateRoutingResult();
	}

	auto RoutingEngine::routingOnceAndGetConflictSize() -> uint
	{
		_monitor.clear();

		for (uint net_index = 0; net_index < _all_nets.size(); net_index++)
			routingOneNet(net_index);

		_monitor.updateConflictPool();

		return _monitor.getConflictPoolSize();
	}

	auto RoutingEngine::loadPlacement(const vector<Cord>* ptr_) -> void
	{
		_placement = ptr_;
	}

	auto RoutingEngine::detachPlacement() -> void
	{
		_placement = nullptr;
	}

	auto RoutingEngine::saveRoutingResult(const string& addr_) -> void
	{
		const char* declaration = R"(<?xml version="1.0" encoding="UTF-8" standalone="no"?>)";
		XMLDocument doc;
		doc.Parse(declaration);

		XMLElement* root = doc.NewElement("RoutingResult");
		doc.InsertEndChild(root);

		// wire rate
		XMLElement* wire_rate_xml = doc.NewElement("wire_rate");
		wire_rate_xml->SetAttribute("rate", std::to_string(_result.resource_wire_rate).c_str());
		root->InsertEndChild(wire_rate_xml);

		// segment rate
		XMLElement* segment_rate_xml = doc.NewElement("segment_rate");
		segment_rate_xml->SetAttribute("rate", std::to_string(_result.resource_segment_rate).c_str());
		root->InsertEndChild(segment_rate_xml);

		// total rate
		XMLElement* resource_rate_xml = doc.NewElement("total_rate");
		resource_rate_xml->SetAttribute("rate", std::to_string(_result.resource_rate).c_str());
		root->InsertEndChild(resource_rate_xml);

		// all paths
		for (uint net_index = 0; net_index < _result.net.size(); net_index++)
		{
			// source info
			const AppNet& temp_net = _result.net[net_index];
			const vector<vector<uint>>& temp_paths = _result.paths[net_index];

			uint source_node_vec_index = temp_net.source_node_vec_index;
			uint source_port_index = temp_net.source_port_index;
			const AppNode& source_node = _app_nodes[source_node_vec_index];
			Cord source_placement = (*_placement)[source_node_vec_index];

			XMLElement* net_xml = doc.NewElement("net");
			string source_node_str = BlockTypeConverter::toString(source_node.type) + std::to_string(source_node.type_num);
			net_xml->SetAttribute("source_node", source_node_str.c_str());
			net_xml->SetAttribute("source_port", std::to_string(source_port_index).c_str());
			net_xml->SetAttribute("placement", static_cast<string>(source_placement).c_str());

			root->InsertEndChild(net_xml);

			for (uint target_index = 0; target_index < temp_net.targets.size(); target_index++)
			{
				// target info
				uint target_node_vec_index = temp_net.targets[target_index].first;
				uint target_port_index = temp_net.targets[target_index].second;
				const AppNode& target_node = _app_nodes[target_node_vec_index];
				Cord target_placement = (*_placement)[target_node_vec_index];

				XMLElement* target_xml = doc.NewElement("target");
				string target_node_str = BlockTypeConverter::toString(target_node.type) + std::to_string(target_node.type_num);
				target_xml->SetAttribute("target_node", target_node_str.c_str());
				target_xml->SetAttribute("target_port", std::to_string(target_port_index).c_str());
				target_xml->SetAttribute("placement", static_cast<string>(target_placement).c_str());

				net_xml->InsertEndChild(target_xml);

				// path
				for (uint i = 0; i < temp_paths[target_index].size(); i++)
				{
					uint resource_index = temp_paths[target_index][i];
					XMLElement* path_xml = doc.NewElement("path");
					path_xml->SetAttribute("id", std::to_string(resource_index).c_str());
					_resource[resource_index]->generateXmlInfoBrief(doc, path_xml);
					target_xml->InsertEndChild(path_xml);
				}
			}
		}

		doc.SaveFile(addr_.c_str());
	}

	auto RoutingEngine::initAppNetRoutingResult(const AppGraph& app_graph_) const -> vector<AppNetRoutingResult>
	{
		vector<AppNet> all_nets = app_graph_.generateAllNet();
		vector<AppNetRoutingResult> result;
		for (auto& net : all_nets)
			result.emplace_back(net, vector<Path>{});

		return result;
	}

	auto RoutingEngine::routingOneNet(uint net_index_) -> void
	{
		AppNetRoutingResult& net = _all_nets[net_index_];
		// find source port resource index
		uint source_node_index = net.source_node_vec_index;
		uint source_port_index = net.source_port_index;
		BlockType source_block_type = _app_nodes[source_node_index].type;
		Cord source_block_cord = (*_placement)[source_node_index];
		PortPos source_port_pos = PortPos{ source_block_type,
			source_block_cord, PortDirection::out,
			PortFunction::normal, source_port_index };
		uint source_port_res_index = _r_graph_monitor.queryPortIndex(source_port_pos);

		// for every target port, find a path
		net.target_paths.resize(net.targets.size());
		for (uint i = 0; i < net.targets.size(); i++)
		{
			// find target port resource index
			uint target_node_index = net.targets[i].first;
			uint target_port_index = net.targets[i].second;
			BlockType target_block_type = _app_nodes[target_node_index].type;
			Cord target_block_cord = (*_placement)[target_node_index];
			PortPos target_port_pos = PortPos{ target_block_type,
				target_block_cord, PortDirection::in,
				PortFunction::normal, target_port_index };
			uint target_port_res_index = _r_graph_monitor.queryPortIndex(target_port_pos);

			// find path
			auto[is_success, dijkstra_result] = dijkstra(source_port_res_index, target_port_res_index);
			
			if (!is_success)
			{
				string error_info = "There is no path between resource id " + std::to_string(source_port_res_index) + " and " + std::to_string(target_port_res_index);
#ifdef DEBUG_CONSOLE
				std::cout << error_info << std::endl;
#endif
				throw error_info;
			}
			
			std::swap(net.target_paths[i], dijkstra_result);
		}

		// record state
		_monitor.updateState(net_index_, net);
	}

	auto RoutingEngine::dijkstra(uint source_index_, uint target_index_) const -> pair<bool, Path>
	{
		vector<Bool> visit(_resource.size(), false);
		vector<uint> dist(_resource.size(), UINT_MAX);
		vector<uint> path(_resource.size(), UINT_MAX);

		struct ResElement
		{
			uint res_num;
			uint dist;

			bool operator<(const ResElement& b_) const
			{
				return dist > b_.dist;
			}
		};

		std::priority_queue<ResElement> q;

		// init : close all block node 
		for (uint i = 0; i < _resource.size(); i++)
			if (typeid(*_resource[i]) == typeid(RNodeBlock))
				visit[i] = true;

		dist[source_index_] = 0;
		q.push(ResElement{ source_index_, 0 });

		// search
		bool is_success = false;
		while (!q.empty())
		{
			ResElement this_ele = q.top();
			q.pop();

			visit[this_ele.res_num] = true;

			if (this_ele.res_num == target_index_)
			{
				is_success = true;
				break;
			}

			vector<uint> next_indexes = _resource[this_ele.res_num]->getAllNextId();
			for (auto every_next : next_indexes)
			{
				if (visit[every_next])
					continue;

				uint weight = _monitor.queryWeight(every_next);
				if (dist[this_ele.res_num] + weight < dist[every_next])
				{
					dist[every_next] = dist[this_ele.res_num] + weight;
					path[every_next] = this_ele.res_num;
				}

				q.push(ResElement{ every_next, dist[every_next] });
			}
		}

		// deal result
		if (!is_success)
			return std::make_pair(false, vector<uint>{});

		stack<uint> s;
		uint temp = target_index_;
		while (temp != source_index_)
		{
			s.push(temp);
			temp = path[temp];
		}
		s.push(source_index_);

		// make path
		Path result_path;
		while (!s.empty())
		{
			result_path.push_back(s.top());
			s.pop();
		}

		return std::make_pair(true, result_path);
	}

	auto RoutingEngine::generateRoutingResult() -> void
	{
		// net and path
		for (auto& net_result : _all_nets)
		{
			_result.net.emplace_back(net_result.source_node_vec_index, net_result.source_port_index, net_result.targets);
			_result.paths.push_back(net_result.target_paths);
		}

		// wire rate
		uint wire_count = 0;
		uint wire_used_count = 0;
		Interval wire_interval = _r_graph_monitor.getWireInterval();
		for (uint i = wire_interval.left; i < wire_interval.right; i++)
		{
			wire_count++;
			if (_monitor.queryResourceOccupy(i))
				wire_used_count++;
		}
		_result.resource_wire_rate = static_cast<double>(wire_used_count) / static_cast<double>(wire_count);

		// segment rate
		uint segment_count = 0;
		uint segment_used_count = 0;
		Interval segment_interval = _r_graph_monitor.getSegmentInterval();
		for (uint i = segment_interval.left; i < segment_interval.right; i++)
		{
			segment_count++;
			if (_monitor.queryResourceOccupy(i))
				segment_used_count++;
		}
		_result.resource_segment_rate = static_cast<double>(segment_used_count) / static_cast<double>(segment_count);

		// total rate
		uint total_resource = wire_count + segment_count;
		uint total_resource_used = wire_used_count + segment_used_count;
		_result.resource_rate = static_cast<double>(total_resource_used) / static_cast<double>(total_resource);
	}
}
