#pragma once

#include "../define/global.hpp"
#include "../preprocess/preprocess.h"

namespace Simulator
{
	struct AppLink final
	{
		uint source_node_vec_index;
		uint source_port_index;
		uint target_node_vec_index;
		uint target_port_index;
	};

	struct AppNet final
	{
		uint source_node_vec_index;
		uint source_port_index;
		vector<pair<uint, uint>> targets;

		AppNet(uint source_node_vec_index_, uint source_port_index_, const vector<pair<uint, uint>>& targets_);
		AppNet(uint source_node_vec_index_, uint source_port_index_, vector<pair<uint, uint>>&& targets_);
	};

	struct AppNode final
	{
		BlockType type;
		uint type_num;

		vector<AppLink> pre_node;
		vector<AppLink> next_node;

		AppNode(BlockType type_, uint num_);

		bool operator==(const AppNode& b_) const;
		bool operator<(const AppNode& b_) const;

		explicit operator string() const;
	};

	struct AppSimNode final
	{
		BlockType type;
		uint type_num;
		uint order;
		vector<AppLink> pre_node;
		vector<AppLink> next_node;

		AppSimNode(BlockType type_, uint num_, uint order);

		bool operator==(const AppSimNode& b_) const;
		bool operator<(const AppSimNode& b_) const;

		explicit operator string() const;
	};

	class AppGraph final
	{
	public:
		/** big 6 - customize */
		AppGraph();
		/** big 6 - default */
		~AppGraph() = default;
		/** big 6 - delete */
		AppGraph(const AppGraph& b_) = delete;
		AppGraph(AppGraph&& b_) = delete;
		AppGraph& operator=(const AppGraph& b_) = delete;
		AppGraph& operator=(AppGraph&& b_) = delete;

	public:
		auto generateAllLink() const -> vector<AppLink>;
		auto generateAllNet() const -> vector<AppNet>;

	public:
		/** get-function interface */
		auto getNodes() const -> const vector<AppNode>&;
		auto getSimNodes() const -> const vector<AppNode> &;
		auto getSize() const -> uint;
		auto getManualPlacement() const -> const vector<Cord>&;

	private:
		vector<AppNode> _nodes;
		vector<AppNode> sim_nodes;
		vector<Cord> _manual_placement;

	private:
		auto init() -> void;
	};
}

namespace std
{
	template <>
	struct hash<Simulator::AppLink> final
	{
	public:
		size_t operator()(const Simulator::AppLink& b_) const noexcept
		{
			return hash<uint>()(b_.source_node_vec_index) ^ hash<uint>()(b_.source_port_index)
				^ hash<uint>()(b_.target_node_vec_index) ^ hash<uint>()(b_.target_port_index);
		}
	};
}
