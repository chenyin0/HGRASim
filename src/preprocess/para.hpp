#pragma once

#include "../util/cord.hpp"
#include "../../lib/tinyxml2/tinyxml2.h"
#include "dfg_node.hpp"
#include "enum_definition.hpp"
#include "arch_block_net.hpp"

namespace Simulator::Preprocess
{
/** Array Parameter */
	struct ArrayPara
	{
	    //system
		uint debug_level;
		bool print_screen;
		bool attach_memory;
		bool profiling;
		bool read_bypass;
		bool write_bypass;
		bool cache_mode;
		bool inflight_block;
		uint maxclk;
		uint max_memory_depth;
		stallType stall_mode;
		//pe
		uint pe_num;
		uint pe_in_buffer_depth;
		BpType buffer_bp;
		uint pe_out_buffer_depth;
		uint lse_buffer_depth;
		PortType data_port_type;
		PortType bool_port_type;
		uint data_inport_breadth;
		uint bool_inport_breadth;
		uint data_outport_breadth;
		uint bool_outport_breadth;
		uint alu_in_num;

		//lc
		uint lc_num;
		uint lc_endin_num;
		uint lc_regin_num;
		uint lc_output_num;
		uint lc_buffer_bool_breadth;
		uint lc_buffer_data_breadth;
		uint lc_buffer_depth;

		//lse
		uint lse_num;
		uint lse_virtual_num;  // 硬件实际的LSE数量
		uint lse_inbuffer_depth;
		uint lse_datain_breadth;
		uint lse_boolin_breadth;
		uint le_dataout_breadth;
		uint le_boolout_breadth;
		uint le_outbuffer_depth_small;
		uint le_outbuffer_depth_middle;
		uint le_outbuffer_depth_large;



		//SPM
		uint SPM_depth;

		//lsu
		uint tabline_num;
		uint fifoline_num;
		uint offset_depth;
		uint post_offset_depth;
		uint in_num;
		uint tagbits;
		bool overlap_print;
		bool bus_enable;
		uint bus_delay;
		bool print_bus;
	};

/** Arch Parameter */

	struct ArchPara
	{
		// 阵列行列数
		uint row_num;
		uint col_num;

		// block的类型信息，每一类block有一个ArchBlock
		unordered_map<BlockType, ArchBlock> arch_block_type;

		// wire的类型信息，每一类有一系列WireNetInterface*
		// 调用他们的WireNetInterface->generate()可以生成对应的线网
		unordered_map<WireNetConfigMode, vector<WireNetInterface*>> wire_nets;

		vector<SegmentNet> segment_net;

		~ArchPara()
		{
			for (auto&[mode, v_ptr] : wire_nets)
				for (auto& ptr : v_ptr)
					delete ptr;
		}
	};


}