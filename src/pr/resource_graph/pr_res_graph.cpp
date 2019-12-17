#include "pr_res_graph.h"

namespace Simulator::Mpr
{
	ResourceGraph::ResourceGraph()
		: _resources(vector<Resource*>{})
	{
		std::function< const vector<Resource*>* (void)> call_back_get_resources = std::bind(&ResourceGraph::getResources, this);
		_monitor = new RGraphMonitor(call_back_get_resources);
		generate();
	}

	ResourceGraph::~ResourceGraph()
	{
		delete _monitor;
		for (auto ptr : _resources)
			delete ptr;
	}

	auto ResourceGraph::generate() -> void
	{
		elementsConstruct();
		wireNetConstruct();
		segmentNetConstruct();
	}

	auto ResourceGraph::showInfo() const -> void
	{
		xmlGenerateBlock();
		xmlGeneratePort();
		xmlGenerateSegment();
		xmlGenerateWire();
	}

	void ResourceGraph::elementsConstruct()
	{
		const Preprocess::ArchPara& arch_para = Preprocess::Para::getInstance()->getArchPara();

		/** block construct */

		_monitor->setBlockIntervalLeft(_resources.size());
		
		const unordered_map<BlockType, ArchBlock>& all_block_dict = arch_para.arch_block_type;
		for (auto& [block_type, arch_block] : all_block_dict)
		{
			const ArchBlock* block_ptr = &arch_block;
			const vector<vector<Bool>> positions = arch_block.position;
			for (uint i = 0; i < positions.size(); i++)
			{
				for (uint j = 0; j < positions[i].size(); j++)
				{
					if (positions[i][j])
					{
						RNodeBlock* ptr = new RNodeBlock(block_type, block_ptr, Cord{ i, j });
						_resources.push_back(dynamic_cast<Resource*>(ptr));

						_monitor->blockTypeIndexesUpdate(block_type, _resources.size() - 1);
					}
				}
			}
		}

		_monitor->setBlockIntervalRight(_resources.size());

		/** ports construct */

		_monitor->setPortIntervalLeft(_resources.size());

		Interval block_interval = _monitor->getBlockInterval();
		for (uint block_id = block_interval.left; block_id < block_interval.right; block_id++)
		{
			RNodeBlock* block_node_ptr = dynamic_cast<RNodeBlock*>(_resources[block_id]);

			const Cord pos = block_node_ptr->pos;
			BlockType block_type = block_node_ptr->block_type;

			//  normal in ports
			const vector<PortType>& in_ports_vec = block_node_ptr->block_ptr->normal_dict.find(PortDirection::in)->second;
			for (uint port_id = 0; port_id < in_ports_vec.size(); port_id++)
			{
				PortType port_type = in_ports_vec[port_id];
				PortPos port_pos{ block_type, pos, PortDirection::in, PortFunction::normal, port_id };
				RNodePort* tmp_ptr = new RNodePort{ port_type, port_pos };

				tmp_ptr->next_block_id.push_back(block_id);
				_resources.push_back(dynamic_cast<Resource*>(tmp_ptr));

				_monitor->blockInPortsQueryUpdate(block_id, _resources.size() - 1);
			}

			// normal out ports
			const vector<PortType>& out_ports_vec = block_node_ptr->block_ptr->normal_dict.find(PortDirection::out)->second;
			for (uint port_id = 0; port_id < out_ports_vec.size(); port_id++)
			{
				PortType port_type = out_ports_vec[port_id];
				PortPos port_pos{ block_type, pos, PortDirection::out, PortFunction::normal, port_id };
				RNodePort* tmp_ptr = new RNodePort{ port_type, port_pos };

				_resources.push_back(dynamic_cast<Resource*>(tmp_ptr));
				block_node_ptr->next_port_id.push_back(_resources.size() - 1);

				_monitor->blockOutPortsQueryUpdate(block_id, _resources.size() - 1);
			}

			//  router ports
			const vector<PortType>& router_dict = block_node_ptr->block_ptr->router_dict;
			for (uint port_id = 0; port_id < router_dict.size(); port_id++)
			{
				PortType port_type = out_ports_vec[port_id];

				PortPos in_port_pos{ block_type, pos, PortDirection::in, PortFunction::router, port_id };
				RNodePort* in_tmp_ptr = new RNodePort{ port_type, in_port_pos };
				_resources.push_back(dynamic_cast<Resource*>(in_tmp_ptr));
				_monitor->blockInPortsQueryUpdate(block_id, _resources.size() - 1);

				PortPos out_port_pos{ block_type, pos, PortDirection::out, PortFunction::router, port_id };
				RNodePort* out_tmp_ptr = new RNodePort{ port_type, out_port_pos };
				_resources.push_back(dynamic_cast<Resource*>(out_tmp_ptr));
				_monitor->blockOutPortsQueryUpdate(block_id, _resources.size() - 1);

				// router pipe: in port connect to out port directly 
				in_tmp_ptr->next_port_id.push_back(_resources.size() - 1);
			}

		}

		_monitor->setPortIntervalRight(_resources.size());
	}

