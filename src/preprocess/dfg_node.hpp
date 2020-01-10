#pragma once

/**
 * 在DFG节点的设计上使用静态模板泛型+动态多态的模式
 * 模板泛型实现让不同类型体现在<>中，而不是改变类的名字
 * 动态多态实现不同函数的重写
 *
 * 行 2019.10.6
 * 
 */

#include <utility>
#include "../util/cord.hpp"
#include "../../lib/tinyxml2/tinyxml2.h"

namespace Simulator::Preprocess
{
	// interface表示这个类是一个接口，纯虚类，全部函数为纯虚函数
	interface DFGNodeInterface
	{
		// 获取这个节点的所有输入值，每一个值为一个三元tuple体，分别是什么类型，哪一个，哪个端口
		virtual vector<tuple<NodeType, uint, uint>> getAllInput() const = 0;

		// 获取这个DFG节点的手工输入布局位置
		virtual Cord getManualPlacementCord() const = 0;

		virtual ~DFGNodeInterface() = default;
	};

	// 模板基类，不做任何实现，用来特化，在使用中所有模板值都特化掉
	template<NodeType Type>
	struct DFGNode : DFGNodeInterface
	{
	};

	// 输入来源结构体，来自什么类型，第几个，哪一个端口
	struct Input
	{
		NodeType source_node_type;
		uint source_node_index;
		uint source_node_port_index;

		Input(NodeType type_, uint index_, uint port_index_)
			: source_node_type(type_)
			, source_node_index(index_)
			, source_node_port_index(port_index_)
		{
		}
	};

/** PE */
	template<>
	struct DFGNode<NodeType::pe> : DFGNodeInterface
	{
		//uint index;
		//PEOpcode opcode;
		//ControlMode control_mode;
		//LocalregFrom lr_from;
		//vector<AluInFrom> alu_in_from;
		//vector<OutBufferFrom> ob_from;
		//vector<OutputFrom> output_from;
		//vector<Input> input_vec;
		//int reg;
		//Cord manual_placement;
		uint index;
		PEOpcode opcode;
		ControlMode control_mode;
		vector<OutputFrom> output_from;
		vector<Input> input_vec;
		vector<int> reg;
		Cord manual_placement;
		vector<BufferMode> buffer_mode;
		InputBypass input_bypass;
		vector<InBufferFrom> inbuffer_from;
		CondMode cond_mode;
		bool key_cal;
		//DFGNode(uint index_, PEOpcode opcode_, ControlMode control_mode_, LocalregFrom lr_from_,
		//	vector<AluInFrom> alu_in_from_, vector<OutBufferFrom> ob_from_, vector<OutputFrom> output_from_,
		//	vector<Input> input_vec_, int reg_, Cord manual_placement_)
		//	: index(index_)
		//	, opcode(opcode_)
		//	, control_mode(control_mode_)
		//	, lr_from(lr_from_)
		//	, alu_in_from(std::move(alu_in_from_))
		//	, ob_from(std::move(ob_from_))
		//	, output_from(std::move(output_from_))
		//	, input_vec(std::move(input_vec_))
		//	, reg(reg_)
		//	, manual_placement(std::move(manual_placement_))
		//{
		//}
		DFGNode(uint index_, PEOpcode opcode_, ControlMode control_mode_,
		vector<OutputFrom> output_from_,vector<Input> input_vec_, vector<int> reg_vec_, Cord manual_placement_, 
		vector<BufferMode> buffer_mode_, InputBypass input_bypass_, vector<InBufferFrom> inbuffer_from_,bool key_cal_=false)
		: index(index_)
		, opcode(opcode_)
		, control_mode(control_mode_)
		, output_from(std::move(output_from_))
		, input_vec(std::move(input_vec_))
		, reg(reg_vec_)
		, manual_placement(std::move(manual_placement_))
		, buffer_mode(buffer_mode_)
		, input_bypass(input_bypass_)
		, inbuffer_from(inbuffer_from_)
		, key_cal(key_cal_)
		{
		}

		vector<tuple<NodeType, uint, uint>> getAllInput() const override
		{
			vector<tuple<NodeType, uint, uint>> result;
			for (auto[node_type, node_index, port_index] : input_vec)
			{
				result.emplace_back(node_type, node_index, port_index);
			}
			return result;
		}

		Cord getManualPlacementCord() const override
		{
			return manual_placement;
		}
	};

/** FG */
	template<>
	struct DFGNode<NodeType::fg> : DFGNodeInterface
	{
		uint index;
		FGMode mode;
		uint mask;
		uint lut;
		bool tag_mode;
		bool combine_path;
		bool tag_bind;
		vector<Input> input_vec;
		uint reg;
		Cord manual_placement;

