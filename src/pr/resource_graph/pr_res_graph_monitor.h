#pragma once

#include "pr_res_graph_element.h"

namespace Simulator::Mpr
{
	
	class ResourceGraph;

	class RGraphMonitor final
	{
		/**
		 * input a block node
		 * return all this block's in ports' indexes
		 */
		using BlockInPortsQuery = pair<uint, vector<uint>>;


		/**
		 * input a block node
		 * return all this block's out ports' indexes
		 */
		using BlockOutPortsQuery = pair<uint, vector<uint>>;


		/**
		 * a segment can be covered by a rectangular
		 * rectangular can be described by up-left cord and down-right cord
		 */
		using UpLeftCord = Cord;

	public:
		explicit RGraphMonitor(std::function< const vector<Resource*>* (void)> f_);
		~RGraphMonitor() = default;
		RGraphMonitor(const RGraphMonitor& b_) = delete;
		RGraphMonitor(RGraphMonitor&& b_) = delete;
		RGraphMonitor& operator=(const RGraphMonitor& b_) = delete;
		RGraphMonitor& operator=(RGraphMonitor&& b_) = delete;

	public:
		// write interface
		auto setPortIntervalLeft(uint b_) -> void;
		auto setPortIntervalRight(uint b_) -> void;

		auto setBlockIntervalLeft(uint b_) -> void;
		auto setBlockIntervalRight(uint b_) -> void;

		auto setWireIntervalLeft(uint b_) -> void;
		auto setWireIntervalRight(uint b_) -> void;

		auto setSegmentIntervalLeft(uint b_) -> void;
		auto setSegmentIntervalRight(uint b_) -> void;

		auto setRouterIntervalLeft(uint b_) -> void;
		auto setRouterIntervalRight(uint b_) -> void;

		auto segmentUpdate(uint index_, const RNodeSegment& segment_node_) -> void;

		auto blockTypeIndexesUpdate(BlockType block_type_, uint index_) -> void;

		auto blockInPortsQueryUpdate(uint block_id_, uint port_id_) -> void;
		auto blockOutPortsQueryUpdate(uint block_id_, uint port_id_) -> void;

		// read interface
		auto getPortInterval() const->Interval;
		auto getBlockInterval() const->Interval;
		auto getWireInterval() const->Interval;
		auto getSegmentInterval() const->Interval;
		auto getRouterInterval() const->Interval;

		/**
		 * find specific block index
		 *  */
		auto queryBlockIndex(const Cord& block_pos_) const->uint;
		auto queryBlockIndex(PortPos port_pos_) const->uint;

		/**
		 * find all specific type block indexes
		 *  */
		auto queryBlockTypeIndexes(BlockType block_type_) const->vector<uint>;

		/**
		 * find all in port indexes of specific block
		 *  */
		auto queryInPortsOfOneBlock(uint block_index_) const->vector<uint>;

		/**
		 * find all out port indexes of specific block
		 *  */
		auto queryOutPortsOfOneBlock(uint block_index_) const->vector<uint>;

		/**
		 * find specific port
		 *  */
		auto queryPortIndex(const PortPos& port_pos_) const->uint;

		/**
		 * find specific wire
		 *  */
		auto queryWireIndex(PortPos source_port_pos_, PortPos target_port_pos_) const->pair<bool, uint>;

	private:

		/** ResourceGraph's callback function */
		std::function< const vector<Resource*>* (void)> _call_back_get_resources;

		/** record resource type boundary, interval = [x, x) */
		Interval _port_interval;
		Interval _block_interval;
		Interval _wire_interval;
		Interval _segment_interval;
		Interval _router_interval;

		/** record every segment's covered rectangular boundary */
		map<uint, UpLeftCord> _segment_boundary;

		/** record same type block indexes */
		unordered_map<BlockType, vector<uint>> _block_type_indexes;

		// for query
		unordered_map<uint, vector<uint>> _block_in_ports_query;
		unordered_map<uint, vector<uint>> _block_out_ports_query;

	};

}