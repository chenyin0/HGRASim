#pragma once

#include "../../resource_graph/pr_res_graph.h"
#include "../../../app_graph/app_graph.h"

namespace Simulator::Mpr
{

	struct RoutingResult
	{
		vector<AppNet> net;
		vector<vector<vector<uint>>> paths;
		double resource_rate = 0;
		double resource_wire_rate = 0;
		double resource_segment_rate = 0;
	};

	class RoutingEngine
	{
	private:

		using Path = vector<uint>;

		struct AppNetRoutingResult
		{
			uint source_node_vec_index;
			uint source_port_index;
			vector<pair<uint, uint>> targets;
			vector<Path> target_paths;

			AppNetRoutingResult(const AppNet& app_net_, const vector<Path>& path_);

			auto clearPaths() -> void;
		};

		struct State
		{
			list<uint> occupy_net;
			double weight;
		};

		class ConflictPool
		{
			using ResNodeIndex = uint;
			using ConflictSize = uint;
		public:
			ConflictPool() = default;

		public:
			auto updateMaxInfo() -> void;

			auto checkAllConflict() const -> bool;

			auto getPoolSize() const -> uint;

			auto getOneMaxConflictResourceIndex() const -> uint;

			auto reduceResource(uint res_index_) -> void;

			auto addResource(uint res_index_) -> void;

			auto getConflictPool() const -> const unordered_map<ResNodeIndex, ConflictSize>&;

			auto clear() -> void;
			
		private:
			unordered_map<ResNodeIndex, ConflictSize> _conflict_pool;
			uint _current_max_conflict;
			unordered_set<int> _conflict_resources;
		};

		class Monitor
		{
		public:
			explicit Monitor(const vector<Resource*>& resource_, const RGraphMonitor& r_graph_monitor_);

		public:
			auto checkAllConflict() const -> bool;

			auto getConflictPoolSize() const->uint;
			auto getNowConflictNet() const -> uint;
			auto popNet(uint net_num_, AppNetRoutingResult& net_result_) -> void;

			auto queryWeight(uint num_) const -> double;
			auto updateWeight() -> void;

			auto queryResourceOccupy(uint num_) const -> bool;
			auto updateState(uint net_num_, AppNetRoutingResult& net_result_) -> void;
			auto updateConflictPool() -> void;

			auto clear() -> void;

		private:
			const RGraphMonitor& _r_graph_monitor;

			vector<State> _state;
			ConflictPool _conflict_pool;
		};

	public:
		explicit RoutingEngine(const ResourceGraph& r_graph_, const AppGraph& app_graph_);

	public:
		auto goRouting() -> void;

		auto routingOnceAndGetConflictSize() -> uint;

		auto loadPlacement(const vector<Cord>* ptr_) -> void;

		auto detachPlacement() -> void;

		auto saveRoutingResult(const string& addr_) -> void;

	private:
		auto initAppNetRoutingResult(const AppGraph& app_graph_) const -> vector<AppNetRoutingResult>;

		auto routingOneNet(uint net_index_) -> void;

		auto dijkstra(uint source_index_, uint target_index_) const -> pair<bool, Path>;

		auto generateRoutingResult() -> void;

	private:
		const vector<Resource*>& _resource;
		const RGraphMonitor& _r_graph_monitor;
		const vector<AppNode>& _app_nodes;

		vector<AppNetRoutingResult> _all_nets;
		const vector<Cord>* _placement;

		Monitor _monitor;

		RoutingResult _result;
	};

}
