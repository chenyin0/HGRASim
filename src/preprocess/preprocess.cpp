#include "preprocess.h"

namespace Simulator::Preprocess
{

	DFG::DFG()
	{
		xmlRead();
	}

	DFG::~DFG()
	{
		for (auto& [type, vec] : _dictionary)
		{
			for (auto& ptr : vec)
				delete ptr;
		}
	}

	auto DFG::getDictionary() const -> const unordered_map<NodeType, vector<const DFGNodeInterface*>>&
	{
		return _dictionary;
	}

	auto DFG::getDfgDict() const -> const std::vector<std::pair<NodeType, const DFGNodeInterface*>> &
	{
		return dfg_dictionary;
	}

	auto DFG::getDfgCluster() const -> const ClusterGroupInterface &
	{
		return cluster_group;
	}
	auto DFG::isManualPlacement() const -> bool
	{
		return _manual_placement;
	}
	auto DFG::getContext() const -> const unordered_map<NodeType, vector<vector<const DFGNodeInterface*>>> &
	{
		return context_dictionary;
	}
	auto DFG::xmlRead() -> void
	{
		XMLDocument doc;
		if (doc.LoadFile(GlobalPara::getInstance()->getInputAddr(InputAddr::DFG_xml_addr).c_str()))
		{
			string path_info = GlobalPara::getInstance()->getInputAddr(InputAddr::DFG_xml_addr);
			DEBUG_ASSERT(false);
		}
		context_dictionary[NodeType::ls].resize(Para::getInstance()->getArrayPara().lscluster_num);
		XMLElement* root_xml = doc.RootElement();
		_manual_placement = static_cast<string>(root_xml->FindAttribute("manual_placement")->Value()) == string("true");

		XMLElement* node_xml = root_xml->FirstChildElement("node");

		while (node_xml)
		{
			NodeType node_type = NodeTypeConverter::toEnum(node_xml->FindAttribute("type")->Value());
			try {
				switch (node_type)
				{
				case NodeType::pe:
				{
					_dictionary[NodeType::pe].push_back(peRead(node_xml));
					dfg_dictionary.push_back({ NodeType::pe,peRead(node_xml) });
					break;
				}
				case NodeType::fg:
				{
					_dictionary[NodeType::fg].push_back(fgRead(node_xml));
					dfg_dictionary.push_back({ NodeType::fg, fgRead(node_xml) });

					break;
				}
				case NodeType::ls:
				{
					_dictionary[NodeType::ls].push_back(lsRead(node_xml));
					dfg_dictionary.push_back({ NodeType::ls, lsRead(node_xml) });
					if (MemAccessModeConverter::toEnum(node_xml->FindAttribute("mem_access_mode")->Value()) != MemAccessMode::none) {
						context_dictionary[NodeType::ls][std::stoi(node_xml->FindAttribute("cluster")->Value())].push_back(lsRead(node_xml));
					}
					break;
				}
				case NodeType::lv:
				{
					_dictionary[NodeType::lv].push_back(lvRead(node_xml));
					dfg_dictionary.push_back(std::make_pair(NodeType::lv, lvRead(node_xml)));
					break;
				}
				case NodeType::lc:
				{
					_dictionary[NodeType::lc].push_back(lcRead(node_xml));
					dfg_dictionary.push_back({ NodeType::lc,lcRead(node_xml) });
					break;
				}
				default:
					break;
				}
			}
			catch (std::runtime_error err) {
				std::cout << err.what() << " at config order"<< dfg_dictionary .size() << "\nTry again? enter y or n" << std::endl;
				char c;
				std::cin >> c;
				if (!std::cin || c == 'n')
					break;
			}

			node_xml = node_xml->NextSiblingElement("node");
		}
		XMLElement* cluster_xml = root_xml->FirstChildElement("cluster");
		while (cluster_xml) {
			uint id= std::stoi(cluster_xml->FindAttribute("index")->Value());
			NodeType node_type = NodeTypeConverter::toEnum(cluster_xml->FindAttribute("type")->Value());
			string indexstr = static_cast<string>(cluster_xml->FindAttribute("lsindexs")->Value());
			vector<string> indexs= Util::splitString(indexstr, "_", true);
			if (indexs.size() != 3) {
				throw std::runtime_error("raed cluster bug at lsindexs");
			}
			vector<Input> input_vec;
			XMLElement* input_xml = cluster_xml->FirstChildElement("input");
			NodeType input_type = NodeTypeConverter::toEnum(input_xml->FindAttribute("type")->Value());
			if (input_type == NodeType::null || input_type == NodeType::begin)
			{
				input_vec.emplace_back(input_type, 0, 0);
			}
			else
			{
				uint input_index = std::stoi(input_xml->FindAttribute("index")->Value());
				uint port_index = std::stoi(input_xml->FindAttribute("port")->Value());
				input_vec.emplace_back(input_type, input_index, port_index);
			}

			input_xml = input_xml->NextSiblingElement("input");
			uint start_node = std::stoi(indexs[0]),step= std::stoi(indexs[1]),end_node= std::stoi(indexs[2]);
			shared_ptr<ClusterInterface> cluster=make_shared<ClusterInterface>(node_type, id, start_node, step, end_node, 0, input_vec);
			cluster_group.clusters.insert({ {node_type, id}, cluster });
			for (uint i = start_node; i <= end_node; i++) {
				cluster_group.ls_cluster.insert({ {node_type, i}, id });
			}
			cluster_xml = cluster_xml->NextSiblingElement("cluster");
		}
	}

