#include "HgraSystem.h"

using namespace Simulator::Array;

HgraArray::HgraArray(const Simulator::AppGraph& app_graph) : bridge(Bridge(app_graph))
{
	const auto& system_para = Preprocess::Para::getInstance()->getArrayPara();
	//clk = 0;
	if (system_para.stall_mode == stallType::none) {
		#define stall
	}
	const vector<AppNode>& app_nodes = app_graph.getSimNodes();
	uint lc_index = 0, pe_index = 0, ls_index = 0;
	for (auto& every_node : app_nodes)
	{
		if (every_node.type == NodeType::pe)
		{
			//			uint index = every_node.type_num;
			type_index temp;
			temp.type = NodeType::pe;
			temp.index = every_node.type_num;
			//	temp.order = 
			config_order.push_back(temp);

			auto ptr = new Processing_element(system_para, pe_index);
			pe_map[pe_index++] = ptr;
			bridge.inputFunRegister(every_node.type, ptr->getAttr()->index,
				std::bind(&Processing_element::getInput, ptr, std::placeholders::_1, std::placeholders::_2));
			bridge.bpFunRegister(every_node.type, ptr->getAttr()->index,
				std::bind(&Processing_element::getBp, ptr, std::placeholders::_1, std::placeholders::_2));
		}
		else if (every_node.type == NodeType::ls)
		{
			type_index temp;
			temp.type = NodeType::ls;
			temp.index = every_node.type_num;
			config_order.push_back(temp);

			auto ptr = new Loadstore_element(system_para, ls_index, lsu);
			lse_map[ls_index] = ptr;
			bridge.inputFunRegister(every_node.type, ptr->getAttr()->index,
				std::bind(&Loadstore_element::getInput, ptr, std::placeholders::_1, std::placeholders::_2));
			bridge.bpFunRegister(every_node.type, ptr->getAttr()->index,
				std::bind(&Loadstore_element::getBp, ptr, std::placeholders::_1, std::placeholders::_2));

			auto dict = Preprocess::DFG::getInstance()->getDictionary();
			auto ls_vec = dict.find(Simulator::NodeType::ls)->second;
			auto attribution = dynamic_cast<const Preprocess::DFGNode<Simulator::NodeType::ls>*>(ls_vec[ls_index]);
			//×¢ÒâSEAµÄindexÎªÆ¥ÅäµÄSEDµÄindex-1£¬ÍØÆËÅÅÐòÊ±Ð¡ÐÄ
			if (attribution->ls_mode == LSMode::store_data)
			{
				const auto cp_ptr = lse_map[ls_index - 1];
				ptr->seAttach(cp_ptr);
			}
			++ls_index;
		}
		else if (every_node.type == NodeType::lc)
		{

			type_index temp;
			temp.type = NodeType::lc;
			temp.index = every_node.type_num;
			config_order.push_back(temp);

			auto ptr = new LoopControl(system_para, lc_index);
			lc_map[lc_index++] = ptr;
			bridge.inputFunRegister(every_node.type, ptr->getAttr()->index,
				std::bind(&LoopControl::getInput, ptr, std::placeholders::_1, std::placeholders::_2));
			bridge.bpFunRegister(every_node.type, ptr->getAttr()->index,
				std::bind(&LoopControl::getBp, ptr, std::placeholders::_1, std::placeholders::_2));
		}
	}
	lsu = new DRAMSim::Lsu(system_para, lse_map);
	mem = new DRAMSim::MultiChannelMemorySystem("./resource/input/DRAMSimini/DDR3_micron_16M_8B_x8_sg15.ini", "./resource/input/DRAMSimini/system.ini", ".", "example_app", 16384);
	DRAMSim::TransactionCompleteCB* read_cb = new Callback<DRAMSim::Cache, void, unsigned, uint64_t, uint64_t>(&(*(lsu->cache)), &DRAMSim::Cache::mem_read_complete);
	DRAMSim::TransactionCompleteCB* write_cb = new Callback<DRAMSim::Cache, void, unsigned, uint64_t, uint64_t>(&(*(lsu->cache)), &DRAMSim::Cache::mem_write_complete);
	mem->RegisterCallbacks(read_cb, write_cb, power_callback);
	lsu->AttachMem(mem);
	for (auto& lse : lse_map)
		lse.second->attachLsu(lsu);
	match_set = new Match_set(system_para, lse_map);
}

