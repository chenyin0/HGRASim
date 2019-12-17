#pragma once

/**
 * 这个文档是初始化读取所有xml用的
 * 需要改动务必通知全体
 * 
 * 使用说明：
 * DFG类保存读取之后的内容，单例类
 * 所有内容用字典的形式保存，类型为
 * unordered_map<NodeType, vector<const DFGNodeInterface*>>
 * 用DFGNodeType作为键值可以拿到对应类型所有节点的基类指针的向量
 * 如需使用，需要自行dynamic_cast<DFGNode<NodeType::xx>*>(your_ptr)
 * 使用DFG::getInstance()->getDictionary()获得这个字典
 * 返回类型为常引用，记得加const
 * 
 * 
 * 行 7.10
 */

#include "../define/global.hpp"
#include "../util/util.hpp"
#include "../util/cord.hpp"
#include "../../lib/tinyxml2/tinyxml2.h"
#include "enum_definition.hpp"
#include "enum_converter.hpp"
#include "dfg_node.hpp"
#include "para.hpp"

using namespace tinyxml2;

namespace Simulator::Preprocess
{

	class DFG : public Singleton<DFG>
	{
	private:
		DFG();
		~DFG();

	public:
		[[nodiscard]]
		auto getDictionary() const -> const unordered_map<NodeType, vector<const DFGNodeInterface*>>&;
		auto getDfgDict() const-> const std::vector<std::pair<NodeType, const DFGNodeInterface*>>&;//config_order不能用_dictionary来初始化，得按仿真顺序初始化，使用dfg_dictionary初始化config_order
		auto isManualPlacement() const -> bool;

	private:
		friend Singleton<DFG>;

		bool _manual_placement;
		unordered_map<NodeType, vector<const DFGNodeInterface*>> _dictionary;
		std::vector<std::pair<NodeType, const DFGNodeInterface*>> dfg_dictionary;

	private:
		auto xmlRead() -> void;

		auto peRead(XMLElement* node_xml_) const -> DFGNodeInterface*;
		auto fgRead(XMLElement* node_xml_) const -> DFGNodeInterface*;
		auto lsRead(XMLElement* node_xml_) const -> DFGNodeInterface*;
		auto lvRead(XMLElement* node_xml_) const -> DFGNodeInterface*;
		auto lcRead(XMLElement* node_xml_) const -> DFGNodeInterface*;

	};

	class Para : public Singleton<Para>
	{
	private:
		Para();

	public:
		[[nodiscard]]
		auto getArrayPara() const -> const ArrayPara&;

		[[nodiscard]]
		auto getArchPara() const -> const ArchPara&;

	private:
		friend Singleton<Para>;

		ArrayPara _array_para;
		ArchPara _arch_para;

	private:
		auto xmlRead() -> void;
		auto arrayParaXmlRead() -> void;
		auto archParaXmlRead() -> void;

		auto archBlockXmlRead(XMLElement* block_xml_) -> void;
		auto archNetXmlRead(XMLElement* net_xml_) -> void;
		auto archWireNetXmlRead(XMLElement* wire_net_config_xml_) -> void;
		auto archWireNetMeshXmlRead(XMLElement* wire_net_xml_) -> tuple<WireType, BlockType, vector<PortIndex>, vector<PortIndex>>;
		auto archWireNetFullConnectXmlRead(XMLElement* wire_net_xml_) ->WireNetInterface*;
		auto archWireNetMultiConnectXmlRead(XMLElement* wire_net_xml_) ->WireNetInterface*;
		auto archSegmentNetXmlRead(XMLElement* segment_net_config_xml_) -> void;
	};

}