	auto DFG::fgRead(XMLElement* node_xml_) const -> DFGNodeInterface*
	{
		// attributes
		uint index = std::stoi(node_xml_->FindAttribute("index")->Value());
		FGMode mode = FGModeConverter::toEnum(node_xml_->FindAttribute("mode")->Value());
		uint mask = std::stoi(node_xml_->FindAttribute("mask")->Value());
		uint lut = std::stoi(node_xml_->FindAttribute("lut")->Value());
		bool tag_mode = static_cast<string>(node_xml_->FindAttribute("tagMode")->Value()) == string("true");
		bool combine_path = static_cast<string>(node_xml_->FindAttribute("combinePath")->Value()) == string("true");
		bool tag_bind = static_cast<string>(node_xml_->FindAttribute("tagBind")->Value()) == string("true");

		// inputs
		vector<Input> input_vec;
		XMLElement* input_xml = node_xml_->FirstChildElement("input");
		while (input_xml)
		{
			NodeType input_type = NodeTypeConverter::toEnum(input_xml->FindAttribute("type")->Value());
			if (input_type == NodeType::null || input_type == NodeType::begin)
			{
				input_vec.emplace_back(input_type, 0, 0);
			}
			else
			{
				uint input_index = std::stoi(input_xml->FindAttribute("index")->Value());
				uint port_index = std::stoi(input_xml->FindAttribute("port")->Value());
				input_vec.emplace_back(input_type, input_index, port_index);
			}

			input_xml = input_xml->NextSiblingElement("input");
		}

		// reg
		XMLElement* reg_xml = node_xml_->FirstChildElement("reg");
		int reg_value = std::stoi(reg_xml->FindAttribute("value")->Value());

		// manual placement
		XMLElement* manual_placement_xml = node_xml_->FirstChildElement("placement");
		Cord manual_cord = Cord{};
		if (manual_placement_xml)
			manual_cord = Cord::stringAnalysis(static_cast<string>(manual_placement_xml->FindAttribute("cord")->Value()));

		// build
		DFGNode<NodeType::fg>* ptr = new DFGNode<NodeType::fg>(index, mode, mask, lut, tag_mode, combine_path, tag_bind, input_vec, reg_value, manual_cord);

		return dynamic_cast<DFGNodeInterface*>(ptr);
	}