HgraArray::~HgraArray()
{
	const auto& system_para = Preprocess::Para::getInstance()->getArrayPara();
	for (auto iter = pe_map.begin(); iter != pe_map.end(); ++iter)
		delete iter->second;

	for (auto iter = lse_map.begin(); iter != lse_map.end(); ++iter)
		delete iter->second;

	for (auto iter = lc_map.begin(); iter != lc_map.end(); ++iter)
		delete iter->second;

	delete match_set;
	delete lsu;
	delete mem;
}
void HgraArray::initIndex()
{
	uint pe_index = 0, ls_index = 0, lc_index = 0;
	for (uint i = 0; i <= config_order.size() - 1; i++) {
		if (config_order[i].type == NodeType::pe) {
			order2index.insert({ i,{NodeType::pe,pe_index } });
			index2order.insert({ {NodeType::pe,config_order[i].index},pe_index });
			ele2order.insert({ {NodeType::pe,config_order[i].index},i });
			pe_index++;
		}
		if (config_order[i].type == NodeType::lc) {
			order2index.insert({ i,{NodeType::lc,lc_index } });
			index2order.insert({ {NodeType::lc,config_order[i].index },lc_index });
			ele2order.insert({ {NodeType::lc,config_order[i].index},i });
			lc_index++;
		}
		if (config_order[i].type == NodeType::ls) {
			order2index.insert({ i,{NodeType::ls,ls_index} });
			index2order.insert({ {NodeType::ls,config_order[i].index },ls_index });
			ele2order.insert({ {NodeType::ls,config_order[i].index},i });
			ls_index++;
		}
	}
}
void HgraArray::printStall(type_index type_in, uint port)
{
	Bridge::Location self_location{ type_in.type,type_in.index,port };
	if (ClkDomain::getInstance()->getClk() >= Debug::getInstance()->print_file_begin && ClkDomain::getInstance()->getClk() < Debug::getInstance()->print_file_end) {
		if (bridge._bp_temporary_buffer.find(self_location) != bridge._bp_temporary_buffer.end()) {
			if (bridge._bp_temporary_buffer[self_location] == false) {
				for (const auto& i : bridge.stall_note[self_location]) { Debug::getInstance()->getPortFile() << maps[int(self_location.type)] << self_location.node_index << "." << self_location.port_index << " stall reason is " << maps[int(i.type)] << i.node_index << "." << i.port_index << std::endl; }
			}
		}
	}
}
void HgraArray::nextStep1(type_index type_in,uint port)
{
	const auto& system_para = Preprocess::Para::getInstance()->getArrayPara();
		const vector<Simulator::Bridge::Location>& nextLocaVec = bridge.findNextNode(type_in.type, type_in.index, port);
		if (nextLocaVec.size() != 0) {
			for (auto& nextLo : nextLocaVec) {
				if (nextLo.type == NodeType::pe) {
#ifdef stall					
					if (ele2order[{NodeType::pe, nextLo.node_index}] < ele2order[{type_in.type, type_in.index}]
						&& pe_map[index2order[{NodeType::pe, nextLo.node_index}]]->input_port[nextLo.port_index].valid)//成环之后的内部循环
					{
						pe_map[index2order[{NodeType::pe, nextLo.node_index}]]->stall_one = true;//如果是环状结构,由于仿真顺序的限制,此时不能拿step1进的数做运算
					}
#endif // stall
					if (pe_map[index2order[{NodeType::pe, nextLo.node_index}]]->getAttr()->input_bypass == InputBypass::bypass)
						pe_map[index2order[{NodeType::pe, nextLo.node_index}]]->controlBlock();
					pe_map[index2order[{NodeType::pe, nextLo.node_index}]]->simStep1(nextLo.port_index);
				}
				else if (nextLo.type == NodeType::ls) {
					lse_map[index2order[{NodeType::ls, nextLo.node_index}]]->simStep1(nextLo.port_index);
					//								lse_map[index2order[{NodeType::ls, nextLo.node_index}]]->lseReset();
				}
				else if (nextLo.type == NodeType::lc) {
//#ifdef stall		if (nextLo.port_index < reg_input.size()) {
//						if (ele2order[{nextLo.type, nextLo.node_index}] < ele2order[{type_in.type, type_in.index}]
//							&& lc_map[index2order[{nextLo.type, nextLo.node_index}]]->reg_input[nextLo.port_index].valid)//成环之后的内部循环
//						{
//							lc_map[index2order[{nextLo.type, nextLo.node_index}]]->stall_one = true;//如果是环状结构,由于仿真顺序的限制,此时不能拿step1进的数做运算
//						}
//					}
//					else {
//						if (ele2order[{nextLo.type, nextLo.node_index}] < ele2order[{type_in.type, type_in.index}]
//							&& lc_map[index2order[{nextLo.type, nextLo.node_index}]]->end_input[nextLo.port_index-system_para.lc_regin_num].valid)//成环之后的内部循环
//						{
//							lc_map[index2order[{nextLo.type, nextLo.node_index}]]->stall_one = true;//如果是环状结构,由于仿真顺序的限制,此时不能拿step1进的数做运算
//						}
//					}
//#endif // stall
					lc_map[index2order[{NodeType::lc, nextLo.node_index}]]->simStep1(nextLo.port_index);
				}
			}
		}
}
void HgraArray::reverseStep1(type_index type_in, uint port)
{
	const auto& system_para = Preprocess::Para::getInstance()->getArrayPara();
	const vector<Simulator::Bridge::Location>& nextLocaVec = bridge.findNextNode(type_in.type, type_in.index, port);
	if (nextLocaVec.size() != 0) {
		for (auto& nextLo : nextLocaVec) {
			if (ele2order[{nextLo.type, nextLo.node_index}] < ele2order[{type_in.type, type_in.index}]) {
				if (nextLo.type == NodeType::pe) {
					pe_map[index2order[{NodeType::pe, nextLo.node_index}]]->simStep1(nextLo.port_index);
					pe_map[index2order[{NodeType::pe, nextLo.node_index}]]->simBp();
				}
				else if (nextLo.type == NodeType::ls) {
					lse_map[index2order[{NodeType::ls, nextLo.node_index}]]->simStep1(nextLo.port_index);
					lse_map[index2order[{NodeType::ls, nextLo.node_index}]]->simBp();
					//								lse_map[index2order[{NodeType::ls, nextLo.node_index}]]->lseReset();
				}
				else if (nextLo.type == NodeType::lc) {
					lc_map[index2order[{NodeType::lc, nextLo.node_index}]]->simStep1(nextLo.port_index);
					lc_map[index2order[{NodeType::lc, nextLo.node_index}]]->simBp();
				}
			}
		}
	}
}
bool HgraArray::sendOutput(Simulator::NodeType type, uint index)
{
	if (type == NodeType::pe)
	{
		for (uint i = 0; i < pe_map[index]->output_port.size(); i++) {
			bridge.setNextInput(pe_map[index]->output_port[i], NodeType::pe, pe_map[index]->getAttr()->index, i);
			if (clk < Debug::getInstance()->print_screen_end && pe_map[index]->output_port[i].valid)
			{
				if (pe_map[index]->output_port[i].last)
					data_flow[make_tuple(NodeType::pe, pe_map[index]->getAttr()->index,i)].push_back(make_tuple(clk, pe_map[index]->output_port[i].value_data, true));
				else
					data_flow[make_tuple(NodeType::pe, pe_map[index]->getAttr()->index,i)].push_back(make_tuple(clk, pe_map[index]->output_port[i].value_data, false));
			}
		}
	}
	else if (type == NodeType::lc)
	{
		for (uint i = 0; i < lc_map[index]->lc_output.size(); i++) {
			bridge.setNextInput(lc_map[index]->lc_output[i], NodeType::lc, lc_map[index]->getAttr()->index, i);
			if (clk<Debug::getInstance()->print_screen_end &&lc_map[index]->lc_output[i].valid)
			{
				if (lc_map[index]->lc_output[i].last)
					data_flow[make_tuple(NodeType::lc, lc_map[index]->getAttr()->index, i)].push_back(make_tuple(clk, lc_map[index]->lc_output[i].value_data, true));
				else
					data_flow[make_tuple(NodeType::lc, lc_map[index]->getAttr()->index, i)].push_back(make_tuple(clk, lc_map[index]->lc_output[i].value_data, false));
			}
		}

		if (lc_map[index]->attribution->outermost)
		{
			if (lc_map[index]->lc_output[3].last && lc_map[index]->lc_output[3].valid)
				return true;
		}
	}
	else if (type == NodeType::ls) {
		bridge.setNextInput(lse_map[index]->output_port_2array, NodeType::ls, lse_map[index]->getAttr()->index, 0);
		if (clk < Debug::getInstance()->print_screen_end && lse_map[index]->output_port_2array.valid)
		{
			if (lse_map[index]->output_port_2array.last)
				data_flow[make_tuple(NodeType::ls, lse_map[index]->getAttr()->index, 0)].push_back(make_tuple(clk, lse_map[index]->output_port_2array.value_data, true));
			else
				data_flow[make_tuple(NodeType::ls, lse_map[index]->getAttr()->index, 0)].push_back(make_tuple(clk, lse_map[index]->output_port_2array.value_data, false));
		}
	}

	return false;
}

