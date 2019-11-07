#include "HgraSystem.h"

using namespace Simulator::Array;

HgraArray::HgraArray(const Simulator::AppGraph& app_graph) : bridge(Bridge(app_graph))
{
	const auto& system_para = Preprocess::Para::getInstance()->getArrayPara();
	//clk = 0;

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

bool HgraArray::sendOutput(Simulator::NodeType type, uint index)
{
	if (type == NodeType::pe)
	{
		for (uint i = 0; i < pe_map[index]->output_port.size(); i++)
			bridge.setNextInput(pe_map[index]->output_port[i], NodeType::pe, pe_map[index]->getAttr()->index, i);
	}
	else if (type == NodeType::lc)
	{
		for (uint i = 0; i < lc_map[index]->lc_output.size(); i++)
			bridge.setNextInput(lc_map[index]->lc_output[i], NodeType::lc, lc_map[index]->getAttr()->index, i);

		if (lc_map[index]->attribution->outermost)
		{
			if (lc_map[index]->lc_output[3].last && lc_map[index]->lc_output[3].valid)
				return true;
		}
	}
	else if (type == NodeType::ls)
		bridge.setNextInput(lse_map[index]->output_port_2array, NodeType::ls, lse_map[index]->getAttr()->index, 0);

	return false;
}

//È±ÉÙ·ÂÕæË³ÐòÐÅÏ¢
void HgraArray::run()
{
	const auto& system_para = Preprocess::Para::getInstance()->getArrayPara();
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
		uint pe_index = 0, ls_index = 0, lc_index = 0;
		std::map<uint, std::pair<NodeType, uint>> order2index;
		std::map<std::pair<NodeType, uint>, uint> index2order;
		//if (system_para.attach_memory) {
		//	lsu->update();//lsu->updateÒª·ÅÔÚ×îÉÏÃæ£¬ÕâÑùlsu°ÑÊä³ö·ÅÔÚ¶Ë¿ÚÉÏ¾ÍÄÜÊÕµ½
		//}
		for (uint i = 0; i <= config_order.size() - 1; i++) {
			if (config_order[i].type == NodeType::pe) {
				order2index.insert({ i,{NodeType::pe,pe_index } });
				index2order.insert({ {NodeType::pe,config_order[i].index},pe_index });
				pe_index++;
			}
			if (config_order[i].type == NodeType::lc) {
				order2index.insert({ i,{NodeType::lc,lc_index } });
				index2order.insert({ {NodeType::lc,config_order[i].index },lc_index });
				lc_index++;
			}
			if (config_order[i].type == NodeType::ls) {
				order2index.insert({ i,{NodeType::ls,ls_index} });
				index2order.insert({ {NodeType::ls,config_order[i].index },ls_index });
				ls_index++;
			}
		}
//		vector<Port_inout> input_port2_last = pe_map[0]->input_port;
//		vector<Port_inout> input_port2 = pe_map[0]->input_port;
		std::map<Bridge::Location, Bridge::Location> ls_pre;
		//for (int i = config_order.size() - 1; i >= 0; i--) {
		//	if (config_order[i].type == NodeType::ls) {
		//		for (uint port = 0; port < system_para.lse_boolin_breadth + system_para.lse_datain_breadth;port++) {
		//			Bridge::Location self_location{ NodeType::ls,order2index[i].second,port };///////¿¼ÂÇÕâ¸öÊ±ºòlsÖ»ÓÐÒ»¸öÊäÈë
		//			const Bridge::Location preLoc = bridge.findPreNode(NodeType::ls,order2index[i].second,port );
		//			ls_pre[]
		//		}
		//	}
		//}
		for (int i = config_order.size() - 1; i >= 0; i--)
		{
			static const char* maps[7] = { "pe","fg","ls","lv","lc","null","begin" };
			//			DEBUG_ASSERT(input_port2[0].valid == input_port2_last[0].valid);
			//			vector<Port_inout> input_port_last = input_port2;
			if (config_order[i].type == NodeType::pe) {
				for (uint port = 0; port < system_para.data_outport_breadth + system_para.bool_outport_breadth; ++port) {
					Bridge::Location self_location{ NodeType::pe,config_order[i].index,port };
					if (bridge._bp_temporary_buffer.find(self_location) != bridge._bp_temporary_buffer.end()) {
						bridge.recvOneBp({ self_location.type, self_location.node_index, self_location.port_index }, bridge._bp_temporary_buffer[self_location]);
						//		if (bridge._bp_temporary_buffer[self_location] == false) {
						////			static const char* maps[7] = { "pe","fg","ls","lv","lc","null","begin" };
						//			for (const auto& i : bridge.stall_note[self_location]) { Debug::getInstance()->getPortFile() << maps[int(self_location.type)] << self_location.node_index << self_location.port_index << "stall reason is" << maps[int(i.type)] << i.node_index << i.port_index << std::endl; }
						//		}
					}
				}
				pe_map[order2index[i].second]->simStep3();
				if (sendOutput(config_order[i].type, order2index[i].second))        //·µ»Ø1Ê±ÈÎÎñ½áÊø
				{
					task_finish = true;
				}
				//				input_port2 = pe_map[7]->input_port;
								//if (i == config_order.size() - 1) {
								//	lc_map[0]->simall();
								//}
				for (uint port = 0; port < system_para.data_outport_breadth + system_para.bool_outport_breadth; ++port) {
					const vector<Simulator::Bridge::Location>& nextLocaVec = bridge.findNextNode(config_order[i].type, config_order[i].index, port);
					if (nextLocaVec.size() != 0) {
						for (auto& nextLo : nextLocaVec) {
							if (nextLo.type == NodeType::pe) {
								pe_map[index2order[{NodeType::pe, nextLo.node_index}]]->simStep1(nextLo.port_index);
							}
							else if (nextLo.type == NodeType::ls) {
								lse_map[index2order[{NodeType::ls, nextLo.node_index}]]->simStep1(nextLo.port_index);
								//								lse_map[index2order[{NodeType::ls, nextLo.node_index}]]->lseReset();
							}
							else if (nextLo.type == NodeType::lc) {
								lc_map[index2order[{NodeType::lc, nextLo.node_index}]]->simStep1(nextLo.port_index);
							}
						}
					}
				}
				//				pe_map[order2index[i].second]->bufferprint();
				//				pe_map[order2index[i].second]->bufferprint();
				pe_map[order2index[i].second]->simStep2();//´ÓÐ´µÄµÚÒ»¸öÀ´·Ã
				pe_map[order2index[i].second]->simBp();
//				for (uint port = 0; port < pe_map[order2index[i].second]->thispe_bp.size(); port++)
				for (uint port = 0; port < 3; port++)
					bridge.setBp(pe_map[order2index[i].second]->thispe_bp[port], NodeType::pe, pe_map[order2index[i].second]->getAttr()->index, port);
				//				vector<Port_inout> input_port2 = pe_map[7]->input_port;
				//				if((order2index[i].second<4)|| (order2index[i].second>27))
				//				if(order2index[i].second==0)
						//		pe_map[order2index[i].second]->print();
						//		pe_map[order2index[i].second]->wireReset();
				//				pe_map[order2index[i].second]->print();

			}
			else if (config_order[i].type == NodeType::ls) {
				for (uint port = 0; port < system_para.le_dataout_breadth + system_para.le_boolout_breadth; ++port) {
					Bridge::Location self_location{ NodeType::ls,config_order[i].index,port };
					if (bridge._bp_temporary_buffer.find(self_location) != bridge._bp_temporary_buffer.end()) {
						bridge.recvOneBp({ self_location.type, self_location.node_index, self_location.port_index }, bridge._bp_temporary_buffer[self_location]);
						//if (bridge._bp_temporary_buffer[self_location] == false) {
						//	//			static const char* maps[7] = { "pe","fg","ls","lv","lc","null","begin" };
						//	for (const auto& i : bridge.stall_note[self_location]) { Debug::getInstance()->getPortFile() << maps[int(self_location.type)] << self_location.node_index << self_location.port_index << "stall reason is" << maps[int(i.type)] << i.node_index << i.port_index << std::endl; }
						//}
					}
				}
				//				lse_map[order2index[i].second]->printBuffer();//´òÓ¡µÄÊÇÕâ´Îclk¿ªÊ¼µÄbuffer
				lse_map[order2index[i].second]->simStep2();
				if (sendOutput(config_order[i].type, order2index[i].second))        //·µ»Ø1Ê±ÈÎÎñ½áÊø
				{
					task_finish = true;
				}
				//if (i == config_order.size() - 1) {
				//	lc_map[0]->simall();
				//}
				for (uint port = 0; port < system_para.le_dataout_breadth + system_para.le_boolout_breadth; ++port) {
					const vector<Simulator::Bridge::Location>& nextLocaVec = bridge.findNextNode(NodeType::ls, config_order[i].index, port);
					if (nextLocaVec.size() != 0) {
						for (auto& nextLo : nextLocaVec) {
							if (nextLo.type == NodeType::pe) {
								pe_map[index2order[{NodeType::pe, nextLo.node_index}]]->simStep1(nextLo.port_index);
							}
							else if (nextLo.type == NodeType::ls) {
								lse_map[index2order[{NodeType::ls, nextLo.node_index}]]->simStep1(nextLo.port_index);
								//						lse_map[index2order[{NodeType::ls, nextLo.node_index}]]->lseReset();
							}
							else if (nextLo.type == NodeType::lc) {
								lc_map[index2order[{NodeType::lc, nextLo.node_index}]]->simStep1(nextLo.port_index);
							}
						}
					}
				}
				//ls_map[order2index[i].second]->simStep2();//´ÓÐ´µÄµÚÒ»¸öÀ´·Ã
				lse_map[order2index[i].second]->simBp();
				for (uint port = 0; port < lse_map[order2index[i].second]->this_bp.size(); port++)
					bridge.setBp(lse_map[order2index[i].second]->this_bp[port], NodeType::ls, lse_map[order2index[i].second]->getAttr()->index, port);
				//				if ((order2index[i].second < 3) || (order2index[i].second > 7))
						//		lse_map[order2index[i].second]->print();
						//		lse_map[order2index[i].second]->wireReset();
			}
			else if (config_order[i].type == NodeType::lc) {
				for (uint port = 0; port < system_para.lc_output_num; ++port) {
					Bridge::Location self_location{ NodeType::lc,config_order[i].index,port };
					if (bridge._bp_temporary_buffer.find(self_location) != bridge._bp_temporary_buffer.end()) {
						bridge.recvOneBp({ self_location.type, self_location.node_index, self_location.port_index }, bridge._bp_temporary_buffer[self_location]);
						//if (bridge._bp_temporary_buffer[self_location] == false) {
						//	//			static const char* maps[7] = { "pe","fg","ls","lv","lc","null","begin" };
						//	for (const auto& i : bridge.stall_note[self_location]) { Debug::getInstance()->getPortFile() << maps[int(self_location.type)] << self_location.node_index << self_location.port_index << "stall reason is" << maps[int(i.type)] << i.node_index << i.port_index << std::endl; }
						//}
					}
				}
				lc_map[order2index[i].second]->simStep3();
				if (sendOutput(config_order[i].type, order2index[i].second))        //·µ»Ø1Ê±ÈÎÎñ½áÊø
				{
					task_finish = true;
				}
				//if (i == config_order.size() - 1) {
				//	lc_map[0]->simall();
				//}
				for (uint port = 0; port < system_para.lc_output_num; ++port) {
					const vector<Simulator::Bridge::Location>& nextLocaVec = bridge.findNextNode(NodeType::lc, config_order[i].index, port);
					if (nextLocaVec.size() != 0) {
						for (auto& nextLo : nextLocaVec) {
							if (nextLo.type == NodeType::pe) {
								pe_map[index2order[{NodeType::pe, nextLo.node_index}]]->simStep1(nextLo.port_index);
							}
							else if (nextLo.type == NodeType::ls) {
								lse_map[index2order[{NodeType::ls, nextLo.node_index}]]->simStep1(nextLo.port_index);
								//							lse_map[index2order[{NodeType::ls, nextLo.node_index}]]->lseReset();
							}
							else if (nextLo.type == NodeType::lc) {
								lc_map[index2order[{NodeType::lc, nextLo.node_index}]]->simStep1(nextLo.port_index);
							}
						}
					}
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
				//		lc_map[order2index[i].second]->print();
				//		lc_map[order2index[i].second]->wireReset();
//				vector<Port_inout> input_port2_last = input_port2;
			}
			/*			else if (config_order[i].type == NodeType::ls)
							lse_map[order2index[i].second]->update();
						else if (config_order[i].type == NodeType::lc)
							lc_map[order2index[i].second]->update();*/

							/*			sendBpOut(config_order[i].type, order2index[i].second);

										if (sendOutput(config_order[i].type, order2index[i].second))        //·µ»Ø1Ê±ÈÎÎñ½áÊø
										{
											task_finish = true;
										}*/
			auto bpbuffer = bridge._bp_temporary_buffer;
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
			static const char* maps[7] = { "pe","fg","ls","lv","lc","null","begin" };
			if (config_order[i].type == NodeType::pe) {
				for (uint port = 0; port < system_para.data_outport_breadth + system_para.bool_outport_breadth; ++port) {
					Bridge::Location self_location{ NodeType::pe,config_order[i].index,port };
					if (bridge._bp_temporary_buffer.find(self_location) != bridge._bp_temporary_buffer.end()) {
						if (bridge._bp_temporary_buffer[self_location] == false) {
							//			static const char* maps[7] = { "pe","fg","ls","lv","lc","null","begin" };
							for (const auto& i : bridge.stall_note[self_location]) { Debug::getInstance()->getPortFile() << maps[int(self_location.type)] << self_location.node_index << " " << self_location.port_index << " stall reason is " << maps[int(i.type)] << i.node_index << " " << i.port_index << std::endl; }
						}
					}
				}
				pe_map[order2index[i].second]->print();
				pe_map[order2index[i].second]->wireReset();
			}
			else if (config_order[i].type == NodeType::ls) {
				for (uint port = 0; port < system_para.le_dataout_breadth + system_para.le_boolout_breadth; ++port) {
					Bridge::Location self_location{ NodeType::ls,config_order[i].index,port };
					if (bridge._bp_temporary_buffer.find(self_location) != bridge._bp_temporary_buffer.end()) {
						if (bridge._bp_temporary_buffer[self_location] == false) {
							//			static const char* maps[7] = { "pe","fg","ls","lv","lc","null","begin" };
							for (const auto& i : bridge.stall_note[self_location]) { Debug::getInstance()->getPortFile() << maps[int(self_location.type)] << self_location.node_index << self_location.port_index << "stall reason is" << maps[int(i.type)] << i.node_index << i.port_index << std::endl; }
						}
					}
				}
				lse_map[order2index[i].second]->print();
				lse_map[order2index[i].second]->wireReset();
			}
			else if (config_order[i].type == NodeType::lc) {
				for (uint port = 0; port < system_para.lc_output_num; ++port) {
					Bridge::Location self_location{ NodeType::lc,config_order[i].index,port };
					if (bridge._bp_temporary_buffer.find(self_location) != bridge._bp_temporary_buffer.end()) {
						if (bridge._bp_temporary_buffer[self_location] == false) {
							//			static const char* maps[7] = { "pe","fg","ls","lv","lc","null","begin" };
							for (const auto& i : bridge.stall_note[self_location]) { Debug::getInstance()->getPortFile() << maps[int(self_location.type)] << self_location.node_index << self_location.port_index << "stall reason is" << maps[int(i.type)] << i.node_index << i.port_index << std::endl; }
						}
					}
				}
				lc_map[order2index[i].second]->print();
				lc_map[order2index[i].second]->wireReset();
			}
			else { DEBUG_ASSERT(false); }
		}
		//		bridge.setAllBp();
		for (auto& i : pe_map)
			i.second->outReset();
		for (auto& i : lse_map)/////////////////////////////////ÕâÀïÃæÔÚconfig_orderÖÐÁ¬µ½lcµÄ×îºóÒ»¸ölsµÄoutput¿ÉÄÜ²»ÄÜÇå¿Õ//
			i.second->outReset();
		for (auto& i : lc_map)
			i.second->outReset();
		if (task_finish) {
			Debug::getInstance()->getPortFile() << "clk" << clk << std::endl;
			break;
		}

		ClkDomain::getInstance()->selfAdd();
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