	auto DFG::peRead(XMLElement* node_xml_) const -> DFGNodeInterface*
	{
		// attributes
		uint index = std::stoi(node_xml_->FindAttribute("index")->Value());
		bool key_cal = false;
		PEOpcode opcode = PEOpcodeConverter::toEnum(node_xml_->FindAttribute("opcode")->Value());
		ControlMode control_mode = ControlModeConverter::toEnum(node_xml_->FindAttribute("controlMode")->Value());
	//	bool key_cal = static_cast<string>(node_xml_->FindAttribute("controlMode")->Value()) == string("true");
	//	InputBypass input_bypass= InputBypassConverter::toEnum(node_xml_->FindAttribute("input_bypass")->Value());
		// inner connection
		XMLElement* inner_connection_xml = node_xml_->FirstChildElement("inner_connection");
		InputBypass input_bypass = InputBypassConverter::toEnum(inner_connection_xml->FindAttribute("input_bypass")->Value());
//		LocalregFrom lr_from = LocalregFromConverter::toEnum(inner_connection_xml->FindAttribute("lr_from")->Value());
//		CondMode cond_mode=CondConverter::toEnum(inner_connection_xml->FindAttribute("lr_from")->Value());
		//vector<AluInFrom> alu_in_from;
		//AluInFrom alu_in0_from = AluInFromConverter::toEnum(inner_connection_xml->FindAttribute("alu_in0_from")->Value());
		//AluInFrom alu_in1_from = AluInFromConverter::toEnum(inner_connection_xml->FindAttribute("alu_in1_from")->Value());
		//AluInFrom alu_in2_from = AluInFromConverter::toEnum(inner_connection_xml->FindAttribute("alu_in2_from")->Value());
		//alu_in_from.push_back(alu_in0_from);
		//alu_in_from.push_back(alu_in1_from);
		//alu_in_from.push_back(alu_in2_from);

		vector<InBufferFrom> inbuffer_from;
		InBufferFrom buffer0_from = InBufferFromConverter::toEnum(inner_connection_xml->FindAttribute("buffer0_from")->Value());
		InBufferFrom buffer1_from = InBufferFromConverter::toEnum(inner_connection_xml->FindAttribute("buffer1_from")->Value());
		InBufferFrom buffer2_from = InBufferFromConverter::toEnum(inner_connection_xml->FindAttribute("buffer2_from")->Value());
		inbuffer_from.push_back(buffer0_from);
		inbuffer_from.push_back(buffer1_from);
		inbuffer_from.push_back(buffer2_from);

		vector<BufferMode> buffer_mode;
		BufferMode buffer0_mode = BufferModeConverter::toEnum(inner_connection_xml->FindAttribute("buffer0_mode")->Value());
		BufferMode buffer1_mode = BufferModeConverter::toEnum(inner_connection_xml->FindAttribute("buffer1_mode")->Value());
		BufferMode buffer2_mode = BufferModeConverter::toEnum(inner_connection_xml->FindAttribute("buffer2_mode")->Value());
		buffer_mode.push_back(buffer0_mode);
		buffer_mode.push_back(buffer1_mode);
		buffer_mode.push_back(buffer2_mode);

		//vector<OutBufferFrom> ob_from;
		//OutBufferFrom ob0_from = OutBufferFromConverter::toEnum(inner_connection_xml->FindAttribute("ob0_from")->Value());
		//ob_from.push_back(ob0_from);
		
		vector<OutputFrom> output_from;
		OutputFrom output0_from = OutputFromConverter::toEnum(inner_connection_xml->FindAttribute("output_from")->Value());
		output_from.push_back(output0_from);

		//inputs
		vector<Input> input_vec;
		XMLElement* input_xml = node_xml_->FirstChildElement("input");
		while (input_xml)
		{
			NodeType input_type = NodeTypeConverter::toEnum(input_xml->FindAttribute("type")->Value());
			if (input_type == NodeType::null || input_type == NodeType::begin)
			{
				input_vec.emplace_back(input_type, 0, 0);
			}
			else
			{
				uint input_index = std::stoi(input_xml->FindAttribute("index")->Value());
				uint port_index = std::stoi(input_xml->FindAttribute("port")->Value());
				input_vec.emplace_back(input_type, input_index, port_index);
			}

			input_xml = input_xml->NextSiblingElement("input");
		}

		// reg
		vector<int> reg_vec;
		XMLElement* reg_xml = node_xml_->FirstChildElement("reg");
		while (reg_xml)
		{
			int reg_value;
			if (reg_xml->FindAttribute("value")->Value() != string("null"))
				reg_value = std::stoi(reg_xml->FindAttribute("value")->Value());
			else
				reg_value = INT_MAX;
			reg_vec.push_back(reg_value);
			reg_xml = reg_xml->NextSiblingElement("reg");
		}

		// manual placement
		XMLElement* manual_placement_xml = node_xml_->FirstChildElement("placement");
		Cord manual_cord = Cord{};
		if (manual_placement_xml)
			manual_cord = Cord::stringAnalysis(static_cast<string>(manual_placement_xml->FindAttribute("cord")->Value()));

		// build
		//DFGNode<NodeType::pe>* ptr = new DFGNode<NodeType::pe>(index, opcode, control_mode, lr_from, alu_in_from, 
		//	ob_from, output_from, input_vec, reg_value, manual_cord);
		if (node_xml_->FindAttribute("key_cal")) {
			key_cal = static_cast<string>(node_xml_->FindAttribute("key_cal")->Value()) == string("true");
		}
		DFGNode<NodeType::pe>* ptr = new DFGNode<NodeType::pe>(index, opcode, control_mode, output_from, input_vec, reg_vec, manual_cord, buffer_mode, input_bypass, inbuffer_from, key_cal);
		return dynamic_cast<DFGNodeInterface*>(ptr);
	}
	