//È±ÉÙ·ÂÕæË³ÐòÐÅÏ¢
void HgraArray::run()
{
	const auto& system_para = Preprocess::Para::getInstance()->getArrayPara();
	initIndex();

	if (system_para.stall_mode == stallType::none) {
		while (ClkDomain::getInstance()->getClk() < system_para.maxclk)
		{
			for (auto& i : bridge.stall_note) {
				i.second.clear();
			}
			clk = ClkDomain::getInstance()->getClk();
			bool task_finish = false;
			if (ClkDomain::getInstance()->getClk() == 0)
			{
				DEBUG_ASSERT(config_order[0].type == NodeType::lc && config_order[0].index == 0);
				const Port_inout begin(false, 0, true, true, false);
				lc_map[0]->getInput(0, begin);
			}
			bridge.InitBp();


			for (int i = config_order.size() - 1; i >= 0; i--)
			{
				if (config_order[i].type == NodeType::pe) {
					for (uint port = 0; port < system_para.data_outport_breadth + system_para.bool_outport_breadth; ++port) {
						Bridge::Location self_location{ NodeType::pe,config_order[i].index,port };
						if (bridge._bp_temporary_buffer.find(self_location) != bridge._bp_temporary_buffer.end()) {
							bridge.recvOneBp({ self_location.type, self_location.node_index, self_location.port_index }, bridge._bp_temporary_buffer[self_location]);
						}
					}
					pe_map[order2index[i].second]->simStep3();
					pe_map[order2index[i].second]->simStep2();
					if (sendOutput(config_order[i].type, order2index[i].second))        //·µ»Ø1Ê±ÈÎÎñ½áÊø
					{
						task_finish = true;
					}
					for (uint port = 0; port < system_para.data_outport_breadth + system_para.bool_outport_breadth; ++port) {
						nextStep1(config_order[i], port);
					}
					//if()
			//		pe_map[order2index[i].second]->simStep2();//´ÓÐ´µÄµÚÒ»¸öÀ´·Ã
					pe_map[order2index[i].second]->simBp();
					for (uint port = 0; port < system_para.bool_inport_breadth+ system_para.data_inport_breadth; port++)
						bridge.setBp(pe_map[order2index[i].second]->thispe_bp[port], NodeType::pe, pe_map[order2index[i].second]->getAttr()->index, port);
				}
				else if (config_order[i].type == NodeType::ls) {
					for (uint port = 0; port < system_para.le_dataout_breadth + system_para.le_boolout_breadth; ++port) {
						Bridge::Location self_location{ NodeType::ls,config_order[i].index,port };
						if (bridge._bp_temporary_buffer.find(self_location) != bridge._bp_temporary_buffer.end()) {
							bridge.recvOneBp({ self_location.type, self_location.node_index, self_location.port_index }, bridge._bp_temporary_buffer[self_location]);
						}
					}
					lse_map[order2index[i].second]->simStep2();
					if (sendOutput(config_order[i].type, order2index[i].second))        //·µ»Ø1Ê±ÈÎÎñ½áÊø
					{
						task_finish = true;
					}
					for (uint port = 0; port < system_para.le_dataout_breadth + system_para.le_boolout_breadth; ++port) {
						nextStep1(config_order[i], port);
					}
					lse_map[order2index[i].second]->simBp();
					for (uint port = 0; port < lse_map[order2index[i].second]->this_bp.size(); port++)
						bridge.setBp(lse_map[order2index[i].second]->this_bp[port], NodeType::ls, lse_map[order2index[i].second]->getAttr()->index, port);
				}
				else if (config_order[i].type == NodeType::lc) {
					for (uint port = 0; port < system_para.lc_output_num; ++port) {
						Bridge::Location self_location{ NodeType::lc,config_order[i].index,port };
						if (bridge._bp_temporary_buffer.find(self_location) != bridge._bp_temporary_buffer.end()) {
							bridge.recvOneBp({ self_location.type, self_location.node_index, self_location.port_index }, bridge._bp_temporary_buffer[self_location]);

						}
					}
					lc_map[order2index[i].second]->simStep3();
					if (sendOutput(config_order[i].type, order2index[i].second))        //·µ»Ø1Ê±ÈÎÎñ½áÊø
					{
						task_finish = true;
					}

					for (uint port = 0; port < system_para.lc_output_num; ++port) {
						nextStep1(config_order[i], port);
					}
					lc_map[order2index[i].second]->simStep2();//´ÓÐ´µÄµÚÒ»¸öÀ´·Ã
					if (order2index[i].second == 0) {
						lc_map[order2index[i].second]->simStep1(0);///¶ÔÓÚÍ·²¿lc£¬ÐèÒª½«Íâ²¿regÊäÈë
					}
					lc_map[order2index[i].second]->simBp();
					for (uint port = 0; port < lc_map[order2index[i].second]->thisbp_reg.size(); port++)
						bridge.setBp(lc_map[order2index[i].second]->thisbp_reg[port], NodeType::lc, lc_map[order2index[i].second]->getAttr()->index, port);
					for (uint port = 0; port < lc_map[order2index[i].second]->thisbp_end.size(); port++)
						bridge.setBp(lc_map[order2index[i].second]->thisbp_end[port], NodeType::lc, lc_map[order2index[i].second]->getAttr()->index, port + lc_map[order2index[i].second]->thisbp_reg.size());

				}

			}


			if (system_para.attach_memory) {
				lsu->update();//lsu->updateÒª·ÅÔÚ×îÉÏÃæ£¬ÕâÑùlsu°ÑÊä³ö·ÅÔÚ¶Ë¿ÚÉÏ¾ÍÄÜÊÕµ½
			}



			for (int i = config_order.size() - 1; i >= 0; i--) {
				if (config_order[i].type == NodeType::pe) {
					pe_map[order2index[i].second]->bufferprint();
				}
				else if (config_order[i].type == NodeType::ls) {
					lse_map[order2index[i].second]->printBuffer();
				}
				else if (config_order[i].type == NodeType::lc) {
					lc_map[order2index[i].second]->buffer_print();
				}
				else { DEBUG_ASSERT(false); }
			}


			for (int i = config_order.size() - 1; i >= 0; i--) {
				if (config_order[i].type == NodeType::pe) {
					for (uint port = 0; port < system_para.data_outport_breadth + system_para.bool_outport_breadth; ++port) {
						printStall(config_order[i], port);
					}
					pe_map[order2index[i].second]->print();
					pe_map[order2index[i].second]->wireReset();
				}
				else if (config_order[i].type == NodeType::ls) {
					for (uint port = 0; port < system_para.le_dataout_breadth + system_para.le_boolout_breadth; ++port) {
						printStall(config_order[i], port);
					}
					lse_map[order2index[i].second]->print();
					lse_map[order2index[i].second]->wireReset();
				}
				else if (config_order[i].type == NodeType::lc) {
					for (uint port = 0; port < system_para.lc_output_num; ++port) {
						printStall(config_order[i], port);
					}
					size_t tupid = 0;
					//get<tupid>(three_map)[order2index[i].second]->print();
					lc_map[order2index[i].second]->print();
					lc_map[order2index[i].second]->wireReset();

				}
				else { DEBUG_ASSERT(false); }
			}

			if (task_finish) {
				Debug::getInstance()->getPortFile() << "clk" << clk << std::endl;
				break;
			}

			ClkDomain::getInstance()->selfAdd();
		}
	}

	else if (system_para.stall_mode == stallType::inbuffer_stall) 
	{
		while (ClkDomain::getInstance()->getClk() < system_para.maxclk)
		{
			for (auto& i : bridge.stall_note) {
				i.second.clear();
			}
			clk = ClkDomain::getInstance()->getClk();
			bool task_finish = false;
			if (clk == 0)
			{
				DEBUG_ASSERT(config_order[0].type == NodeType::lc && config_order[0].index == 0);
				const Port_inout begin(false, 0, true, true, false);
				lc_map[0]->getInput(0, begin);
			}
			bridge.InitBp();
			for (int i= 0;i <= config_order.size()-1; i++)
			{
				if (config_order[i].type == NodeType::pe) {
					pe_map[order2index[i].second]->update();
					//pe_map[order2index[i].second]->simStep3();
					//if (sendOutput(config_order[i].type, order2index[i].second))        //·µ»Ø1Ê±ÈÎÎñ½áÊø
					//{
					//	task_finish = true;
					//}
					//for (uint port = 0; port < system_para.data_outport_breadth + system_para.bool_outport_breadth; ++port) {
					//	nextStep1(config_order[i], port);
					//}
					//pe_map[order2index[i].second]->simStep2();//´ÓÐ´µÄµÚÒ»¸öÀ´·Ã
					//pe_map[order2index[i].second]->simBp();
					sendOutput(config_order[i].type, order2index[i].second);
					for (uint port = 0; port < system_para.data_outport_breadth + system_para.bool_outport_breadth; ++port) {
						reverseStep1(config_order[i], port);
					}
					for (uint port = 0; port < system_para.bool_inport_breadth + system_para.data_inport_breadth; port++)
						bridge.setBp(pe_map[order2index[i].second]->thispe_bp[port], NodeType::pe, pe_map[order2index[i].second]->getAttr()->index, port);
				}
				else if (config_order[i].type == NodeType::ls) {
					lse_map[order2index[i].second]->update();
					//lse_map[order2index[i].second]->simStep2();
					if (lse_map[order2index[i].second]->getAttr()->ls_mode != LSMode::store_addr) {
						if (lse_map[order2index[i].second]->getAttr()->ls_mode == LSMode::store_data) {
							sendOutput(config_order[i].type, order2index[i].second - 1);       //此时将上一个ls的输出发送出去(由于仿真顺序的原因)
							sendOutput(config_order[i].type, order2index[i].second);        //·µ»Ø1Ê±ÈÎÎñ½áÊø
						}
						else {
							sendOutput(config_order[i].type, order2index[i].second);
						}
					}
				
					for (uint port = 0; port < system_para.le_dataout_breadth + system_para.le_boolout_breadth; ++port) {
						reverseStep1(config_order[i], port);
					}
					//lse_map[order2index[i].second]->simBp();
					for (uint port = 0; port < lse_map[order2index[i].second]->this_bp.size(); port++)
						bridge.setBp(lse_map[order2index[i].second]->this_bp[port], NodeType::ls, lse_map[order2index[i].second]->getAttr()->index, port);
				}					
				else if (config_order[i].type == NodeType::lc) {
					lc_map[order2index[i].second]->update();
					if (sendOutput(config_order[i].type, order2index[i].second))        //·µ»Ø1Ê±ÈÎÎñ½áÊø
					{
						task_finish = true;
					}
					for (uint port = 0; port < system_para.lc_output_num; ++port) {
						reverseStep1(config_order[i], port);
					}
					//lc_map[order2index[i].second]->simStep3();
					//if (sendOutput(config_order[i].type, order2index[i].second))        //·µ»Ø1Ê±ÈÎÎñ½áÊø
					//{
					//	task_finish = true;
					//}

					//for (uint port = 0; port < system_para.lc_output_num; ++port) {
					//	nextStep1(config_order[i], port);
					//}
					//lc_map[order2index[i].second]->simStep2();//´ÓÐ´µÄµÚÒ»¸öÀ´·Ã
					//if (order2index[i].second == 0) {
					//	lc_map[order2index[i].second]->simStep1(0);///¶ÔÓÚÍ·²¿lc£¬ÐèÒª½«Íâ²¿regÊäÈë
					//}
					//lc_map[order2index[i].second]->simBp();
					for (uint port = 0; port < lc_map[order2index[i].second]->thisbp_reg.size(); port++)
						bridge.setBp(lc_map[order2index[i].second]->thisbp_reg[port], NodeType::lc, lc_map[order2index[i].second]->getAttr()->index, port);
					for (uint port = 0; port < lc_map[order2index[i].second]->thisbp_end.size(); port++)
						bridge.setBp(lc_map[order2index[i].second]->thisbp_end[port], NodeType::lc, lc_map[order2index[i].second]->getAttr()->index, port + lc_map[order2index[i].second]->thisbp_reg.size());
				}
					

//				sendBpOut(config_order[i].type, config_order[i].index);//ÕâÀïlcºÍlseÒ²ÊÇÒ»ÑùµÄsendBpÂð

				//if (sendOutput(config_order[i].type, order2index[i].second))        //·µ»Ø1Ê±ÈÎÎñ½áÊø//setAllBpËÆºõÃ»ÓÐÊ¹ÓÃ///////////////////////////////
				//{
				//	task_finish = true;
				//}
			}
			if (system_para.attach_memory) {
				lsu->update();//lsu->updateÒª·ÅÔÚ×îÉÏÃæ£¬ÕâÑùlsu°ÑÊä³ö·ÅÔÚ¶Ë¿ÚÉÏ¾ÍÄÜÊÕµ½
			}

			bridge.setAllBp();

			for (int i = config_order.size() - 1; i >= 0; i--) {
				if (config_order[i].type == NodeType::pe) {
					pe_map[order2index[i].second]->bufferprint();
				}
				else if (config_order[i].type == NodeType::ls) {
					lse_map[order2index[i].second]->printBuffer();
				}
				else if (config_order[i].type == NodeType::lc) {
					lc_map[order2index[i].second]->buffer_print();
				}
				else { DEBUG_ASSERT(false); }
			}

			bridge.setAllBp();
			for (int i = config_order.size() - 1; i >= 0; i--) {
				if (config_order[i].type == NodeType::pe) {
					for (uint port = 0; port < system_para.data_outport_breadth + system_para.bool_outport_breadth; ++port) {
						printStall(config_order[i], port);
					}
					pe_map[order2index[i].second]->print();
					pe_map[order2index[i].second]->wireReset();
				}
				else if (config_order[i].type == NodeType::ls) {
					for (uint port = 0; port < system_para.le_dataout_breadth + system_para.le_boolout_breadth; ++port) {
						printStall(config_order[i], port);
					}
					lse_map[order2index[i].second]->print();
					lse_map[order2index[i].second]->wireReset();
				}
				else if (config_order[i].type == NodeType::lc) {
					for (uint port = 0; port < system_para.lc_output_num; ++port) {
						printStall(config_order[i], port);
					}
					size_t tupid = 0;
					//get<tupid>(three_map)[order2index[i].second]->print();
					lc_map[order2index[i].second]->print();
					lc_map[order2index[i].second]->wireReset();
				}
				else { DEBUG_ASSERT(false); }
			}


			if (task_finish) {
				Debug::getInstance()->getPortFile() << "clk" << clk << std::endl;
				break;
			}
	//		bridge.setAllBp();
			ClkDomain::getInstance()->selfAdd();
		}
	}
	Debug::getInstance()->getPortFile() << "clk" << clk << std::endl;
	for (const auto& single_out : data_flow) {
		uint pr_ct = 0;
		Debug::getInstance()->getdataFlowFile() << endl;
		Debug::getInstance()->getdataFlowFile() << "------------------------" << maps[int(get<0>(single_out.first))] << get<1>(single_out.first) << " out" << "------------------------" << endl;
		for (const auto& every_cycle : single_out.second) {
			if (get<2>(every_cycle)) {
				Debug::getInstance()->getdataFlowFile() << get<1>(every_cycle) << "(" << get<0>(every_cycle) << ")" << "(last)" << endl;
				pr_ct = 0;
			}
			else {
				Debug::getInstance()->getdataFlowFile() << get<1>(every_cycle) << "(" << get<0>(every_cycle) << ")";
				if (pr_ct == 5) {
					Debug::getInstance()->getdataFlowFile() << endl;
					pr_ct = 0;
				}
				else{
					Debug::getInstance()->getdataFlowFile() << setw(10);
					pr_ct++;
				}
			}
		}
	}
}

void HgraArray::sendBpOut(Simulator::NodeType type, uint index)
{

	if (type == NodeType::pe)
	{
		for (uint i = 0; i < pe_map[index]->thispe_bp.size(); i++)
			bridge.setBp(pe_map[index]->thispe_bp[i], NodeType::pe, pe_map[index]->getAttr()->index, i);
	}
	else if (type == NodeType::lc)
	{
		for (uint i = 0; i < lc_map[index]->thisbp_reg.size(); i++)
			bridge.setBp(lc_map[index]->thisbp_reg[i], NodeType::lc, lc_map[index]->getAttr()->index, i);
		for (uint i = 0; i < lc_map[index]->thisbp_end.size(); i++)
			bridge.setBp(lc_map[index]->thisbp_end[i], NodeType::lc, lc_map[index]->getAttr()->index, i + lc_map[index]->thisbp_reg.size());
	}
	else if (type == NodeType::ls)
	{
		for (uint i = 0; i < lse_map[index]->this_bp.size(); i++)
			bridge.setBp(lse_map[index]->this_bp[i], NodeType::ls, lse_map[index]->getAttr()->index, i);
	}
}


