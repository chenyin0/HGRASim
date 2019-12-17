#include "placement_board.h"

namespace Simulator::Mpr
{
	PlacementBoard::Occupy::Occupy()
		: _used(Umap<BlockType, Uset<Cord>>{})
		, _un_used(Umap<BlockType, Uset<Cord>>{})
	{
		// fetch parameters from preprocess part
		const Preprocess::ArchPara& arch_para = Preprocess::Para::getInstance()->getArchPara();
		const Umap<BlockType, Preprocess::ArchBlock> arch_block_info = arch_para.arch_block_type;

		// build
		for_each(begin(arch_block_info), end(arch_block_info),
		         [&](const pair<const BlockType, Preprocess::ArchBlock>& type_para_)
		         {
			         BlockType type = type_para_.first;
			         const vector<vector<Bool>>& block_info = type_para_.second.position;

			         // set used
			         _used[type] = Uset<Cord>{};

			         // set unused
			         for (uint i = 0; i < block_info.size(); i++)
				         for (uint j = 0; j < block_info[0].size(); j++)
							 if (block_info[i][j])
								_un_used[type].insert(Cord{i, j});
		         });
	}

	PlacementBoard::Occupy::Occupy(Occupy&& rhs_) noexcept
		: _used(std::move(rhs_._used))
		, _un_used(std::move(rhs_._un_used))
	{
	}

	PlacementBoard::Occupy& PlacementBoard::Occupy::operator=(Occupy&& rhs_) noexcept
	{
		_used = move(rhs_._used);
		_un_used = move(rhs_._un_used);
		return *this;
	}

	void PlacementBoard::Occupy::swap(Occupy& a_, Occupy& b_) const
	{
		using std::swap;
		swap(a_._used, b_._used);
		swap(a_._un_used, b_._un_used);
	}

	void PlacementBoard::Occupy::addUsed(BlockType type_, Cord cord_)
	{
		// delete from unused set
		auto un_used_itr = _un_used[type_].find(cord_);
		if (un_used_itr == _un_used[type_].end())
			DEBUG_ASSERT(false);
		_un_used[type_].erase(un_used_itr);

		// add cord to used set
		_used[type_].insert(cord_);
	}

	void PlacementBoard::Occupy::deleteUsed(BlockType type_, Cord cord_)
	{
		// delete from used set
		auto used_itr = _used[type_].find(cord_);
		if (used_itr == _used[type_].end())
			DEBUG_ASSERT(false);
		_used[type_].erase(used_itr);

		// add cord to unused set
		_un_used[type_].insert(cord_);
	}

	pair<bool, Cord> PlacementBoard::Occupy::getSwapTarget(BlockType type_)
	{
		uint total_num = _used[type_].size() + _un_used[type_].size();
		uint target_num = Util::uRandom(0, total_num - 1);

		// if used
		if (target_num < _used[type_].size())
		{
			auto itr = _used[type_].begin();
			while (target_num--)
				++itr;
			return std::make_pair(true, *itr);
		}
		// if unused
		target_num -= _used[type_].size();
		auto itr = _un_used[type_].begin();
		while (target_num--)
			++itr;
		return std::make_pair(false, *itr);
	}

	Cord PlacementBoard::Occupy::getOneUnusedCord(BlockType type_)
	{
		uint size = _un_used[type_].size();
		uint num = Util::uRandom(0, size - 1);
		auto itr = _un_used[type_].begin();
		while (num--)
			++itr;
		Cord result_cord = *itr;
		addUsed(type_, result_cord);
		return result_cord;
	}

	PlacementBoard::PlacementBoard(const AppGraph& app_graph_)
		: _app_graph(app_graph_)
	{
		const Preprocess::ArchPara& arch_para = Preprocess::Para::getInstance()->getArchPara();
		_board = vector<vector<uint>>(arch_para.row_num, vector<uint>(arch_para.col_num, UINT_MAX));
	}

	PlacementBoard::PlacementBoard(PlacementBoard&& rhs_) noexcept
		: _app_graph(rhs_._app_graph)
		, _board(std::move(rhs_._board))
		, _table(std::move(rhs_._table))
		, _occupy(std::move(rhs_._occupy))
	{
	}

	PlacementBoard& PlacementBoard::operator=(PlacementBoard&& rhs_) noexcept
	{
		_board = move(rhs_._board);
		_table = move(rhs_._table);
		_occupy = std::move(rhs_._occupy);
		return *this;
	}

	void PlacementBoard::swap(PlacementBoard& a_, PlacementBoard& b_) noexcept
	{
		using std::swap;
		swap(_board, b_._board);
		swap(_table, b_._table);
		swap(_occupy, b_._occupy);
	}

	auto PlacementBoard::swapOnce(uint node_index_) -> void
	{
		BlockType node_type = _app_graph.getNodes()[node_index_].type;

		uint cur_node_index = node_index_;
		Cord cur_cord = _table[node_index_];
		
		auto[is_used, target_cord] = _occupy.getSwapTarget(node_type);
		
		if (is_used)
		{
			/** if target place has a node, then no need to change _occupy */
			uint target_node_index = _board[target_cord.x][target_cord.y];
			if (target_node_index == node_index_)
				return;
			// delete current info
			deleteRecord(cur_node_index, cur_cord);
			deleteRecord(target_node_index, target_cord);
			// put target index into cur cord, put current index into target cord
			addRecord(cur_node_index, target_cord);
			addRecord(target_node_index, cur_cord);
		}
		else
		{
			/** if target place is empty */
			// delete current info
			_occupy.deleteUsed(node_type, cur_cord);
			deleteRecord(cur_node_index, cur_cord);
			// add cord to _occupy, put current node to target cord
			_occupy.addUsed(node_type, target_cord);
			addRecord(cur_node_index, target_cord);
		}
	}

	void PlacementBoard::addRecord(uint who_, Cord where_)
	{
		_board[where_.x][where_.y] = who_;
		_table[who_] = where_;
	}

	void PlacementBoard::deleteRecord(uint who_, Cord where_)
	{
		_board[where_.x][where_.y] = UINT_MAX;
		auto itr = _table.find(who_);
		if (itr == _table.end())
			DEBUG_ASSERT(false);
		_table.erase(itr);
	}

	void PlacementBoard::clear(const vector<uint>& nodes_index_vec_)
	{
		for_each(begin(nodes_index_vec_), end(nodes_index_vec_), 
			[&](int index_)
		{
			BlockType node_type = _app_graph.getNodes()[index_].type;
			
			auto itr = _table.find(index_);
			if (itr == _table.end())
				return;

			Cord cord = itr->second;
			_occupy.deleteUsed(node_type, cord);
			deleteRecord(index_, cord);
		});
	}

	void PlacementBoard::randomPutNode(BlockType type_, uint node_index_)
	{
		Cord target_cord = _occupy.getOneUnusedCord(type_);
		_table[node_index_] = target_cord;
		_board[target_cord.x][target_cord.y] = node_index_;
	}

	bool PlacementBoard::queryExist(uint node_index_) const
	{
		auto itr = _table.find(node_index_);
		return !(itr == _table.end());
	}

	Cord PlacementBoard::queryCord(uint node_index_) const
	{
		return _table.find(node_index_)->second;
	}

	vector<Cord> PlacementBoard::generateVectorCord() const
	{
		vector<Cord> result(_app_graph.getNodes().size());
		for (uint index = 0; index < _app_graph.getNodes().size(); index++)
		{
			auto itr = _table.find(index);
			if (itr == _table.end())
				result[index] = Cord{ UINT_MAX, UINT_MAX };
			else
				result[index] = itr->second;
		}
		return result;
	}
}