	auto DFG::lsRead(XMLElement* node_xml_) const -> DFGNodeInterface*
	{
		// attributes
		uint index = std::stoi(node_xml_->FindAttribute("index")->Value());
		uint cluster = std::stoi(node_xml_->FindAttribute("cluster")->Value());
		LSMode ls_mode = LSModeConverter::toEnum(node_xml_->FindAttribute("ls_mode")->Value());
		bool tag_bind = static_cast<string>(node_xml_->FindAttribute("tag_mode")->Value()) == string("true");
		bool dae = static_cast<string>(node_xml_->FindAttribute("dae")->Value()) == string("true");
		uint fifo_step = std::stoi(node_xml_->FindAttribute("fifo_step")->Value());
		BufferSize size = BufferSizeConverter::toEnum(node_xml_->FindAttribute("buffer_size")->Value());
		bool match = static_cast<string>(node_xml_->FindAttribute("match")->Value()) == string("true");
		MemAccessMode mem_access_mode = MemAccessModeConverter::toEnum(node_xml_->FindAttribute("mem_access_mode")->Value());
		DirectionMode direction_mode= DirectionModeConverter::toEnum(node_xml_->FindAttribute("direction_mode")->Value());
		BranchMode branch_mode= BranchModeConverter::toEnum(node_xml_->FindAttribute("branch_mode")->Value());
		//inputs
		vector<Input> input_vec;
		XMLElement* input_xml = node_xml_->FirstChildElement("input");
		while (input_xml)
		{
			NodeType input_type = NodeTypeConverter::toEnum(input_xml->FindAttribute("type")->Value());
			if (input_type == NodeType::null || input_type == NodeType::begin)
			{
				input_vec.emplace_back(input_type, 0, 0);
			}
			else
			{
				uint input_index = std::stoi(input_xml->FindAttribute("index")->Value());
				uint port_index = std::stoi(input_xml->FindAttribute("port")->Value());
				input_vec.emplace_back(input_type, input_index, port_index);
			}

			input_xml = input_xml->NextSiblingElement("input");
		}

		// manual placement
		XMLElement* manual_placement_xml = node_xml_->FirstChildElement("placement");
		Cord manual_cord = Cord{};
		if (manual_placement_xml)
			manual_cord = Cord::stringAnalysis(static_cast<string>(manual_placement_xml->FindAttribute("cord")->Value()));

		// build
		if (node_xml_->FindAttribute("vecmode")) {
			VecMode vecmode=VecmodeConverter::toEnum(node_xml_->FindAttribute("vecmode")->Value());
			string pss = static_cast<string>(node_xml_->FindAttribute("pss")->Value());
			vector<string> vec_pss = Util::splitString(pss, "_", true);
			if (vec_pss.size() != 3) {
				throw std::runtime_error("your configuration is boom(pss)");
			}
			DFGNode<NodeType::ls>* ptr = new DFGNode<NodeType::ls>(index, ls_mode, tag_bind,
				dae, fifo_step, size, match, input_vec, manual_cord, mem_access_mode, direction_mode, branch_mode,cluster,vecmode, std::stoi(vec_pss[0]), std::stoi(vec_pss[1]), std::stoi(vec_pss[2]));
			return dynamic_cast<DFGNodeInterface*>(ptr);
		}
		else {
			DFGNode<NodeType::ls>* ptr = new DFGNode<NodeType::ls>(index, ls_mode, tag_bind,
				dae, fifo_step, size, match, input_vec, manual_cord, mem_access_mode, direction_mode, branch_mode, cluster);
			return dynamic_cast<DFGNodeInterface*>(ptr);
		}

		//return dynamic_cast<DFGNodeInterface*>(ptr);
	}

	auto DFG::lsContextRead(XMLElement* context_xml_) const->vector<DFGNodeInterface*>
	{
		vector<DFGNodeInterface*> OneContext;
		XMLElement* node_xml = context_xml_->FirstChildElement("node");
		while(node_xml){
			NodeType node_type = NodeTypeConverter::toEnum(node_xml->FindAttribute("type")->Value());
			OneContext.push_back(lsRead(node_xml));
			node_xml = node_xml->NextSiblingElement("node");
		}
	}
	auto DFG::lvRead(XMLElement* node_xml_) const -> DFGNodeInterface*
	{
		return nullptr;
	}

	auto DFG::lcRead(XMLElement* node_xml_) const -> DFGNodeInterface*
	{
		// attributes
		uint index = std::stoi(node_xml_->FindAttribute("index")->Value());
		bool outermost = static_cast<string>(node_xml_->FindAttribute("outermost")->Value()) == string("true");

		//inputs
		vector<Input> input_vec;
		XMLElement* input_xml = node_xml_->FirstChildElement("input");
		while (input_xml)
		{
			NodeType input_type = NodeTypeConverter::toEnum(input_xml->FindAttribute("type")->Value());
			if (input_type == NodeType::null || input_type == NodeType::begin)
			{
				input_vec.emplace_back(input_type, 0, 0);
			}
			else
			{
				uint input_index = std::stoi(input_xml->FindAttribute("index")->Value());
				uint port_index = std::stoi(input_xml->FindAttribute("port")->Value());
				input_vec.emplace_back(input_type, input_index, port_index);
			}

			input_xml = input_xml->NextSiblingElement("input");
		}

		// reg
		vector<int> reg_vec;
		XMLElement* reg_xml = node_xml_->FirstChildElement("reg");
		while (reg_xml)
		{
			int reg_value;
			if (reg_xml->FindAttribute("value")->Value() != string("null"))
				reg_value = std::stoi(reg_xml->FindAttribute("value")->Value());
			else
				reg_value = INT_MAX;
//			int reg_value = std::stoi(reg_xml->FindAttribute("value")->Value());
			reg_vec.push_back(reg_value);
			reg_xml = reg_xml->NextSiblingElement("reg");
		}

		// manual placement
		XMLElement* manual_placement_xml = node_xml_->FirstChildElement("placement");
		Cord manual_cord = Cord{};
		if (manual_placement_xml)
			manual_cord = Cord::stringAnalysis(static_cast<string>(manual_placement_xml->FindAttribute("cord")->Value()));

		// build
		DFGNode<NodeType::lc>* ptr = new DFGNode<NodeType::lc>(index, outermost, input_vec, reg_vec, manual_cord);

		return dynamic_cast<DFGNodeInterface*>(ptr);
	}