	auto ResourceGraph::wireNetConstruct() -> void
	{
		const Preprocess::ArchPara& arch_para = Preprocess::Para::getInstance()->getArchPara();
		const unordered_map<WireNetConfigMode, vector<Preprocess::WireNetInterface*>>& wire_nets = arch_para.wire_nets;

		_monitor->setWireIntervalLeft(_resources.size());
		for (auto& [wire_net_mode, ptr_vec] : wire_nets)
		{
			for (auto& ptr : ptr_vec)
			{
				auto [wire_type, vec_wires] = ptr->generate();
				for (auto& [source_port_pos, target_port_pos] : vec_wires)
				{
					RNodeWire* wire_ptr = new RNodeWire{ wire_type, source_port_pos, target_port_pos };
					_resources.push_back(dynamic_cast<Resource*>(wire_ptr));

					RNodePort* source_port_ptr = dynamic_cast<RNodePort*>(_resources[_monitor->queryPortIndex(source_port_pos)]);
					source_port_ptr->next_wire_id.push_back(_resources.size() - 1);
					wire_ptr->next_port_id = _monitor->queryPortIndex(target_port_pos);
				}
			}
		}

		_monitor->setWireIntervalRight(_resources.size());
	}

	auto ResourceGraph::segmentNetConstruct() -> void
	{
		const Preprocess::ArchPara& arch_para = Preprocess::Para::getInstance()->getArchPara();
		const vector<Preprocess::SegmentNet>& segment_net = arch_para.segment_net;

		_monitor->setSegmentIntervalLeft(_resources.size());
		for (auto& every_segment_net : segment_net)
		{
			for (uint i = 0; i < every_segment_net.channel_num; i++)
			{
				RNodeSegment* ptr = new RNodeSegment{ every_segment_net.segment_type, every_segment_net.source_pos, every_segment_net.target_pos, i };
				_resources.push_back(dynamic_cast<Resource*>(ptr));

				// source port to segment
				for (auto& source_port_pos : ptr->source_port_pos)
				{
					RNodePort* source_port_ptr = dynamic_cast<RNodePort*>(_resources[_monitor->queryPortIndex(source_port_pos)]);
					source_port_ptr->next_segment_id.push_back(_resources.size() - 1);
				}

				// segment to target port
				for (auto& target_port_pos : ptr->target_port_pos)
				{
					ptr->next_port_id.push_back(_monitor->queryPortIndex(target_port_pos));
				}

				_monitor->segmentUpdate(_resources.size() - 1, *ptr);
			}
		}

		_monitor->setSegmentIntervalRight(_resources.size());
	}

	auto ResourceGraph::xmlGenerateBlock() const -> void
	{
		const char* declaration = R"(<?xml version="1.0" encoding="UTF-8" standalone="no"?>)";
		XMLDocument doc;
		doc.Parse(declaration);

		XMLElement* root = doc.NewElement("Blocks");
		doc.InsertEndChild(root);

		Interval block_interval = _monitor->getBlockInterval();

		for (uint i = block_interval.left; i < block_interval.right; i++)
		{
			XMLElement* block_node = doc.NewElement("Block");

			block_node->SetAttribute("num", std::to_string(i).c_str());
			_resources[i]->generateXmlInfoComplete(doc, block_node);
			root->InsertEndChild(block_node);

			// next id
			vector<uint> next_ids = _resources[i]->getAllNextId();
			for (auto j : next_ids)
			{
				XMLElement* next_node = doc.NewElement("next");
				next_node->SetAttribute("id", std::to_string(j).c_str());
				_resources[j]->generateXmlInfoBrief(doc, next_node);
				block_node->InsertEndChild(next_node);
			}
		}

		string addr = GlobalPara::getInstance()->getOutputAddr(OutputAddr::resource_block_graph_info_xml_addr);
		doc.SaveFile(addr.c_str());
	}

