#pragma once

#include "pr_res_graph_element.h"
#include "pr_res_graph_monitor.h"

namespace Simulator::Mpr
{
	class ResourceGraph final
	{
		friend RGraphMonitor;
	public:
		/** big 6 - customize */
		ResourceGraph();
		~ResourceGraph();
		/** big 6 - default */
		/** big 6 - delete */
		ResourceGraph(const ResourceGraph& b_) = delete;
		ResourceGraph(ResourceGraph&& b_) = delete;
		ResourceGraph& operator=(const ResourceGraph& b_) = delete;
		ResourceGraph& operator=(ResourceGraph&& b_) = delete;

	public:
		/** show info in xml docs */
		auto showInfo() const -> void;

	public:
		/** get-function interface */
		auto getResources() const -> const vector<Resource*>*;
		auto getResource(uint index_) const -> const Resource*;
		auto getMonitor() const -> const RGraphMonitor*;

	private:
		RGraphMonitor* _monitor;
		vector<Resource*> _resources;

	private:
		auto generate() -> void;

		auto elementsConstruct() -> void;
		auto wireNetConstruct() -> void;
		auto segmentNetConstruct() -> void;

		/** print info in a xml doc */
		auto xmlGenerateBlock() const -> void;
		auto xmlGeneratePort() const -> void;
		auto xmlGenerateSegment() const -> void;
		auto xmlGenerateWire() const -> void;
	};
}


