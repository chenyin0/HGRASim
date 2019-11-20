#pragma once

#include "enum_definition.hpp"
#include <stdexcept>
#include <cstdio>

namespace Simulator
{
	class NodeTypeConverter
	{
	public:
		static auto toString(NodeType type_)->string
		{
			if (type_ == NodeType::pe)
				return "pe";
			if (type_ == NodeType::fg)
				return "fg";
			if (type_ == NodeType::ls)
				return "ls";
			if (type_ == NodeType::lv)
				return "lv";
			if (type_ == NodeType::lc)
				return "lc";
			if (type_ == NodeType::null)
				return "null";
			if (type_ == NodeType::begin)
				return "begin";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->NodeType
		{
			if (s_ == "pe")
				return NodeType::pe;
			if (s_ == "fg")
				return NodeType::fg;
			if (s_ == "ls")
				return NodeType::ls;
			if (s_ == "lv")
				return NodeType::lv;
			if (s_ == "lc")
				return NodeType::lc;
			if (s_ == "null")
				return NodeType::null;
			if (s_ == "begin")
				return NodeType::begin;
			throw std::runtime_error("your configuration is boom(nodetype)");
			DEBUG_ASSERT(false);
			return NodeType::null;
		}

		static auto isRealNode(NodeType type_) -> bool
		{
			return !(type_ == NodeType::begin || type_ == NodeType::null);
		}
	};

	using BlockTypeConverter = NodeTypeConverter;
	class InputBypassConverter 
	{
	public:
		static auto toString(InputBypass type_)->string
		{
			if (type_ == InputBypass::inbuffer)
				return "inbuffer";
			if (type_ == InputBypass::bypass)
				return "bypass";
			DEBUG_ASSERT(false);
			return "";
		}
		static auto toEnum(string s_)->InputBypass
		{
			if (s_ == "inbuffer")
				return InputBypass::inbuffer;
			if (s_ == "bypass")
				return InputBypass::bypass;
			throw std::runtime_error("your configuration is boom(input_bypass)");
			DEBUG_ASSERT(false);
			return InputBypass::inbuffer;
		}
	};
	class InBufferFromConverter {
	public:
		static auto toString(InBufferFrom type_)->string
		{
			if (type_ == InBufferFrom::alu)
				return "alu";
			if (type_ == InBufferFrom::flr)
				return "null";
			if (type_ == InBufferFrom::in0)
				return "in0";
			if (type_ == InBufferFrom::in1)
				return "in1";
			if (type_ == InBufferFrom::in2)
				return "in2";
			if (type_ == InBufferFrom::aluin1)
				return "aluin1";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->InBufferFrom
		{
			if (s_ == "alu")
				return InBufferFrom::alu;
			if (s_ == "null")
				return InBufferFrom::flr;
			if (s_ == "in0")
				return InBufferFrom::in0;
			if (s_ == "in1")
				return InBufferFrom::in1;
			if (s_ == "in2")
				return InBufferFrom::in2;
			if (s_ == "aluin1")
				return InBufferFrom::aluin1;
			throw std::runtime_error("your configuration is boom(inbufferfrom)");
			DEBUG_ASSERT(false);
			return InBufferFrom::flr;
		}
	};