	Para::Para()
	{
		xmlRead();
	}

	auto Para::getArrayPara() const -> const ArrayPara&
	{
		return _array_para;
	}

	auto Para::getArchPara() const -> const ArchPara&
	{
		return _arch_para;
	}

	auto Para::xmlRead() -> void
	{
		arrayParaXmlRead();
		archParaXmlRead();
	}

	auto Para::arrayParaXmlRead() -> void
	{
		XMLDocument doc;
		if (doc.LoadFile(GlobalPara::getInstance()->getInputAddr(InputAddr::parameter_xml_addr).c_str()))
		{
			string path_info = GlobalPara::getInstance()->getInputAddr(InputAddr::parameter_xml_addr);
			DEBUG_ASSERT(false);
		}

		XMLElement* root_xml = doc.RootElement();
		XMLElement* array_xml = root_xml->FirstChildElement("array");

		//pe
		_array_para.pe_num = std::stoi(array_xml->FirstChildElement("peNum")->GetText());
		_array_para.pe_in_buffer_depth = std::stoi(array_xml->FirstChildElement("peInBufferDepth")->GetText());
		_array_para.buffer_bp = BufferBpConverter::toEnum(array_xml->FirstChildElement("peBufferBp")->GetText());
		_array_para.pe_out_buffer_depth = std::stoi(array_xml->FirstChildElement("peOutBufferDepth")->GetText());
		_array_para.data_port_type = PortTypeConverter::toEnum(array_xml->FirstChildElement("dataPortType")->GetText());
		_array_para.bool_port_type = PortTypeConverter::toEnum(array_xml->FirstChildElement("boolPortType")->GetText());
		_array_para.data_inport_breadth = std::stoi(array_xml->FirstChildElement("dataInPortNum")->GetText());
		_array_para.bool_inport_breadth = std::stoi(array_xml->FirstChildElement("boolInPortNum")->GetText());
		_array_para.data_outport_breadth = std::stoi(array_xml->FirstChildElement("dataOutPortNum")->GetText());
		_array_para.bool_outport_breadth = std::stoi(array_xml->FirstChildElement("boolOutPortNum")->GetText());
		_array_para.alu_in_num = std::stoi(array_xml->FirstChildElement("aluInNum")->GetText());

		//lc
		_array_para.lc_num = std::stoi(array_xml->FirstChildElement("lcNum")->GetText());
		_array_para.lc_endin_num = std::stoi(array_xml->FirstChildElement("lcEndInNum")->GetText());
		_array_para.lc_regin_num = std::stoi(array_xml->FirstChildElement("lcRegInNum")->GetText());
		_array_para.lc_output_num = std::stoi(array_xml->FirstChildElement("lcOutputNum")->GetText());
		_array_para.lc_buffer_bool_breadth = std::stoi(array_xml->FirstChildElement("lcBufferBoolBreadth")->GetText());
		_array_para.lc_buffer_data_breadth = std::stoi(array_xml->FirstChildElement("lcBufferDataBreadth")->GetText());
		_array_para.lc_buffer_depth = std::stoi(array_xml->FirstChildElement("lcBufferDepth")->GetText());

		//lse
		_array_para.lse_inbuffer_depth = std::stoi(array_xml->FirstChildElement("lseInBufferDepth")->GetText());
		_array_para.lse_num= std::stoi(array_xml->FirstChildElement("lseNum")->GetText());
		_array_para.spm_bank = std::stoi(array_xml->FirstChildElement("spmBank")->GetText());
		_array_para.lscluster_num = std::stoi(array_xml->FirstChildElement("clusterNum")->GetText());
		_array_para.SPM_depth = std::stoi(array_xml->FirstChildElement("spmDepth")->GetText());
		_array_para.lse_datain_breadth = std::stoi(array_xml->FirstChildElement("lseDataInBreadth")->GetText());
		_array_para.lse_boolin_breadth = std::stoi(array_xml->FirstChildElement("lseBoolInBreadth")->GetText());
		_array_para.le_dataout_breadth = std::stoi(array_xml->FirstChildElement("leDataOutBreadth")->GetText());
		_array_para.le_boolout_breadth = std::stoi(array_xml->FirstChildElement("leBoolOutBreadth")->GetText());
		_array_para.le_outbuffer_depth_small = std::stoi(array_xml->FirstChildElement("leOutBufferDepthSmall")->GetText());
		_array_para.le_outbuffer_depth_middle = std::stoi(array_xml->FirstChildElement("leOutBufferDepthMiddle")->GetText());
		_array_para.le_outbuffer_depth_large = std::stoi(array_xml->FirstChildElement("leOutBufferDepthLarge")->GetText());

		//lsu
		_array_para.tabline_num = std::stoi(array_xml->FirstChildElement("tabline_num")->GetText());
		_array_para.fifoline_num = std::stoi(array_xml->FirstChildElement("fifolinen_num")->GetText());
		_array_para.offset_depth = std::stoi(array_xml->FirstChildElement("offset_depth")->GetText());
		_array_para.post_offset_depth = std::stoi(array_xml->FirstChildElement("post_offset_depth")->GetText());
		_array_para.in_num = std::stoi(array_xml->FirstChildElement("in_num")->GetText());
		_array_para.tagbits = std::stoi(array_xml->FirstChildElement("tagbits")->GetText());
		_array_para.overlap_print = (array_xml->FirstChildElement("overlap_print")->GetText() == string("true"));//=true不能正确判断，需要用string做类型转换
		_array_para.bus_enable = (array_xml->FirstChildElement("bus_enable")->GetText() == string("true"));
		_array_para.profiling = (array_xml->FirstChildElement("profiling")->GetText() == string("true"));
		_array_para.bus_delay = std::stoi(array_xml->FirstChildElement("bus_delay")->GetText());
		_array_para.print_bus = (array_xml->FirstChildElement("print_bus")->GetText() == string("true"));
		_array_para.read_bypass = (array_xml->FirstChildElement("read_bypass")->GetText() == string("true"));
		_array_para.write_bypass = (array_xml->FirstChildElement("write_bypass")->GetText() == string("true"));
		_array_para.cache_mode = (array_xml->FirstChildElement("cache_mode")->GetText() == string("true"));
		_array_para.inflight_block = (array_xml->FirstChildElement("inflight_blocking")->GetText() == string("true"));
		//system
		_array_para.debug_level = std::stoi(array_xml->FirstChildElement("debugLevel")->GetText());
		_array_para.stall_mode = StallModeConverter::toEnum(array_xml->FirstChildElement("stallMode")->GetText());
		_array_para.print_screen = (array_xml->FirstChildElement("printscreen")->GetText() == string("true"));
		_array_para.maxclk = std::stoi(array_xml->FirstChildElement("maxClk")->GetText());
		_array_para.max_memory_depth = std::stoi(array_xml->FirstChildElement("maxMemoryDepth")->GetText());
		_array_para.attach_memory = (array_xml->FirstChildElement("attachMemory")->GetText() == string("true"));
		_array_para.spm_mode = (array_xml->FirstChildElement("spm_mode")->GetText() == string("true"));
	
	}

