#include "pr_res_graph_element.h"

namespace Simulator::Mpr
{
	RNodeBlock::RNodeBlock(
		BlockType block_type_, 
		const ArchBlock* content_, 
		const Cord pos_
	)
		: block_type(block_type_)
		, block_ptr(content_)
		, pos(pos_)
	{
	}

	bool RNodeBlock::operator==(const RNodeBlock& b_) const
	{
		return pos == b_.pos && block_type == b_.block_type;
	}

	void RNodeBlock::generateXmlInfoComplete(XMLDocument& doc_, XMLElement* root_) const
	{
		root_->SetAttribute("Block", BlockTypeConverter::toString(block_type).c_str());
		root_->SetAttribute("pos", static_cast<string>(pos).c_str());
	}

	void RNodeBlock::generateXmlInfoBrief(XMLDocument& doc_, XMLElement* root_) const
	{
		generateXmlInfoComplete(doc_, root_);
	}

	vector<uint> RNodeBlock::getAllNextId() const
	{
		// only have next port id
		return next_port_id;
	}

	RNodePort::RNodePort(
		PortType port_type_,
		PortPos port_pos_
	)
		: port_type(port_type_)
		, port_pos(std::move(port_pos_))
	{
	}

	bool RNodePort::operator==(PortPos port_pos_) const
	{
		return port_pos == port_pos_;
	}

	void RNodePort::generateXmlInfoComplete(XMLDocument& doc_, XMLElement* root_) const
	{
		root_->SetAttribute("type", PortTypeConverter::toString(port_type).c_str());
		root_->SetAttribute("port_pos", static_cast<string>(port_pos).c_str());
	}

	void RNodePort::generateXmlInfoBrief(XMLDocument& doc_, XMLElement* root_) const
	{
		generateXmlInfoComplete(doc_, root_);
	}

	vector<uint> RNodePort::getAllNextId() const
	{
		vector<uint> result;
		result.insert(result.end(), next_wire_id.begin(), next_wire_id.end());
		result.insert(result.end(), next_segment_id.begin(), next_segment_id.end());
		result.insert(result.end(), next_block_id.begin(), next_block_id.end());
		result.insert(result.end(), next_port_id.begin(), next_port_id.end());
		return result;
	}

	RNodeWire::RNodeWire(
		WireType type_,
		PortPos source_port_pos_,
		PortPos target_port_pos_
	)
		: type(type_)
		, source_port_pos(source_port_pos_)
		, target_port_pos(target_port_pos_)
	{
	}

	void RNodeWire::generateXmlInfoComplete(XMLDocument& doc_, XMLElement* root_) const
	{
		root_->SetAttribute("type", WireTypeConverter::toString(type).c_str());
		// from
		XMLElement* from_node = doc_.NewElement("from");
		from_node->SetAttribute("port_pos", static_cast<string>(source_port_pos).c_str());
		root_->InsertEndChild(from_node);
		// to
		XMLElement* send_node = doc_.NewElement("to");
		send_node->SetAttribute("port_pos", static_cast<string>(target_port_pos).c_str());
		root_->InsertEndChild(send_node);
	}

	void RNodeWire::generateXmlInfoBrief(XMLDocument& doc_, XMLElement* root_) const
	{
		root_->SetAttribute("type", WireTypeConverter::toString(type).c_str());
	}

	vector<uint> RNodeWire::getAllNextId() const
	{
		vector<uint> result;
		result.push_back(next_port_id);
		return result;
	}

	RNodeSegment::RNodeSegment(
		SegmentType type_,
		vector<PortPos> source_port_pos_,
		vector<PortPos> target_port_pos_,
		uint index_
	)
		: type(type_)
		, source_port_pos(std::move(source_port_pos_))
		, target_port_pos(std::move(target_port_pos_))
		, index(index_)
	{
	}

	void RNodeSegment::generateXmlInfoComplete(XMLDocument& doc_, XMLElement* root_) const
	{
		root_->SetAttribute("type", SegmentTypeConverter::toString(type).c_str());
		// from
		for (uint j = 0; j < source_port_pos.size(); j++)
		{
			XMLElement* from_node = doc_.NewElement("from");
			from_node->SetAttribute("port_pos", static_cast<string>(source_port_pos[j]).c_str());
			root_->InsertEndChild(from_node);
		}
		// to
		for (uint j = 0; j < target_port_pos.size(); j++)
		{
			XMLElement* send_node = doc_.NewElement("to");
			send_node->SetAttribute("port_pos", static_cast<string>(target_port_pos[j]).c_str());
			root_->InsertEndChild(send_node);
		}
	}

	void RNodeSegment::generateXmlInfoBrief(XMLDocument& doc_, XMLElement* root_) const
	{
		root_->SetAttribute("type", SegmentTypeConverter::toString(type).c_str());
	}

	vector<uint> RNodeSegment::getAllNextId() const
	{
		vector<uint> result;
		result.insert(result.end(), next_port_id.begin(), next_port_id.end());
		result.insert(result.end(), next_segment_id.begin(), next_segment_id.end());
		return result;
	}
}