	class CondConverter {
	public:
		static auto toString(CondMode type_)->string
		{
			if (type_ == CondMode::cb)
				return "cb";
			if (type_ == CondMode::cinvb)
				return "cinvb";
			if (type_ == CondMode::cond)
				return "cond";
			if (type_ == CondMode::in2data)
				return "in2data";
			DEBUG_ASSERT(false);
		}
		static auto toEnum(string s_)->CondMode
		{
			if (s_ == "cb")
				return CondMode::cb;
			if (s_ == "cinvb")
				return CondMode::cinvb;
			if (s_ == "cond")
				return CondMode::cond;
			if (s_ == "in2data")
				return CondMode::in2data;
			throw std::runtime_error("your configuration is boom(condmode)");
			DEBUG_ASSERT(false);
		}
	};
	class BufferModeConverter {
	public:
		static auto toString(BufferMode type_)->string
		{
			if (type_ == BufferMode::buffer)
				return "buffer";
			if (type_ == BufferMode::keep)
				return "keep";
			if (type_ == BufferMode::lr_out)
				return "lr_out";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->BufferMode
		{
			if (s_ == "buffer")
				return BufferMode::buffer;
			if (s_ == "keep")
				return BufferMode::keep;
			if (s_ == "lr")
				return BufferMode::lr;
			if (s_ == "lr_out")
				return BufferMode::lr_out;
			throw std::runtime_error("your configuration is boom(buffer_mode)");
			DEBUG_ASSERT(false);
			return BufferMode::buffer;
		}
	};
	class FGModeConverter
	{
	public:
		static auto toString(FGMode type_)->string
		{
			if (type_ == FGMode::lbegin)
				return "lbegin";
			if (type_ == FGMode::null)
				return "null";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->FGMode
		{
			if (s_ == "null")
				return FGMode::null;
			if (s_ == "lbegin")
				return FGMode::lbegin;
			throw std::runtime_error("your configuration is boom(fgmode)");
			DEBUG_ASSERT(false);
			return FGMode::null;
		}
	};

	class PEOpcodeConverter
	{
	public:
		static auto toString(PEOpcode type_)->string
		{
			if (type_ == PEOpcode::null)
				return "nop";
			if (type_ == PEOpcode::hadd)
				return "hadd";
			if (type_ == PEOpcode::hlt)
				return "hlt";
			if (type_ == PEOpcode::add)
				return "add";
			if (type_ == PEOpcode::mul)
				return "mul";
			if (type_ == PEOpcode::sub)
				return "sub";
			if (type_ == PEOpcode::div)
				return "div";
			if (type_ == PEOpcode::mod)
				return "mod";
			if (type_ == PEOpcode::lt)
				return "lt";
			if (type_ == PEOpcode::eq)
				return "eq";
			if (type_ == PEOpcode::mux)
				return "mux";
			if (type_ == PEOpcode::mac)
				return "mac";
			if (type_ == PEOpcode::add1)
				return "add1";
			if (type_ == PEOpcode::leshift)
				return "leshift";
			if (type_ == PEOpcode::rishift)
				return "rishift";
			if (type_ == PEOpcode::bit_and)
				return "and";
			if (type_ == PEOpcode::bit_or)
				return "or";
			if (type_ == PEOpcode::cos)
				return "cos";
			if (type_ == PEOpcode::sin)
				return "sin";
			if (type_ == PEOpcode::bit_xor)
				return "xor";
			if (type_ == PEOpcode::smux)
				return "smux";
			if (type_ == PEOpcode::neq)
				return "neq";
			if (type_ == PEOpcode::bit_not)
				return "not";
			if (type_ == PEOpcode::merge)
				return "merge";
			if (type_ == PEOpcode::lte)
				return "lte";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->PEOpcode
		{
			if (s_ == "nop")
				return PEOpcode::null;
			if (s_ == "add")
				return PEOpcode::add;
			if (s_ == "mul")
				return PEOpcode::mul;
			if (s_ == "sub")
				return PEOpcode::sub;
			if (s_ == "div")
				return PEOpcode::div;
			if (s_ == "mod")
				return PEOpcode::mod;
			if (s_ == "lt")
				return PEOpcode::lt;
			if (s_ == "eq")
				return PEOpcode::eq;
			if (s_ == "mux")
				return PEOpcode::mux;
			if (s_ == "mac")
				return PEOpcode::mac;
			if (s_ == "add1")
				return PEOpcode::add1;
			if (s_ == "hadd")
				return PEOpcode::hadd;
			if (s_ == "hlt")
				return PEOpcode::hlt;
			if (s_ == "leshift")
				return PEOpcode::leshift;
			if (s_ == "rishift")
				return PEOpcode::rishift;
			if (s_ == "and")
				return PEOpcode::bit_and;
			if (s_ == "or")
				return PEOpcode::bit_or;
			if (s_ == "cos")
				return PEOpcode::cos;
			if (s_ == "sin")
				return PEOpcode::sin;
			if (s_ == "xor")
				return PEOpcode::bit_xor;
			if (s_ == "smux")
				return PEOpcode::smux;
			if (s_ == "neq")
				return PEOpcode::neq;
			if (s_ == "not")
				return PEOpcode::bit_not;
			if (s_ == "merge")
				return PEOpcode::merge;
			if (s_ == "lte")
				return PEOpcode::lte;
			throw std::runtime_error("your configuration is boom(opcode)");
			DEBUG_ASSERT(false);
			return PEOpcode::null;
		}
	};

