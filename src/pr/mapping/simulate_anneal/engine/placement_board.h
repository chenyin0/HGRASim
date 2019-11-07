#pragma once
#include "../../../../define/define.hpp"
#include "../../../../app_graph/app_graph.h"

namespace Simulator::Mpr
{	
	class PlacementBoard
	{
		template<typename T> using Uset = unordered_set<T>;
		template<typename T, typename K> using Umap = unordered_map<T, K>;
	private:

		class Occupy
		{
		public:
			Occupy();

			Occupy(const Occupy& rhs_) = default;

			Occupy& operator= (const Occupy& rhs_) = default;

			Occupy(Occupy&& rhs_) noexcept;

			Occupy& operator=(Occupy&& rhs_) noexcept;

			~Occupy() = default;

			void swap(Occupy& a_, Occupy& b_) const;

		public:
			void addUsed(BlockType type_, Cord cord_);

			void deleteUsed(BlockType type_, Cord cord_);

			// bool = is target used, true = used
			pair<bool, Cord> getSwapTarget(BlockType type_);

			Cord getOneUnusedCord(BlockType type_);

		private:
			Umap<BlockType, Uset<Cord>> _used;
			Umap<BlockType, Uset<Cord>> _un_used;
		};
		
	public:
		explicit PlacementBoard(const AppGraph& app_graph_);

		PlacementBoard(const PlacementBoard& rhs_) = default;

		PlacementBoard& operator=(const PlacementBoard& rhs_) = default;

		PlacementBoard(PlacementBoard&& rhs_) noexcept;

		PlacementBoard& operator=(PlacementBoard&& rhs_) noexcept;

		~PlacementBoard() = default;

		void swap(PlacementBoard& a_, PlacementBoard& b_) noexcept;

	public:
		auto swapOnce(uint node_index_) -> void;

		void clear(const vector<uint>& nodes_index_vec_);

		void randomPutNode(BlockType type_, uint node_index_);

		bool queryExist(uint node_index_) const;

		Cord queryCord(uint node_index_) const;

		vector<Cord> generateVectorCord() const;

	private:
		void addRecord(uint who_, Cord where_);

		void deleteRecord(uint who_, Cord where_);

	private:
		const AppGraph& _app_graph;
		
		vector<vector<uint>> _board;
		map<uint, Cord> _table;

		Occupy _occupy;
		
	};
	
}