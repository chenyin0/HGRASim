#pragma once

#include "../util/cord.hpp"
#include "../../lib/tinyxml2/tinyxml2.h"

namespace Simulator
{
	enum class NodeType
	{
		/** real node */
		pe,
		fg,
		ls,
		lv,
		lc,
		/** functional node */
		null,
		begin
		
	};
	//std::ostream& operator<<(std::ostream& os, const enum class NodeType& nodetype)
	//{
	//	static const char* maps[7] = { "pe","fg","ls","lv","lc","null","begin" };
	//	return os << maps[int(nodetype)];
	//}
	using BlockType = NodeType;

	enum class PEOpcode
	{
		null,
		add,
		mul,
		sub,
		div,
		mod,
		lt,
		eq,
		mux,
		mac,
		add1,
		leshift,
		rishift,
		bit_and,
		bit_or,
		bit_xor,
		cos,
		sin,
		smux,
		neq,
		lte,
		bit_not,
		merge,
		hlt,
		hadd
	};

	enum class VecMode
	{
		vect,
		vecr,
		null
	};
	enum class RecallMode
	{
		cached,
		nocache,
		both
	};
	enum class BpType
	{
		ack,
		credit
	};
	enum class stallType
	{
		none,
		inbuffer_stall,
		step_stall
	};
	enum class ControlMode
	{
		data_flow,
		loop_activate,
		calc_activate,
		loop_reset,
		break_pre,
		break_post,
		continue_,
		cb,
		cinvb,
		bind,
		transout,
		trans
	};

	enum class LocalregFrom
	{
		alu,
		in1,
		null
	};

	enum class AluInFrom
	{
		inbuffer,
		lr,
		inport,
		first_loop,
		null
	};

	enum class OutBufferFrom
	{
		alu,
		lr,
		inbuffer,
		null
	};

	enum class InBufferFrom
	{
		alu,
		flr,
		in0,
		in1,
		in2,
		aluin1
		
	};
	enum class CondMode
	{
		in2data,
		cond,
		cb,
		cinvb
	};
	enum class BufferMode
	{
		buffer,
		keep,
		lr,
		lr_out
	};
	enum class InputBypass
	{
		inbuffer,
		bypass,
	};
	enum class OutputFrom
	{
		alu,
		outbuffer,
	};

	enum class FGMode
	{
		null,
		lbegin
	};

	enum class LSMode
	{
		load,
		store_data,
		store_addr,
		dummy,
		null
	};

	enum class BufferSize
	{
		small,
		middle,
		large
	};

	enum class PortType
	{
		d32_b1_v1,
		d32_v1,
		d32_b1_v1_t,
		d32_v1_t,
		d1_v1,
		d1_v1_t,
		bp,
		null
	};

	enum class PortDirection
	{
		in,
		out,
		null
	};
	
	using WireType = PortType;
	using SegmentType = PortType;

	enum class WireNetConfigMode
	{
		way4,
		way8,
		way4hop1,
		way4hop2,
		full_connect,
		multi_connect,
		null
	};

	enum class PortFunction
	{
		normal,
		router,
		bp,
		null
	};

	enum class MappingMethod
	{
		simulate_anneal
	};


	enum class MemAccessMode
	{
		//Idle,  // 当前context，LSE空闲
		temp,  // 存储中间数据,	LSE处于load和store模式都有效！！
		stream,  // 有规律的连续访存, 仅当LSE处于load模式；store模式无效！！
		irregular,  // 无规律的访存，Out of order,；仅当LSE处于load模式；store模式无效！！
		none  // 当LSE处于store模式时，如果不是配为temp，就都配为None！！
	};

	// 以下配置字仅在LSE处于load模式下有效，因为DAE是针对LSE的load操作
	enum class DaeMode
	{
		send_addr,  // send address to SPM in LSE load
		get_data,  // get data from SPM in LSE load
		none  // if LSE not in load mode(namely when LSE in store mode), daeMode should configured as "None"
	};

	enum class BranchMode
	{
		truePath,  // current context is a true path in the branch
		falsePath, // current context is a false path in the branch
		none  // current context is not in a branch
	};
}