	class LocalregFromConverter
	{
	public:
		static auto toString(LocalregFrom type_)->string
		{
			if (type_ == LocalregFrom::null)
				return "null";
			if (type_ == LocalregFrom::alu)
				return "alu";
			if (type_ == LocalregFrom::in1)
				return "in1";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->LocalregFrom
		{
			if (s_ == "null")
				return LocalregFrom::null;
			if (s_ == "alu")
				return LocalregFrom::alu;
			if (s_ == "in1")
				return LocalregFrom::in1;
			throw std::runtime_error("your configuration is boom(lr_from)");
			DEBUG_ASSERT(false);
			return LocalregFrom::null;
		}
	};

	class ControlModeConverter
	{
	public:
		static auto toString(ControlMode type_)->string
		{
			if (type_ == ControlMode::data_flow)
				return "data_flow";
			if (type_ == ControlMode::loop_activate)
				return "loop_activate";
			if (type_ == ControlMode::loop_reset)
				return "loop_reset";
			if (type_ == ControlMode::break_pre)
				return "break_pre";
			if (type_ == ControlMode::break_post)
				return "break_post";
			if (type_ == ControlMode::continue_)
				return "continue_";
			if (type_ == ControlMode::cb)
				return "cb";
			if (type_ == ControlMode::cinvb)
				return "cinvb";
			if (type_ == ControlMode::bind)
				return "bind";
			if (type_ == ControlMode::transout)
				return "transout";
			if (type_ == ControlMode::trans)
				return "trans";
			if(type_ == ControlMode::calc_activate)
				return "calc_activate";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->ControlMode
		{
			if (s_ == "data_flow")
				return ControlMode::data_flow;
			if (s_ == "loop_activate")
				return ControlMode::loop_activate;
			if (s_ == "loop_reset")
				return ControlMode::loop_reset;
			if (s_ == "calc_activate")
				return ControlMode::calc_activate;
			if (s_ == "break_pre")
				return ControlMode::break_pre;
			if (s_ == "break_post")
				return ControlMode::break_post;
			if (s_ == "continue_")
				return ControlMode::continue_;
			if (s_ == "cb")
				return ControlMode::cb;
			if (s_ == "cinvb")
				return ControlMode::cinvb;
			if(s_=="bind")
				return ControlMode::bind;
			if (s_ == "transout")
				return ControlMode::transout;
			if (s_ == "trans")
				return ControlMode::trans;
			throw std::runtime_error("your configuration is boom(control mode)");
			DEBUG_ASSERT(false);
			return ControlMode::data_flow;
		}
	};

