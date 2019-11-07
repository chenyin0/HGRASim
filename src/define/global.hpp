#pragma once

/**
 * 这个类用来保存所有的文件路径
 * 类为单例的
 * 使用GlobalPara::getInstance()->getInputAddr(...)/getOutputAddr(...)来访问
 * 对这个文件的改动要通知全员，最好就别改！
 * 
 * 行 19.7.9
 */

#include "define.hpp"
#include "../util/util.hpp"

namespace Simulator
{
	enum class InputAddr : unsigned char
	{
		architecture_xml_addr,
		DFG_xml_addr,
		parameter_xml_addr
	};

	enum class OutputAddr : unsigned char
	{
		hgra_debug_inner_txt_addr,
		hgra_debug_port_txt_addr,
		hgra_debug_memory_txt_addr,

		resource_block_graph_info_xml_addr,
		resource_port_graph_info_xml_addr,
		resource_segment_graph_info_xml_addr,
		resource_wire_graph_info_xml_addr,

		placement_result_xml_addr,
		routing_result_xml_addr,

		bitstream_result_xml_addr
	};

	class GlobalPara final : public Singleton<GlobalPara>
	{
	private:
		friend class Singleton<GlobalPara>;
	private:
		// INPUT
		string _architecture_xml_addr;
		string _dfg_xml_addr;
		string _parameter_xml_addr;

		// OUTPUT
		string _hgra_debug_inner_txt_addr;
		string _hgra_debug_port_txt_addr;
		string _hgra_debug_memory_txt_addr;

		string _resource_block_graph_info_xml_addr;
		string _resource_port_graph_info_xml_addr;
		string _resource_segment_graph_info_xml_addr;
		string _resource_wire_graph_info_xml_addr;

		string _placement_result_xml_addr;
		string _routing_result_xml_addr;

		string _bitstream_result_xml_addr;

	private:
		GlobalPara() = default;
		~GlobalPara() = default;

		GlobalPara(const GlobalPara& b_) = delete;
		GlobalPara(GlobalPara&& b_) = delete;
		GlobalPara& operator=(const GlobalPara& b_) = delete;
		GlobalPara& operator=(GlobalPara&& b_) = delete;

	public:
		// get addr
		[[nodiscard]] 
		auto getInputAddr(InputAddr type_) const->string
		{
			if (type_ == InputAddr::architecture_xml_addr)
				return _architecture_xml_addr;
			if (type_ == InputAddr::DFG_xml_addr)
				return _dfg_xml_addr;
			if (type_ == InputAddr::parameter_xml_addr)
				return _parameter_xml_addr;
			DEBUG_ASSERT(false);
			return "";
		}

		[[nodiscard]] 
		auto getOutputAddr(OutputAddr type_) const -> string
		{
			if (type_ == OutputAddr::hgra_debug_inner_txt_addr)
				return _hgra_debug_inner_txt_addr;
			if (type_ == OutputAddr::hgra_debug_port_txt_addr)
				return _hgra_debug_port_txt_addr;
			if (type_ == OutputAddr::hgra_debug_memory_txt_addr)
				return _hgra_debug_memory_txt_addr;
			if (type_ == OutputAddr::resource_block_graph_info_xml_addr)
				return _resource_block_graph_info_xml_addr;
			if (type_ == OutputAddr::resource_port_graph_info_xml_addr)
				return _resource_port_graph_info_xml_addr;
			if (type_ == OutputAddr::resource_segment_graph_info_xml_addr)
				return _resource_segment_graph_info_xml_addr;
			if (type_ == OutputAddr::resource_wire_graph_info_xml_addr)
				return _resource_wire_graph_info_xml_addr;
			if (type_ == OutputAddr::placement_result_xml_addr)
				return _placement_result_xml_addr;
			if (type_ == OutputAddr::routing_result_xml_addr)
				return _routing_result_xml_addr;
			if (type_ == OutputAddr::bitstream_result_xml_addr)
				return _bitstream_result_xml_addr;

			DEBUG_ASSERT(false);
			return "";
		}

		// set addr
		auto setInputAddr(InputAddr type_, string addr_) -> void
		{
			if (type_ == InputAddr::DFG_xml_addr)
				_dfg_xml_addr = addr_;
			if (type_ == InputAddr::architecture_xml_addr)
				_architecture_xml_addr = addr_;
			if (type_ == InputAddr::parameter_xml_addr)
				_parameter_xml_addr = addr_;
		}

		auto setOutputAddr(OutputAddr type_, string addr_) -> void
		{
			if (type_ == OutputAddr::hgra_debug_inner_txt_addr)
				_hgra_debug_inner_txt_addr = addr_;
			else if (type_ == OutputAddr::hgra_debug_port_txt_addr)
				_hgra_debug_port_txt_addr = addr_;
			else if (type_ == OutputAddr::hgra_debug_memory_txt_addr)
				_hgra_debug_memory_txt_addr = addr_;

			else if (type_ == OutputAddr::resource_block_graph_info_xml_addr)
				_resource_block_graph_info_xml_addr = addr_;
			else if (type_ == OutputAddr::resource_port_graph_info_xml_addr)
				_resource_port_graph_info_xml_addr = addr_;
			else if (type_ == OutputAddr::resource_segment_graph_info_xml_addr)
				_resource_segment_graph_info_xml_addr = addr_;
			else if (type_ == OutputAddr::resource_wire_graph_info_xml_addr)
				_resource_wire_graph_info_xml_addr = addr_;
			else if (type_ == OutputAddr::placement_result_xml_addr)
				_placement_result_xml_addr = addr_;
			else if (type_ == OutputAddr::routing_result_xml_addr)
				_routing_result_xml_addr = addr_;
			else if (type_ == OutputAddr::bitstream_result_xml_addr)
				_bitstream_result_xml_addr = addr_;
			else
				DEBUG_ASSERT(false);
		}
	};

}
