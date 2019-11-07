#pragma once

#include "../app_graph/app_graph.h"
#include "../sim/inout.h"

namespace Simulator
{

	class Bridge
	{
		using NodeIndex = uint;
		using PortIndex = uint;
	public:

		class BpCallBackFuns
		{
		public:
			BpCallBackFuns() = default;

			void registerFun(NodeType type_, NodeIndex node_index_, std::function<void(PortIndex, bool)> fun_);

			void operator() (NodeType type_, NodeIndex node_index_, PortIndex port_index_, bool bp_value_);

		private:
			unordered_map<NodeType,
				unordered_map<NodeIndex,
				std::function<void(PortIndex, bool)>>
				> _funs;
		};

		class InputCallBackFuns
		{
		public:
			InputCallBackFuns() = default;

			void registerFun(NodeType type_, NodeIndex node_index_, std::function<void(PortIndex, Array::Port_inout)> fun_);

			void operator() (NodeType type_, NodeIndex node_index_, PortIndex port_index_, Array::Port_inout value_);

		private:
			unordered_map<NodeType,
				unordered_map<NodeIndex,
				std::function<void(PortIndex, Array::Port_inout)>>
				> _funs;
		};

		struct Location
		{
			NodeType type;
			uint node_index;
			uint port_index;

			Location() = default;

			Location(NodeType type_, uint node_index_, uint port_index_);

			bool operator== (const Location& b_) const;
			//std::ostream &operator<<(ostream &os,const Location)
		};

		struct LocationHash
		{
			std::size_t operator()(const Location& b_) const;
		};

	public:
		explicit Bridge(const AppGraph& app_graph_);

	public:
		void setNextInput(Array::Port_inout output_, NodeType type_, uint type_index_, uint port_index_);
		void setBp(bool bp_, NodeType type_, uint type_index_, uint port_index_);
		void setAllBp();
		void InitBp();

		void bpFunRegister(NodeType type_, NodeIndex node_index_, std::function<void(PortIndex, bool)> fun_);
		void inputFunRegister(NodeType type_, NodeIndex node_index_, std::function<void(PortIndex, Array::Port_inout)> fun_);
		void recvOneBp(Location self_location,bool value);
		unordered_map<Location, bool, LocationHash> _bp_temporary_buffer;
		unordered_map<Location, vector<Location>, LocationHash> stall_note;
		vector<Location> findNextNode(NodeType type_, uint type_index_, uint port_index_);
		unordered_map<Location, bool, LocationHash> getBpbuffer() { return _bp_temporary_buffer; };
		Simulator::Bridge::Location findPreNode(NodeType type_, uint type_index_, uint port_index_);
	private:
		auto initTable() -> void;

	private:

		const AppGraph& _app_graph;

		BpCallBackFuns _bp_funs;
		InputCallBackFuns _input_funs;

		unordered_map<Location, vector<Location>, LocationHash> _next_table;
		unordered_map<Location, Location, LocationHash> _pre_table;
//		unordered_map<Location, bool, LocationHash> _bp_temporary_buffer;

	};
}