	class AluInFromConverter
	{
	public:
		static auto toString(AluInFrom type_)->string
		{
			if (type_ == AluInFrom::null)
				return "null";
			if (type_ == AluInFrom::inbuffer)
				return "inbuffer";
			if (type_ == AluInFrom::inport)
				return "inport";
			if (type_ == AluInFrom::lr)
				return "lr";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->AluInFrom
		{
			if (s_ == "null")
				return AluInFrom::null;
			if (s_ == "inbuffer")
				return AluInFrom::inbuffer;
			if (s_ == "inport")
				return AluInFrom::inport;
			if (s_ == "lr")
				return AluInFrom::lr;
			throw std::runtime_error("your configuration is boom(alu from)");
			DEBUG_ASSERT(false);
			return AluInFrom::null;
		}
	};
	class StallModeConverter
	{
	public:
		static auto toString(stallType type_)->string
		{
			if (type_ == stallType::none)
				return "none";
			if (type_ == stallType::inbuffer_stall)
				return "inbuffer_stall";
			if (type_ == stallType::step_stall)
				return "step_stall";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->stallType
		{
			if (s_ == "none")
				return stallType::none;
			if (s_ == "inbuffer_stall")
				return stallType::inbuffer_stall;
			if (s_ == "step_stall")
				return stallType::step_stall;
			throw std::runtime_error("your configuration is boom(stall_type,recommend choice:none,inbuffer_stall,step_stall)");
			DEBUG_ASSERT(false);
		}
	};
	class OutBufferFromConverter
	{
	public:
		static auto toString(OutBufferFrom type_)->string
		{
			if (type_ == OutBufferFrom::null)
				return "null";
			if (type_ == OutBufferFrom::alu)
				return "alu";
			if (type_ == OutBufferFrom::inbuffer)
				return "inbuffer";
			if (type_ == OutBufferFrom::lr)
				return "lr";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->OutBufferFrom
		{
			if (s_ == "null")
				return OutBufferFrom::null;
			if (s_ == "inbuffer")
				return OutBufferFrom::inbuffer;
			if (s_ == "alu")
				return OutBufferFrom::alu;
			if (s_ == "lr")
				return OutBufferFrom::lr;
			throw std::runtime_error("your configuration is boom(obfrom)");
			DEBUG_ASSERT(false);
			return OutBufferFrom::null;
		}
	};

	class OutputFromConverter
	{
	public:
		static auto toString(OutputFrom type_)->string
		{
			if (type_ == OutputFrom::alu)
				return "alu";
			if (type_ == OutputFrom::outbuffer)
				return "ob";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->OutputFrom
		{
			if (s_ == "outbuffer")
				return OutputFrom::outbuffer;
			if (s_ == "alu")
				return OutputFrom::alu;
			throw std::runtime_error("your configuration is boom(output from)");
			DEBUG_ASSERT(false);
			return OutputFrom::outbuffer;
		}
	};

	class LSModeConverter
	{
	public:
		static auto toString(LSMode type_)->string
		{
			if (type_ == LSMode::load)
				return "load";
			if (type_ == LSMode::store_data)
				return "store_data";
			if (type_ == LSMode::store_addr)
				return "store_addr";
			if (type_ == LSMode::dummy)
				return "dummy";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->LSMode
		{
			if (s_ == "load")
				return LSMode::load;
			if (s_ == "store_data")
				return LSMode::store_data;
			if (s_ == "store_addr")
				return LSMode::store_addr;
			if (s_ == "dummy")
				return LSMode::dummy;
			throw std::runtime_error("your configuration is boom(lsmode)");
			DEBUG_ASSERT(false);
			return LSMode::load;
		}
	};

	class BufferSizeConverter
	{
	public:
		static auto toString(BufferSize type_)->string
		{
			if (type_ == BufferSize::small)
				return "small";
			if (type_ == BufferSize::middle)
				return "middle";
			if (type_ == BufferSize::large)
				return "large";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->BufferSize
		{
			if (s_ == "small")
				return BufferSize::small;
			if (s_ == "middle")
				return BufferSize::middle;
			if (s_ == "large")
				return BufferSize::large;
			throw std::runtime_error("your configuration is boom(buffersize)");
			DEBUG_ASSERT(false);
			return BufferSize::small;
		}
	};