	auto Para::archParaXmlRead() -> void
	{
		XMLDocument doc;
		if (doc.LoadFile(GlobalPara::getInstance()->getInputAddr(InputAddr::architecture_xml_addr).c_str()))
		{
			string path_info = GlobalPara::getInstance()->getInputAddr(InputAddr::architecture_xml_addr);
			DEBUG_ASSERT(false);
		}

		XMLElement* root_xml = doc.RootElement();
		XMLElement* block_xml = root_xml->FirstChildElement("BlockConfig");
		archBlockXmlRead(block_xml);
		XMLElement* net_xml = root_xml->FirstChildElement("NetConfig");
		archNetXmlRead(net_xml);
	}

	auto Para::archBlockXmlRead(XMLElement* block_xml_) -> void
	{
		_arch_para.row_num = std::stoi(block_xml_->FirstChildElement("global")->FindAttribute("row")->Value());
		_arch_para.col_num = std::stoi(block_xml_->FirstChildElement("global")->FindAttribute("row")->Value());

		XMLElement* block_xml = block_xml_->FirstChildElement("block");

		while (block_xml)
		{
			// prepare
			map<PortDirection, vector<PortType>> normal_dict;
			vector<PortType> router_dict;
			vector<vector<Bool>> position;
			// block type
			string block_type_s = block_xml->FirstChildElement("type")->GetText();
			BlockType block_type = BlockTypeConverter::toEnum(block_type_s);
			// in ports
			XMLElement* inport_xml = block_xml->FirstChildElement("in_ports")->FirstChildElement("port_type");
			while (inport_xml)
			{
				PortType port_type = PortTypeConverter::toEnum(inport_xml->GetText());
				normal_dict[PortDirection::in].push_back(port_type);

				inport_xml = inport_xml->NextSiblingElement("port_type");
			}
			// out ports
			XMLElement* outport_xml = block_xml->FirstChildElement("out_ports")->FirstChildElement("port_type");
			while (outport_xml)
			{
				PortType port_type = PortTypeConverter::toEnum(outport_xml->GetText());
				normal_dict[PortDirection::out].push_back(port_type);

				outport_xml = outport_xml->NextSiblingElement("port_type");
			}
			// router pipes
			XMLElement* router_pipe_xml = block_xml->FirstChildElement("router_pipes");
			if (router_pipe_xml)
			{
				XMLElement* pipe_xml = router_pipe_xml->FirstChildElement("port_type");

				while (pipe_xml)
				{
					PortType port_type = PortTypeConverter::toEnum(pipe_xml->GetText());
					router_dict.push_back(port_type);

					pipe_xml = pipe_xml->NextSiblingElement("port_type");
				}
			}
			// array
			XMLElement* array_xml = block_xml->FirstChildElement("array")->FirstChildElement("row");
			while (array_xml)
			{
				string tmp = array_xml->GetText();
				vector<string> tmp_row_string = Util::splitString(tmp, " ");
				vector<Bool> tmp_row_bool(tmp_row_string.size());

				std::transform(begin(tmp_row_string), end(tmp_row_string), begin(tmp_row_bool), [&](string s_)
				{
					return s_ == block_type_s;
				});

				position.push_back(tmp_row_bool);

				array_xml = array_xml->NextSiblingElement("row");
			}
			// build
			_arch_para.arch_block_type[block_type] = ArchBlock{ normal_dict, router_dict, position };

			block_xml = block_xml->NextSiblingElement("block");
		}
	}

