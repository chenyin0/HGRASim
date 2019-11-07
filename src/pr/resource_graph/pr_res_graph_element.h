#pragma once

#include "../../define/global.hpp"
#include "../../util/util.hpp"
#include "../global/global.hpp"
#include "../../../lib/tinyxml2/tinyxml2.h"

namespace Simulator::Mpr
{
	using namespace tinyxml2;
	
	interface Resource
	{
		virtual void generateXmlInfoComplete(XMLDocument& doc_, XMLElement* root_) const = 0;

		virtual void generateXmlInfoBrief(XMLDocument& doc_, XMLElement* root_) const = 0;

		virtual vector<uint> getAllNextId() const = 0;

		virtual ~Resource() = default;
	};

	struct RNodeBlock : Resource
	{
	public:
		// 
		BlockType block_type;
		const ArchBlock* block_ptr;
		Cord pos;
		// 
		vector<uint> next_port_id;

	public:
		RNodeBlock(
			BlockType block_type_, 
			const ArchBlock* content_, 
			Cord pos_
		);

		bool operator==(const RNodeBlock& b_) const;

	public:
		void generateXmlInfoComplete(XMLDocument& doc_, XMLElement* root_) const override;
		void generateXmlInfoBrief(XMLDocument& doc_, XMLElement* root_) const override;
		vector<uint> getAllNextId() const override;

	};

	struct RNodePort : Resource
	{
	public:
		PortType port_type;
		PortPos port_pos;

		vector<uint> next_wire_id;
		vector<uint> next_segment_id;
		vector<uint> next_block_id;
		vector<uint> next_port_id;

	public:
		RNodePort(
			PortType port_type_,
			PortPos port_pos_
		);

		bool operator==(PortPos port_pos_) const;

	public:
		void generateXmlInfoComplete(XMLDocument& doc_, XMLElement* root_) const override;
		void generateXmlInfoBrief(XMLDocument& doc_, XMLElement* root_) const override;
		vector<uint> getAllNextId() const override;
	};

	struct RNodeWire : Resource
	{
	public:
		WireType type;
		PortPos source_port_pos;
		PortPos target_port_pos;

		uint next_port_id;

	public:
		RNodeWire(
			WireType type_, 
			PortPos source_port_pos_, 
			PortPos target_port_pos_
		);

	public:
		void generateXmlInfoComplete(XMLDocument& doc_, XMLElement* root_) const override;
		void generateXmlInfoBrief(XMLDocument& doc_, XMLElement* root_) const override;
		vector<uint> getAllNextId() const override;
	};

	struct RNodeSegment : Resource
	{
	public:
		SegmentType type;
		vector<PortPos> source_port_pos;
		vector<PortPos> target_port_pos;
		uint index;

		vector<uint> next_port_id;
		vector<uint> next_segment_id;

	public:
		RNodeSegment(
			SegmentType type_,
			vector<PortPos> source_port_pos_,
			vector<PortPos> target_port_pos_,
			uint index_
		);

	public:
		void generateXmlInfoComplete(XMLDocument& doc_, XMLElement* root_) const override;
		void generateXmlInfoBrief(XMLDocument& doc_, XMLElement* root_) const override;
		vector<uint> getAllNextId() const override;
	};

}