	class PortTypeConverter
	{
	public:
		static auto toString(PortType type_)->string
		{
			if (type_ == PortType::d32_b1_v1)
				return "d32_b1_v1";
			if (type_ == PortType::d32_v1)
				return "d32_v1";
			if (type_ == PortType::d32_b1_v1_t)
				return "d32_b1_v1_t";
			if (type_ == PortType::d32_v1_t)
				return "d32_v1_t";
			if (type_ == PortType::d1_v1)
				return "d1_v1";
			if (type_ == PortType::d1_v1_t)
				return "d1_v1_t";
			if (type_ == PortType::bp)
				return "bp";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->PortType
		{
			if (s_ == "d32_b1_v1")
				return PortType::d32_b1_v1;
			if (s_ == "d32_v1")
				return PortType::d32_v1;
			if (s_ == "d32_b1_v1_t")
				return PortType::d32_b1_v1_t;
			if (s_ == "d32_v1_t")
				return PortType::d32_v1_t;
			if (s_ == "d1_v1")
				return PortType::d1_v1;
			if (s_ == "d1_v1_t")
				return PortType::d1_v1_t;
			if (s_ == "bp")
				return PortType::bp;
			DEBUG_ASSERT(false);
			return PortType::null;
		}
	};

	using WireTypeConverter = PortTypeConverter;
	using SegmentTypeConverter = PortTypeConverter;

	class PortDirectionConverter
	{
	public:
		static auto toString(PortDirection type_)->string
		{
			if (type_ == PortDirection::in)
				return "in";
			if (type_ == PortDirection::out)
				return "out";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->PortDirection
		{
			if (s_ == "in")
				return PortDirection::in;
			if (s_ == "out")
				return PortDirection::out;
			DEBUG_ASSERT(false);
			return PortDirection::null;
		}
	};

	class WireNetConfigModeConverter
	{
	public:
		static auto toString(WireNetConfigMode type_)->string
		{
			if (type_ == WireNetConfigMode::way4)
				return "way4";
			if (type_ == WireNetConfigMode::way8)
				return "way8";
			if (type_ == WireNetConfigMode::way4hop1)
				return "way4hop1";
			if (type_ == WireNetConfigMode::way4hop2)
				return "way4hop2";
			if (type_ == WireNetConfigMode::full_connect)
				return "full_connect";
			if (type_ == WireNetConfigMode::multi_connect)
				return "multi_connect";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->WireNetConfigMode
		{
			if (s_ == "way4")
				return WireNetConfigMode::way4;
			if (s_ == "way8")
				return WireNetConfigMode::way8;
			if (s_ == "way4hop1")
				return WireNetConfigMode::way4hop1;
			if (s_ == "way4hop2")
				return WireNetConfigMode::way4hop2;
			if (s_ == "full_connect")
				return WireNetConfigMode::full_connect;
			if (s_ == "multi_connect")
				return WireNetConfigMode::multi_connect;
			DEBUG_ASSERT(false);
			return WireNetConfigMode::null;
		}
	};

	class PortFunctionConverter
	{
	public:
		static auto toString(PortFunction type_)->string
		{
			if (type_ == PortFunction::normal)
				return "normal";
			if (type_ == PortFunction::router)
				return "router";
			if (type_ == PortFunction::bp)
				return "bp";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->PortFunction
		{
			if (s_ == "normal")
				return PortFunction::normal;
			if (s_ == "router")
				return PortFunction::router;
			if (s_ == "bp")
				return PortFunction::bp;
			DEBUG_ASSERT(false);
			return PortFunction::null;
		}
	};

	class BufferBpConverter
	{
	public:
		static auto toString(BpType type_)->string
		{
			if (type_ == BpType::credit)
				return "credit";
			if (type_ == BpType::ack)
				return "ack";
			DEBUG_ASSERT(false);
			return "";
		}

		static auto toEnum(string s_)->BpType
		{
			if (s_ == "credit")
				return BpType::credit;
			if (s_ == "ack")
				return BpType::ack;
			DEBUG_ASSERT(false);
			return BpType::credit;
		}
	};
}