		DFGNode(uint index_, FGMode mode_, uint mask_, uint lut_, bool tag_mode_,
			bool combine_path_, bool tag_bind_, vector<Input> input_vec_, uint reg_, Cord manual_placement_)
			: index(index_)
			, mode(mode_)
			, mask(mask_)
			, lut(lut_)
			, tag_mode(tag_mode_)
			, combine_path(combine_path_)
			, tag_bind(tag_bind_)
			, input_vec(std::move(input_vec_))
			, reg(reg_)
			, manual_placement(std::move(manual_placement_))
		{
		}

		vector<tuple<NodeType, uint, uint>> getAllInput() const override
		{
			vector<tuple<NodeType, uint, uint>> result;
			for (auto [node_type, node_index, port_index] : input_vec)
			{
				result.emplace_back(node_type, node_index, port_index);
			}
			return result;
		}

		Cord getManualPlacementCord() const override
		{
			return manual_placement;
		}
	};

/** LS */
	template<>
	struct DFGNode<NodeType::ls> : DFGNodeInterface
	{
		uint index;
		LSMode ls_mode;
		bool tag_bind;
		bool dae;
		bool fifo_mode;
		uint fifo_step;
		BufferSize size;
		bool match;
		vector<Input> input_vec;
		Cord manual_placement;
		VecMode vec_mode;
		DaeMode dae_mode;
		BranchMode branch_mode;
		MemAccessMode mem_access_mode;
		uint pointer;
		uint cluster;
		uint step;
		uint vec_size;

		DFGNode(uint index_, LSMode ls_mode_, bool tag_bind_, bool dae_, uint fifo_step_, BufferSize size_,
			bool match_, vector<Input> input_vec_, Cord manual_placement_,MemAccessMode mem_access_mode_,
			DaeMode dae_mode_, BranchMode branch_mode_,uint cluster_, VecMode vec_mode_ = VecMode::null,
			uint pointer_=0, uint step_ = 0, uint thesize_=0)
			: index(index_)
			, ls_mode(ls_mode_)
			, tag_bind(tag_bind_)
			, dae(dae_)
			, fifo_step(fifo_step_)
			, size(size_)
			, match(match_)
			, input_vec(input_vec_)
			, manual_placement(std::move(manual_placement_))
			,vec_mode(vec_mode_)
			,pointer(pointer_)
			,step(step_)
			,vec_size(thesize_)
			,dae_mode(dae_mode_)
			,mem_access_mode(mem_access_mode_)
			,branch_mode(branch_mode_)
			,cluster(cluster_)
		{
		}

		vector<tuple<NodeType, uint, uint>> getAllInput() const override
		{
			vector<tuple<NodeType, uint, uint>> result;
			for (auto[node_type, node_index, port_index] : input_vec)
			{
				result.emplace_back(node_type, node_index, port_index);
			}
			return result;
		}

		Cord getManualPlacementCord() const override
		{
			return manual_placement;
		}
	};

/** LC */
	template<>
	struct DFGNode<NodeType::lc> : DFGNodeInterface
	{
		uint index;
		bool outermost;
		vector<Input> input_vec;
		vector<int> reg;
		Cord manual_placement;

		DFGNode(uint index_, bool outermost_, vector<Input> input_vec_, vector<int> reg_, Cord manual_placement_)
			: index(index_)
			, outermost(outermost_)
			, input_vec(std::move(input_vec_))
			, reg(std::move(reg_))
			, manual_placement(std::move(manual_placement_))
		{
		}

		vector<tuple<NodeType, uint, uint>> getAllInput() const override
		{
			vector<tuple<NodeType, uint, uint>> result;
			for (auto[node_type, node_index, port_index] : input_vec)
			{
				result.emplace_back(node_type, node_index, port_index);
			}
			return result;
		}

		Cord getManualPlacementCord() const override
		{
			return manual_placement;
		}
	};
	class ClusterInterface
	{
	public:
		NodeType nodetype;
		uint id;
		uint start_node;
		uint step;
		uint end_node;
		uint counter;
		vector<Input> input_vec;
	//	bool update_en;
	public:
		friend class DFG;
		ClusterInterface(NodeType nodetype_,uint id_, uint start_node_, uint step_, uint end_node_, uint counter_, vector<Input> input_vec_)
			: nodetype(nodetype_)
			, id(id_)
			, start_node(start_node_)
			, step(step_)
			, end_node(end_node_)
			, counter(counter_)
			, input_vec(input_vec_)
		{
		}
	};
	class ClusterGroupInterface
	{
	private:
		unordered_map<std::pair<NodeType, uint>, shared_ptr<ClusterInterface>> clusters;
		unordered_map<std::pair<NodeType, uint>, uint> ls_cluster;
	public:
		friend class DFG;
		auto getAllClusters() const->const unordered_map<std::pair<NodeType, uint>, shared_ptr<ClusterInterface>> &
		{
			return clusters;
		}
		auto getClusterID() const->const unordered_map<std::pair<NodeType, uint>, uint> &
		{
			return ls_cluster;
		}
	};
}