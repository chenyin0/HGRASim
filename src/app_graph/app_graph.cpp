#include "app_graph.h"

namespace Simulator
{
	AppNet::AppNet(uint source_node_vec_index_, uint source_port_index_, const vector<pair<uint, uint>>& targets_)
		: source_node_vec_index(source_node_vec_index_)
		, source_port_index(source_port_index_)
		, targets(targets_)
	{
	}

	AppNet::AppNet(uint source_node_vec_index_, uint source_port_index_, vector<pair<uint, uint>>&& targets_)
		: source_node_vec_index(source_node_vec_index_)
		, source_port_index(source_port_index_)
	{
		std::swap(targets, targets_);
	}

	AppNode::AppNode(BlockType type_, const uint num_)
		: type(type_)
		, type_num(num_)
	{
	}

	bool AppNode::operator==(const AppNode& b_) const
	{
		return (type == b_.type && type_num == b_.type_num);
	}

	bool AppNode::operator<(const AppNode& b_) const
	{
		if (type < b_.type)
			return true;
		if (type > b_.type)
			return false;
		if (type_num < b_.type_num)
			return true;

		return false;
	}

	AppNode::operator string() const
	{
		string tmp;
		tmp += BlockTypeConverter::toString(type);
		tmp += "  ";
		tmp += std::to_string(type_num);
		return tmp;
	}

	AppSimNode::AppSimNode(BlockType type_, const uint num_,uint order_)
		: type(type_)
		, type_num(num_)
		, order(order_)
	{
	}

	bool AppSimNode::operator==(const AppSimNode& b_) const
	{
		return (type == b_.type && type_num == b_.type_num && order== b_.order);
	}

	bool AppSimNode::operator<(const AppSimNode& b_) const
	{
		if (type < b_.type)
			return true;
		if (type > b_.type)
			return false;
		if (type_num < b_.type_num)
			return true;

		return false;
	}

	AppSimNode::operator string() const
	{
		string tmp;
		tmp += BlockTypeConverter::toString(type);
		tmp += "  ";
		tmp += std::to_string(type_num);
		return tmp;
	}

	AppGraph::AppGraph()
	{
		init();
	}

	auto AppGraph::generateAllLink() const -> vector<AppLink>
	{
		vector<AppLink> result;
		for (auto& every_node : _nodes)
			for (auto& every_next_link : every_node.next_node)
				result.push_back(every_next_link);
		return result;
	}

	auto AppGraph::generateAllNet() const -> vector<AppNet>
	{
		vector<AppLink> all_link = generateAllLink();
		vector<AppNet> result;

		for (auto& link : all_link)
		{
			auto itr = find_if(begin(result), end(result), [&](const AppNet& net_)
			{
				return link.source_node_vec_index == net_.source_node_vec_index 
					&& link.source_port_index == net_.source_port_index;
			});

			if (itr == result.end())
			{
				vector<pair<uint, uint>> targets;
				targets.emplace_back(link.target_node_vec_index, link.target_port_index);
				result.emplace_back(link.source_node_vec_index, link.source_port_index, std::move(targets));
			}
			else
			{
				itr->targets.emplace_back(link.target_node_vec_index, link.target_port_index);
			}
		}

		return result;
	}

	auto AppGraph::getNodes() const -> const vector<AppNode>&
	{
		return _nodes;
	}


	auto AppGraph::getSimNodes() const -> const vector<AppNode> &
	{
		return sim_nodes;
	}

	auto AppGraph::getSize() const -> uint
	{
		return _nodes.size();
	}

	auto AppGraph::getManualPlacement() const -> const vector<Cord>&
	{
		return _manual_placement;
	}

