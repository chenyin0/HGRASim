#include "pr_res_graph_monitor.h"

#ifdef DEBUG_CONSOLE
#include <iostream>
#endif

namespace Simulator::Mpr
{
	RGraphMonitor::RGraphMonitor(std::function<const vector<Resource*>*(void)> f_)
	{
		_call_back_get_resources = f_;
	}

	auto RGraphMonitor::setPortIntervalLeft(uint b_) -> void
	{
		_port_interval.left = b_;
	}

	auto RGraphMonitor::setPortIntervalRight(uint b_) -> void
	{
		_port_interval.right = b_;
	}

	auto RGraphMonitor::setBlockIntervalLeft(uint b_) -> void
	{
		_block_interval.left = b_;
	}

	auto RGraphMonitor::setBlockIntervalRight(uint b_) -> void
	{
		_block_interval.right = b_;
	}

	auto RGraphMonitor::setWireIntervalLeft(uint b_) -> void
	{
		_wire_interval.left = b_;
	}

	auto RGraphMonitor::setWireIntervalRight(uint b_) -> void
	{
		_wire_interval.right = b_;
	}

	auto RGraphMonitor::setSegmentIntervalLeft(uint b_) -> void
	{
		_segment_interval.left = b_;
	}

	auto RGraphMonitor::setSegmentIntervalRight(uint b_) -> void
	{
		_segment_interval.right = b_;
	}

	auto RGraphMonitor::setRouterIntervalLeft(uint b_) -> void
	{
		_router_interval.left = b_;
	}

	auto RGraphMonitor::setRouterIntervalRight(uint b_) -> void
	{
		_router_interval.right = b_;
	}

	void RGraphMonitor::segmentUpdate(uint index_, const RNodeSegment& segment_node_)
	{
		const auto& source_port_pos = segment_node_.source_port_pos;
		const auto& target_port_pos = segment_node_.target_port_pos;

		Cord up_left{ UINT_MAX, UINT_MAX };

		for (auto& source_port : source_port_pos)
		{
			Cord cord = source_port.block_pos;
			if (cord.x < up_left.x)
			{
				up_left = cord;
				continue;
			}
			if (cord.x == up_left.x && cord.y < up_left.y)
			{
				up_left = cord;
				continue;
			}
		}

		for (auto& target_port : target_port_pos)
		{
			Cord cord = target_port.block_pos;
			if (cord.x < up_left.x)
			{
				up_left = cord;
				continue;
			}
			if (cord.x == up_left.x && cord.y < up_left.y)
			{
				up_left = cord;
				continue;
			}
		}

		_segment_boundary[index_] = up_left;
	}

	auto RGraphMonitor::blockTypeIndexesUpdate(BlockType block_type_, uint index_) -> void
	{
		_block_type_indexes[block_type_].push_back(index_);
	}

	void RGraphMonitor::blockInPortsQueryUpdate(uint block_id_, uint port_id_)
	{
		_block_in_ports_query[block_id_].push_back(port_id_);
	}

	void RGraphMonitor::blockOutPortsQueryUpdate(uint block_id_, uint port_id_)
	{
		_block_out_ports_query[block_id_].push_back(port_id_);
	}

	auto RGraphMonitor::getPortInterval() const -> Interval
	{
		return _port_interval;
	}

	auto RGraphMonitor::getBlockInterval() const -> Interval
	{
		return _block_interval;
	}

	auto RGraphMonitor::getWireInterval() const -> Interval
	{
		return _wire_interval;
	}

	auto RGraphMonitor::getSegmentInterval() const -> Interval
	{
		return _segment_interval;
	}

	auto RGraphMonitor::getRouterInterval() const -> Interval
	{
		return _router_interval;
	}

	auto RGraphMonitor::queryBlockIndex(const Cord& block_pos_) const -> uint
	{
		const vector<Resource*>& resources = *_call_back_get_resources();
		for (uint i = _block_interval.left; i < _block_interval.right; i++)
		{
			const RNodeBlock* ptr = dynamic_cast<const RNodeBlock*>(resources[i]);
			if (ptr->pos == block_pos_)
				return i;
		}
		DEBUG_ASSERT(false);
		return UINT_MAX;
	}

	auto RGraphMonitor::queryBlockIndex(PortPos port_pos_) const -> uint
	{
		return queryBlockIndex(port_pos_.block_pos);
	}

	// read

	auto RGraphMonitor::queryBlockTypeIndexes(BlockType block_type_) const -> vector<uint>
	{
		auto itr1 = _block_type_indexes.find(block_type_);
		DEBUG_ASSERT(itr1 != _block_type_indexes.end());
		return itr1->second;
	}

	vector<uint> RGraphMonitor::queryInPortsOfOneBlock(uint block_index_) const
	{
		auto itr = _block_in_ports_query.find(block_index_);
		DEBUG_ASSERT(itr != _block_in_ports_query.end());
		return itr->second;
	}

	vector<uint> RGraphMonitor::queryOutPortsOfOneBlock(uint block_index_) const
	{
		auto itr = _block_out_ports_query.find(block_index_);
		DEBUG_ASSERT(itr != _block_out_ports_query.end());
		return itr->second;
	}

	auto RGraphMonitor::queryPortIndex(const PortPos& port_pos_) const -> uint
	{
		const vector<Resource*>& resources = *_call_back_get_resources();

		Cord block_pos = port_pos_.block_pos;
		uint block_index = queryBlockIndex(block_pos);
		PortDirection direction = port_pos_.direction;
		vector<uint> candidates = direction == PortDirection::in ? queryInPortsOfOneBlock(block_index) : queryOutPortsOfOneBlock(block_index);

		for (auto candidate_index : candidates)
		{
			const RNodePort* ptr = dynamic_cast<const RNodePort*>(resources[candidate_index]);
			if (ptr->port_pos == port_pos_)
				return candidate_index;
		}

		string error_str = "can not find " + static_cast<string>(port_pos_);
		std::cout << error_str << std::endl;
		DEBUG_ASSERT(false);
		return 0;
	}

	auto RGraphMonitor::queryWireIndex(PortPos source_port_pos_, PortPos target_port_pos_) const -> pair<bool, uint>
	{
		const vector<Resource*>& resources = *_call_back_get_resources();
		uint source_port_index = queryPortIndex(source_port_pos_);
		const RNodePort* port_ptr = dynamic_cast<const RNodePort*>(resources[source_port_index]);
		const vector<uint>& next_wires = port_ptr->next_wire_id;
		for (auto& every_wire_index : next_wires)
		{
			if (dynamic_cast<const RNodeWire*>(resources[every_wire_index])->target_port_pos == target_port_pos_)
				return std::make_pair(true, every_wire_index);
		}

		return std::make_pair(false, UINT_MAX);
	}
}