	auto ResourceGraph::xmlGeneratePort() const -> void
	{
		const char* declaration = R"(<?xml version="1.0" encoding="UTF-8" standalone="no"?>)";
		XMLDocument doc;
		doc.Parse(declaration);

		XMLElement* root = doc.NewElement("Ports");
		doc.InsertEndChild(root);

		Interval port_interval = _monitor->getPortInterval();

		for (uint i = port_interval.left; i < port_interval.right; i++)
		{
			XMLElement* port_node = doc.NewElement("port");

			port_node->SetAttribute("num", std::to_string(i).c_str());
			_resources[i]->generateXmlInfoComplete(doc, port_node);
			root->InsertEndChild(port_node);

			// next id
			vector<uint> next_ids = _resources[i]->getAllNextId();
			for (auto j : next_ids)
			{
				XMLElement* next_node = doc.NewElement("next");
				next_node->SetAttribute("id", std::to_string(j).c_str());
				_resources[j]->generateXmlInfoBrief(doc, next_node);
				port_node->InsertEndChild(next_node);
			}
		}

		string addr = GlobalPara::getInstance()->getOutputAddr(OutputAddr::resource_port_graph_info_xml_addr);
		doc.SaveFile(addr.c_str());
	}

	auto ResourceGraph::xmlGenerateWire() const -> void
	{
		const char* declaration = R"(<?xml version="1.0" encoding="UTF-8" standalone="no"?>)";
		XMLDocument doc;
		doc.Parse(declaration);

		XMLElement* root = doc.NewElement("Wires");
		doc.InsertEndChild(root);

		Interval wire_interval = _monitor->getWireInterval();

		for (uint i = wire_interval.left; i < wire_interval.right; i++)
		{
			XMLElement* wire_node = doc.NewElement("wire");

			wire_node->SetAttribute("num", std::to_string(i).c_str());
			_resources[i]->generateXmlInfoComplete(doc, wire_node);
			root->InsertEndChild(wire_node);

			// no need to show next id
		}

		string addr = GlobalPara::getInstance()->getOutputAddr(OutputAddr::resource_wire_graph_info_xml_addr);
		doc.SaveFile(addr.c_str());
	}

	auto ResourceGraph::xmlGenerateSegment() const -> void
	{
		const char* declaration = R"(<?xml version="1.0" encoding="UTF-8" standalone="no"?>)";
		XMLDocument doc;
		doc.Parse(declaration);

		XMLElement* root = doc.NewElement("Segments");
		doc.InsertEndChild(root);

		Interval segment_interval = _monitor->getSegmentInterval();

		for (uint i = segment_interval.left; i < segment_interval.right; i++)
		{
			XMLElement* segment_node = doc.NewElement("segment");

			segment_node->SetAttribute("num", std::to_string(i).c_str());
			_resources[i]->generateXmlInfoComplete(doc, segment_node);
			root->InsertEndChild(segment_node);

			// next id
			vector<uint> next_ids = _resources[i]->getAllNextId();
			for (auto j : next_ids)
			{
				XMLElement* next_node = doc.NewElement("next");
				next_node->SetAttribute("id", std::to_string(j).c_str());
				_resources[j]->generateXmlInfoBrief(doc, next_node);
				segment_node->InsertEndChild(next_node);
			}
		}

		string addr = GlobalPara::getInstance()->getOutputAddr(OutputAddr::resource_segment_graph_info_xml_addr);
		doc.SaveFile(addr.c_str());
	}

	auto ResourceGraph::getResources() const -> const vector<Resource*>*
	{
		return &_resources;
	}

	auto ResourceGraph::getResource(uint index_) const -> const Resource*
	{
		return _resources[index_];
	}

	auto ResourceGraph::getMonitor() const -> const RGraphMonitor*
	{
		return _monitor;
	}

	
}