	auto Para::archNetXmlRead(XMLElement* net_xml_) -> void
	{
		XMLElement* wire_net_config_xml = net_xml_->FirstChildElement("WireNetConfig");
		if (wire_net_config_xml)
			archWireNetXmlRead(wire_net_config_xml);
		XMLElement* segment_net_config_xml = net_xml_->FirstChildElement("SegmentNetConfig");
		if (segment_net_config_xml)
			archSegmentNetXmlRead(segment_net_config_xml);
	}

	auto Para::archWireNetXmlRead(XMLElement* wire_net_config_xml_) -> void
	{
		XMLElement* wire_net_xml = wire_net_config_xml_->FirstChildElement("WireNet");
		while (wire_net_xml)
		{
			WireNetConfigMode mode = WireNetConfigModeConverter::toEnum(wire_net_xml->FindAttribute("config_mode")->Value());
			if (mode == WireNetConfigMode::way4)
			{
				auto [wire_type, block_type, in_ports, out_ports] = archWireNetMeshXmlRead(wire_net_xml);
				const ArchBlock& arch_block = _arch_para.arch_block_type[block_type];
				auto ptr = new WireNet<WireNetConfigMode::way4>{ wire_type, block_type, in_ports, out_ports, arch_block };
				_arch_para.wire_nets[WireNetConfigMode::way4].push_back(dynamic_cast<WireNetInterface*>(ptr));
			}
			if (mode == WireNetConfigMode::way8)
			{
				auto[wire_type, block_type, in_ports, out_ports] = archWireNetMeshXmlRead(wire_net_xml);
				const ArchBlock& arch_block = _arch_para.arch_block_type[block_type];
				auto ptr = new WireNet<WireNetConfigMode::way8>{ wire_type, block_type, in_ports, out_ports, arch_block };
				_arch_para.wire_nets[WireNetConfigMode::way8].push_back(dynamic_cast<WireNetInterface*>(ptr));
			}
			if (mode == WireNetConfigMode::way4hop1)
			{
				auto[wire_type, block_type, in_ports, out_ports] = archWireNetMeshXmlRead(wire_net_xml);
				const ArchBlock& arch_block = _arch_para.arch_block_type[block_type];
				auto ptr = new WireNet<WireNetConfigMode::way4hop1>{ wire_type, block_type, in_ports, out_ports, arch_block };
				_arch_para.wire_nets[WireNetConfigMode::way4hop1].push_back(dynamic_cast<WireNetInterface*>(ptr));
			}
			if (mode == WireNetConfigMode::way4hop2)
			{
				auto[wire_type, block_type, in_ports, out_ports] = archWireNetMeshXmlRead(wire_net_xml);
				const ArchBlock& arch_block = _arch_para.arch_block_type[block_type];
				auto ptr = new WireNet<WireNetConfigMode::way4hop2>{ wire_type, block_type, in_ports, out_ports, arch_block };
				_arch_para.wire_nets[WireNetConfigMode::way4hop2].push_back(dynamic_cast<WireNetInterface*>(ptr));
			}
			if (mode == WireNetConfigMode::full_connect)
				_arch_para.wire_nets[WireNetConfigMode::full_connect].push_back(archWireNetFullConnectXmlRead(wire_net_xml));
			if (mode == WireNetConfigMode::multi_connect)
				_arch_para.wire_nets[WireNetConfigMode::multi_connect].push_back(archWireNetMultiConnectXmlRead(wire_net_xml));

			wire_net_xml = wire_net_xml->NextSiblingElement("WireNet");
		}
	}

	auto Para::archWireNetMeshXmlRead(XMLElement* wire_net_xml_) ->tuple<WireType, BlockType, vector<PortIndex>, vector<PortIndex>>
	{
		WireType wire_type = WireTypeConverter::toEnum(wire_net_xml_->FirstChildElement("config")->FindAttribute("wire_type")->Value());
		BlockType block_type = BlockTypeConverter::toEnum(wire_net_xml_->FirstChildElement("config")->FindAttribute("block_type")->Value());
		vector<PortIndex> in_ports;
		vector<PortIndex> out_ports;
		// out
		XMLElement* outport_xml = wire_net_xml_->FirstChildElement("out_port");
		while (outport_xml)
		{
			out_ports.emplace_back(PortIndex::stringAnalysis(outport_xml->FindAttribute("port_index")->Value()));
			outport_xml = outport_xml->NextSiblingElement("out_port");
		}
		// in
		XMLElement* inport_xml = wire_net_xml_->FirstChildElement("in_port");
		while (inport_xml)
		{
			in_ports.emplace_back(PortIndex::stringAnalysis(inport_xml->FindAttribute("port_index")->Value()));
			inport_xml = inport_xml->NextSiblingElement("in_port");
		}
		// build
		return std::make_tuple(wire_type, block_type, in_ports, out_ports);
	}