	auto AppGraph::init() -> void
	{
		const auto& dfg = Preprocess::DFG::getInstance()->getDfgDict();
		const vector<string> s1{ "pe","fg","ls","lv","lc","null","begin" };

		// generate node
		//for (auto& [node_type, ptr_vec] : dfg)
		//{
		//	for (uint i = 0; i < ptr_vec.size(); i++)
		//	{
		//		_nodes.emplace_back(node_type, i);
		//		_manual_placement.push_back(ptr_vec[i]->getManualPlacementCord());
		//	}
		//}

		for (auto& [node_type, ptr_vec] : dfg)
		{
//			for (uint i = 0; i < ptr_vec.size(); i++)
//			{
				uint input_node_index;
				if (node_type == NodeType::pe)
					input_node_index = dynamic_cast<const Preprocess::DFGNode<Simulator::NodeType::pe>*>(ptr_vec)->index;/////////这里有问题//////////
				else if (node_type == NodeType::ls)
					input_node_index = dynamic_cast<const Preprocess::DFGNode<Simulator::NodeType::ls>*>(ptr_vec)->index;
				else if (node_type == NodeType::lc)
					input_node_index = dynamic_cast<const Preprocess::DFGNode<Simulator::NodeType::lc>*>(ptr_vec)->index;
				else
					;
				sim_nodes.emplace_back(node_type, input_node_index);
				_manual_placement.push_back(ptr_vec->getManualPlacementCord());/////会有影响吗
//			}
		}

		// generate link
		for (auto& [input_node_type, ptr_vec] : dfg)
		{
			//for (uint i = 0; i < ptr_vec.size(); i++)
			//{
			//	uint input_node_index = Util::findIndex(_nodes, AppNode{ input_node_type, i });/////////这里有问题//////////

			//	auto all_input = ptr_vec[i]->getAllInput();

			//	for (uint input_port_index = 0; input_port_index < all_input.size(); input_port_index++)
			//	{
			//		auto[output_node_type, out_node_num, output_port_index] = all_input[input_port_index];

			//		if (!NodeTypeConverter::isRealNode(output_node_type))
			//			continue;

			//		uint output_node_index = Util::findIndex(_nodes, AppNode{ output_node_type, out_node_num });////////////也是基于顺序仿真的前提//////

			//		AppLink link{ output_node_index, output_port_index, input_node_index, input_port_index };
			//		_nodes[output_node_index].next_node.push_back(link);
			//		_nodes[input_node_index].pre_node.push_back(link);
			//	}
			//}
	//		for (uint i = 0; i < ptr_vec.size(); i++)
	//		{
				uint input_node_num;
				if (input_node_type == NodeType::pe)
					input_node_num = dynamic_cast<const Preprocess::DFGNode<Simulator::NodeType::pe>*>(ptr_vec)->index;/////////这里有问题//////////
				else if (input_node_type == NodeType::ls)
					input_node_num = dynamic_cast<const Preprocess::DFGNode<Simulator::NodeType::ls>*>(ptr_vec)->index;
				else if (input_node_type == NodeType::lc)
					input_node_num = dynamic_cast<const Preprocess::DFGNode<Simulator::NodeType::lc>*>(ptr_vec)->index;
				else
					;
				auto all_input = ptr_vec->getAllInput();
				uint input_node_index = Util::findIndex(sim_nodes, AppNode{ input_node_type, input_node_num });
				for (uint input_port_index = 0; input_port_index < all_input.size(); input_port_index++)
				{
					auto[output_node_type, out_node_num, output_port_index] = all_input[input_port_index];/////////input_port_index//////////这里得到的out_node_num是实际的out_node_index，不能在_nodes里面寻找

					if (!NodeTypeConverter::isRealNode(output_node_type))
						continue;

					uint output_node_index = Util::findIndex(sim_nodes, AppNode{ output_node_type, out_node_num });////////////也是基于顺序仿真的前提//////
					if (output_node_index == UINT_MAX) {
						std::cout << "your " << s1[int(output_node_type)] << out_node_num << " was used but never instiantiated" << "\nTry again? enter y or n" << std::endl;
						char c;
						std::cin >> c;
						if (!std::cin || c == 'n')
							break;
					}
					AppLink link{ output_node_index, output_port_index, input_node_index, input_port_index };
					sim_nodes[output_node_index].next_node.push_back(link);//////////应该将对应attribution->index的nodes push back进一个link,建立bridge的时候只是用来作为索引，因此link应该用列表索引
					sim_nodes[input_node_index].pre_node.push_back(link);
				}
		//	}
		}

	}
}