	auto Para::archWireNetFullConnectXmlRead(XMLElement* wire_net_xml_) -> WireNetInterface*
	{
		WireType wire_type = WireTypeConverter::toEnum(wire_net_xml_->FirstChildElement("config")->FindAttribute("wire_type")->Value());
		BlockType block_type = BlockTypeConverter::toEnum(wire_net_xml_->FirstChildElement("config")->FindAttribute("block_type")->Value());
		vector<PortIndex> in_ports;
		vector<PortIndex> out_ports;
		vector<Cord> positions;
		// out
		XMLElement* outport_xml = wire_net_xml_->FirstChildElement("out_port");
		while (outport_xml)
		{
			out_ports.emplace_back(PortIndex::stringAnalysis(outport_xml->FindAttribute("port_index")->Value()));
			outport_xml = outport_xml->NextSiblingElement("out_port");
		}
		// in
		XMLElement* inport_xml = wire_net_xml_->FirstChildElement("in_port");
		while (inport_xml)
		{
			in_ports.emplace_back(PortIndex::stringAnalysis(inport_xml->FindAttribute("port_index")->Value()));
			inport_xml = inport_xml->NextSiblingElement("in_port");
		}
		// position
		XMLElement* pos_xml = wire_net_xml_->FirstChildElement("block_pos");
		while (pos_xml)
		{
			positions.emplace_back(Cord::stringAnalysis(pos_xml->FindAttribute("block_pos")->Value()));
			pos_xml = pos_xml->NextSiblingElement("block_pos");
		}
		// build
		auto ptr = new WireNet<WireNetConfigMode::full_connect>{ wire_type, block_type, in_ports, out_ports, positions };
		return dynamic_cast<WireNetInterface*>(ptr);
	}

	auto Para::archWireNetMultiConnectXmlRead(XMLElement* wire_net_xml_) -> WireNetInterface*
	{
		WireType wire_type = WireTypeConverter::toEnum(wire_net_xml_->FirstChildElement("config")->FindAttribute("type")->Value());
		vector<PortPos> source_pos;
		vector<PortPos> target_pos;
		// source_pos
		XMLElement* source_xml = wire_net_xml_->FirstChildElement("source");
		while (source_xml)
		{
			source_pos.emplace_back(PortPos::stringAnalysis(source_xml->FindAttribute("receive")->Value()));
			source_xml = source_xml->NextSiblingElement("source");
		}
		// target_pos
		XMLElement* target_xml = wire_net_xml_->FirstChildElement("target");
		while (target_xml)
		{
			target_pos.emplace_back(PortPos::stringAnalysis(target_xml->FindAttribute("send")->Value()));
			target_xml = target_xml->NextSiblingElement("target");
		}
		// build
		auto ptr = new WireNet<WireNetConfigMode::multi_connect>{ wire_type, source_pos, target_pos };
		return dynamic_cast<WireNetInterface*>(ptr);
	}

	auto Para::archSegmentNetXmlRead(XMLElement* segment_net_config_xml_) -> void
	{
		XMLElement* segment_net_xml = segment_net_config_xml_->FirstChildElement("SegmentNet");
		while (segment_net_xml)
		{
			WireType wire_type = WireTypeConverter::toEnum(segment_net_xml->FirstChildElement("config")->FindAttribute("type")->Value());
			uint channel_num = std::stoi(segment_net_xml->FirstChildElement("config")->FindAttribute("channel")->Value());
			vector<PortPos> source_pos;
			vector<PortPos> target_pos;
			// source_pos
			XMLElement* source_xml = segment_net_xml->FirstChildElement("Segment")->FirstChildElement("source");
			while (source_xml)
			{
				source_pos.emplace_back(PortPos::stringAnalysis(source_xml->FindAttribute("receive")->Value()));
				source_xml = source_xml->NextSiblingElement("source");
			}
			// target_pos
			XMLElement* target_xml = segment_net_xml->FirstChildElement("Segment")->FirstChildElement("target");
			while (target_xml)
			{
				target_pos.emplace_back(PortPos::stringAnalysis(target_xml->FindAttribute("send")->Value()));
				target_xml = target_xml->NextSiblingElement("target");
			}
			// build
			_arch_para.segment_net.emplace_back(wire_type, channel_num, source_pos, target_pos);

			segment_net_xml = segment_net_xml->NextSiblingElement("SegmentNet");
		}
	}
}
