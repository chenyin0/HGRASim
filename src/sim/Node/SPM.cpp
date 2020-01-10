#include "SPM.h"
#include <iomanip>
using namespace Simulator::Array;
SpmBuffer::SpmBuffer(uint _bankNum, uint _bankDepth) : bankNum(_bankNum), bankDepth(_bankDepth)
{
	spmInit();
}
void SpmBuffer::spmInit()
{
	rdPtrLse.resize(bankNum);
	wrPtrLse.resize(bankNum);
	rdPtrMem.resize(bankNum);
	//basePtr.resize(bankNum);
	bankFull.resize(bankNum);
	bankEmpty.resize(bankNum);

	lseReadBankFinish.resize(bankNum);
	lseWriteBankFinish.resize(bankNum);
	memReadBankFinish.resize(bankNum);
	memWriteBankFinish.resize(bankNum);

	//lseWriteBankFinishHistory.resize(bankNum);

	cnt.resize(bankNum);

	bankInLoad.resize(bankNum);


	// initial SPM buffer's bank number
	_spmBuffer.resize(bankNum);

	// initial SPM buffer depth
	for (auto& i : _spmBuffer)
	{
		i.resize(bankDepth);
	}

	// initial SPM status
	for (size_t i = 0; i < bankNum; ++i)
	{
		bankFull[i] = 0;
		bankEmpty[i] = 1;
		rdPtrLse[i] = 0;
		wrPtrLse[i] = 0;
		rdPtrMem[i] = 0;
		//basePtr[i] = 0;

		//lseReadBankFinish[i].resize(2);
		//lseWriteBankFinish[i] = 0;

		//memReadBankFinish[i] = 0;
		//memWriteBankFinish[i] = 0;

		//lseWriteBankFinishHistory[i] = 0;

		cnt[i] = 0;

		/*bankInLoad[i] = 0;*/
	}

}
void SpmBuffer::spmReset()
{
	for (size_t i = 0; i < bankNum; ++i)
	{
		rdPtrLse[i] = 0;
		wrPtrLse[i] = 0;
		rdPtrMem[i] = 0;
		//basePtr[i] = 0;
		bankFull[i] = 0;
		bankEmpty[i] = 0;
	}
}
uint SpmBuffer::getRdPtrLse(const uint bankId)
{
	return rdPtrLse[bankId];
}

uint SpmBuffer::getWrPtrLse(const uint bankId)
{
	return wrPtrLse[bankId];
}

uint SpmBuffer::getRdPtrMem(const uint bankId)
{
	return rdPtrMem[bankId];
}

bool SpmBuffer::updateRdPtrLse(uint rdPtrLse, vector<LseConfig> lseConfig)
{
	bool readBankEmpty = 0;
	bool findNextPtr = 0;
	uint nextRdPtr;

	for (uint rowId = rdPtrLse; rowId < bankDepth; ++rowId)
	{
		for (auto config : lseConfig)
		{
			if (config._lseMode == LSMode::load)
			{
				uint bankId = config.lseVirtualTag;

				if (_spmBuffer[bankId][rowId].valid == 1 && _spmBuffer[bankId][rowId].dataReady != 1)  // signify data hasn't been read from memory
				{
					nextRdPtr = rowId;
					findNextPtr = 1;
					break;
				}
			}
		}

		if (findNextPtr)
			break;
	}

	if (findNextPtr)
	{
		for (auto config : lseConfig)
		{
			if (config._lseMode == LSMode::load)
			{
				uint bankId = config.lseVirtualTag;

				this->rdPtrLse[bankId] = nextRdPtr;
			}
		}

		return readBankEmpty = 0;
	}
	else
		return readBankEmpty = 1;

}

void SpmBuffer::updateWrPtrLse(const uint bankId)  // only sequentially write, update wrPtrLse
{
	if (wrPtrLse[bankId] < bankDepth - 1)
	{
		++wrPtrLse[bankId];
	}
}

void SpmBuffer::updateRdPtrMem(const uint bankId)
{
	if (rdPtrMem[bankId] < bankDepth - 1)
	{
		++rdPtrMem[bankId];
	}
}

void SpmBuffer::resetWrPtrLse(uint bankId)
{
	wrPtrLse[bankId] = 0;
}

void SpmBuffer::resetRdPtrLse(uint bankId)
{
	rdPtrLse[bankId] = 0;
}

void SpmBuffer::resetRdPtrMem(uint bankId)
{
	rdPtrMem[bankId] = 0;
}


// get SPM bank full status
bool SpmBuffer::isBankFull(const uint bankId)
{
	return bankFull[bankId];
}

// get SPM bank empty status
bool SpmBuffer::isBankEmpty(const uint bankId)
{
	return bankEmpty[bankId];
}

bool SpmBuffer::checkBankFull(const uint bankId)
{
	bool full = 1;

	for (size_t i = 0; i < bankDepth; ++i)
	{
		full = full & _spmBuffer[bankId][i].valid;
	}

	if (full == 1)
		bankFull[bankId] = 1;
	else
		bankFull[bankId] = 0;

	return full;
}

bool SpmBuffer::checkBankEmpty(const uint bankId)
{
	bool empty = 0;

	for (size_t i = 0; i < bankDepth; ++i)
	{
		empty = empty | _spmBuffer[bankId][i].valid;
	}

	if (empty == 0)
		bankEmpty[bankId] = 1;
	else
		bankEmpty[bankId] = 0;

	return ~empty;
}

bool SpmBuffer::checkBankEmptyWithCond(const uint bankId, bool condition)
{
	bool empty = 0;

	for (size_t i = 0; i < bankDepth; ++i)
	{
		if (_spmBuffer[bankId][i].condition == condition)
		{
			empty = empty | _spmBuffer[bankId][i].valid;
		}
	}

	return ~empty;
}


// get SPM data, provide to "class SPM" 
Port_inout_lsu SpmBuffer::getSpmData(const uint bankId, const uint rowId)
{
	return _spmBuffer[bankId][rowId];
}

// set data to SPM, provide to "class SPM"
void SpmBuffer::setSpmData(const uint bankId, const uint rowId, const Port_inout_lsu data)
{
	_spmBuffer[bankId][rowId] = data;
}

bool SpmBuffer::queueNotEmpty(queue<bool> q)
{
	return ~q.empty();
}

bool SpmBuffer::queueNotFull(queue<bool> q)
{
	if (q.size() < 2)  // most record two read/write records simultaneously 
		return true;
	else
		return false;
}


Port_inout_lsu SpmBuffer::readData(const uint bankId, const uint rowId)
{
	if (_spmBuffer[bankId][rowId].valid == 0)
		throw std::runtime_error("read SPM address error -> try to read an invalid data");
	else
	{
		Port_inout_lsu data = _spmBuffer[bankId][rowId];
		data.dataReady = 0; // clear dataReady
		_spmBuffer[bankId][rowId].valid = 0;  // clear valid flag after data read (only in SPM, keep valid flag in current data)

		return data;
	}
}

Port_inout_lsu SpmBuffer::readAddr(const uint bankId, const uint rowId)
{
	if (_spmBuffer[bankId][rowId].valid == 0)
		throw std::runtime_error("read SPM address error -> try to read an invalid address");
	else if (_spmBuffer[bankId][rowId].dataReady)
		throw std::runtime_error("read SPM address error -> there is a ready data hasn't been read out");
	else
	{
		_spmBuffer[bankId][rowId].inflight = 1;  // set inflight, indicate this memory request has been sent to the memory

		// record the number of load requests inflight, used for OoO memory access
		//++cnt[bankId];
		//if (cnt[bankId] > bankDepth)
		//	throw std::runtime_error("read SPM bank error -> current cnt > bank depth");

		return _spmBuffer[bankId][rowId];
	}
}

void SpmBuffer::writeData(const uint bankId, const uint rowId, Port_inout_lsu data)
{
	if (_spmBuffer[bankId][rowId].valid == 1)
		throw std::runtime_error("write SPM address conflict -> there is already a valid data");
	else
	{
		_spmBuffer[bankId][rowId] = data;
		_spmBuffer[bankId][rowId].valid = 1;  // set valid flag after data write
		_spmBuffer[bankId][rowId].bankId = bankId;  // bind bankId, due to writeMemAck need to write back to the corrsponding bank
		_spmBuffer[bankId][rowId].rowId = rowId;  // bind rowId, due to writeMemAck need to write back to the corrsponding row

		//++cnt[bankId];
		//if (cnt[bankId] > bankDepth)
		//	throw std::runtime_error("write SPM bank error -> current cnt > bank depth");
	}
}

void SpmBuffer::writeMemAck(const uint bankId, const uint rowId, Port_inout_lsu data)
{
	if (_spmBuffer[bankId][rowId].valid != 1 && _spmBuffer[bankId][rowId].inflight != 1)
		throw std::runtime_error("write back memory ack to SPM error -> there is not a valid inflight request");
	else
	{
		_spmBuffer[bankId][rowId] = data;
		_spmBuffer[bankId][rowId].inflight = 0;  // clear inflight, indicate the memory ack is already, the data has been load successfully
		_spmBuffer[bankId][rowId].dataReady = 1; // indicate data is ready

		// record the number of load requests inflight, used for OoO memory access
		//--cnt[bankId];
		//if (cnt[bankId] < 0)
		//	throw std::runtime_error("write memory Ack to SPM bank error -> current cnt < 0");
	}
}


// read data from SPM to LSE
Port_inout_lsu SpmBuffer::readSpm2Lse(const uint bankId, const uint rowId)
{
	if (_spmBuffer[bankId][rowId].valid != 1)
		throw std::runtime_error("try to read a invalid data in SPM -> the memory load has't been completed");

	return readData(bankId, rowId);
}

// write data from LSE to SPM; 
// use for 1) store temp data in branch or non-branch; 2) store address in stream/irregular-memory-access mode; 
void SpmBuffer::writeLse2Spm(const uint bankId, const uint rowId, const Port_inout_lsu data)
{
	if (checkBankFull(bankId))
		throw std::runtime_error("try to write a full SPM bank");

	writeData(bankId, rowId, data);
	updateWrPtrLse(bankId);  // update wrPtrLse, duo to sequentially write
}

// read addr. from SPM to Memory
// use for send addr to memory when load data in DAE
Port_inout_lsu SpmBuffer::readSpm2Mem(const uint bankId, const uint rowId)
{
	Port_inout_lsu addr = readAddr(bankId, rowId);

	// record the number of load requests inflight, used for OoO memory access
	++cnt[bankId];
	if (cnt[bankId] > bankDepth)
		throw std::runtime_error("read SPM bank error -> current cnt > bank depth");

	return addr;
}

// write memory ack back to SPM
// use for 1) load data in stream/irregular-memory-access mode;
void SpmBuffer::writeMem2Spm(const uint bankId, const uint rowId, const Port_inout_lsu data)
{
	writeMemAck(bankId, rowId, data);

	// record the number of load requests inflight, used for OoO memory access
	--cnt[bankId];
	if (cnt[bankId] > bankDepth)
		throw std::runtime_error("read SPM bank error -> current cnt < 0");

	// all load request have been responsed, the data load phase completes.
	if (cnt[bankId] == 0)
	{
		if (queueNotFull(memWriteBankFinish[bankId]))
		{
			memWriteBankFinish[bankId].push(1);
		}
	}
}

Spm::Spm(unordered_map<NodeType, vector<vector<const Simulator::Preprocess::DFGNodeInterface*>>> context_attr_, ClusterGroup & cluster_group_, map<uint, Simulator::Array::Loadstore_element*> & lse_map_, std::map<std::pair<NodeType, uint>, uint> & index2order_) :
	cluster_group(cluster_group_), lse_map(lse_map_), index2order(index2order_)//
{
	for (auto& context : context_attr_[NodeType::ls]) {
		vector<LseConfig> context_config;
		for (auto& attr : context) {
			LseConfig lse_config(attr);
			context_config.push_back(lse_config);
		}
		_lseConfig.push_back(context_config);
	}
	bankNum = Preprocess::Para::getInstance()->getArrayPara().spm_bank;  // SPM bank number is equal to LSE virtual number
	bankDepth = Preprocess::Para::getInstance()->getArrayPara().SPM_depth;  // initial SPM buffer depth
	spmInput.resize(bankNum);
	spmOutput.resize(bankNum);
}
// provide for Scheduler to add a new context to the SPM
void Spm::addContext(uint contextId)
{
	contextQueue.push_back(contextId);

	// reset contextFinish before push a new context
	for (auto config : _lseConfig[contextId])
	{
		config.contextFinish = 0;
	}
}

// provide for Scheduler to delete a finished context in the SPM
void Spm::removeContext()
{
	if (contextQueue.empty())
		throw std::runtime_error("pop contextQueue error -> try to pop an empty contextQueue");
	else
		contextQueue.pop_front();
}

// interface for Scheduler
uint Spm::getContextQueueHead()
{
	if (!contextQueue.empty())
		return contextQueue.front();
	else
		throw std::runtime_error("try to read an empty contextQueue");
}

uint Spm::getContextQueueTail()
{
	if (!contextQueue.empty())
		return contextQueue.back();
	else
		throw std::runtime_error("try to read an empty contextQueue");
}

bool Spm::contextQueueEmpty()
{
	return contextQueue.empty();
}


// update SPM in each cycle
void Spm::spmUpdate()
{
	for (auto contextId : contextQueue)  // traverse each context
	{
		for (auto& lseContext : _lseConfig[contextId])  // traverse each LSE's configuration in the current context
		{
			lse2Spm(lseContext);  // send temp data or addr from LSE to SPM
			//spm2Mem(lseContext);  // send addr from SPM to Mem
			//mem2Spm(lseContext);  // send load data from Mem to SPM
			//spm2Lse(lseContext);  // send temp data or load data from SPM to LSE
		}

		spm2Lse(contextId);  // send temp data or load data from SPM to LSE
	}

	spm2Mem();  // send addr from SPM to Mem
}
Simulator::Array::Loadstore_element* Spm::getLse(uint lse_tag) {
	return lse_map[index2order[{NodeType::ls, lse_tag}]];
}
void Spm::lse2Spm(LseConfig & context)
{
	uint lseTag = 0;
	auto data = getLse(lseTag)->getData();
	uint bankId = context.lseVirtualTag;

	if (data.valid)
	{
		if ((context._lseMode == LSMode::load && context._memAccessMode == MemAccessMode::load && context._daeMode == DaeMode::send_addr) ||
			(context._lseMode == LSMode::store_data && context._memAccessMode == MemAccessMode::temp))  // if context is 1) send addr. to SPM or 2) store temp data to SPM
		{
			uint rowId = _spmBuffer.getWrPtrLse(bankId);
			Port_inout_lsu rowData = _spmBuffer.getSpmData(bankId, rowId);

			if (rowData.valid != 1 && rowData.occupy != 1)  // if write valid; occupy flag is used in branch
			{
				if ((context._branchMode == BranchMode::truePath && data.condition == 1) ||
					(context._branchMode == BranchMode::falsePath && data.condition == 0) ||
					(context._branchMode == BranchMode::none))
				{
					if (~data.last)
					{
						// once a valid addr written in the bank, it begin to load from memory immediately
						if (context._lseMode == LSMode::load && context._memAccessMode == MemAccessMode::load && context._daeMode == DaeMode::send_addr)
						{
							data.dataReady = 0;  // for addr, dataReady set to 0
							_spmBuffer.writeLse2Spm(bankId, rowId, data);

							if (context.contextFinish != 1 && context.hasSetBankInLoad != 1)
							{
								if (_spmBuffer.queueNotFull(_spmBuffer.bankInLoad[bankId]))
								{
									_spmBuffer.bankInLoad[bankId].push(1);
									context.hasSetBankInLoad = 1;
								}
							}
						}

						if (context._lseMode == LSMode::store_data && context._memAccessMode == MemAccessMode::temp)
						{
							data.dataReady = 1;  // for temp data, dataReady set to 1
							_spmBuffer.writeLse2Spm(bankId, rowId, data);
						}
					}
					else
					{
						if (context.contextFinish != 1)
						{
							if (_spmBuffer.queueNotFull(_spmBuffer.lseWriteBankFinish[bankId]))
							{
								_spmBuffer.lseWriteBankFinish[bankId].push(1);
								_spmBuffer.resetWrPtrLse(bankId);  // reset wrPtrLse for next bank write
								context.contextFinish = 1;  // signify current context has finished
							}
						}
						//_spmBuffer.setLseWriteBankFinishHistory(bankId);
						//_spmBuffer.wrPtrReset(bankId);  // don't reset rdPtrLse 
					}
				}
				else
				{
					data.valid = 0;
					data.occupy = 1;
					_spmBuffer.writeLse2Spm(bankId, rowId, data); // write an occupy data to SPM, to occupy a buffer's row; Due to the data matching is according to the row; 
				}
			}

			// check whether Lse writes finish
			if (_spmBuffer.getWrPtrLse(bankId) == bankDepth - 1)
			{
				if (context.contextFinish != 1)
				{
					if (_spmBuffer.queueNotFull(_spmBuffer.lseWriteBankFinish[bankId]))
					{
						_spmBuffer.lseWriteBankFinish[bankId].push(1);
						_spmBuffer.resetWrPtrLse(bankId);  // reset wrPtrLse for next bank write
						context.contextFinish = 1;  // signify current context has finished
					}
				}
				//_spmBuffer.setLseWriteBankFinishHistory(bankId);
			}
		}
	}

	//if (_spmBuffer.getLseWriteBankFinish(bankId) && !_spmBuffer.getLseWriteBankFinishHistory(bankId))  // current LSE write SPM is finish && the data written by LSE last write operation has been read empty 
	//{
	//	_spmBuffer.setLseWriteBankFinishHistory(bankId);
	//	_spmBuffer.resetLseWriteBankFinish(bankId);
	//}
}

void Spm::spm2Mem()
{
	for (size_t bankId = 0; bankId < bankNum; ++bankId)
	{
		if (_spmBuffer.queueNotEmpty(_spmBuffer.bankInLoad[bankId]))  // when current bank is in load status
		{
			if (true)  // when the datapath to memory not stall, due to the memory is only 8 banks/////////////////////////////////////////////////////////////////////////
			{
				uint rowId = _spmBuffer.getRdPtrMem(bankId);

				for (size_t i = rowId; i < bankDepth; ++rowId)
				{
					Port_inout_lsu data = _spmBuffer.getSpmData(bankId, rowId);

					if (data.valid && data.dataReady != 1 && data.inflight != 1)
					{
						data = _spmBuffer.readSpm2Mem(bankId, rowId);
						lsu->AddTrans(data,data.value_addr,false);  // send this addr. to memory
						_spmBuffer.updateRdPtrMem(bankId);  // update rdPtrMem

						break;
					}

					if (rowId == bankDepth - 1)  // execute to the last loop, indicates no addr has been read out
					{
						if (_spmBuffer.queueNotEmpty(_spmBuffer.lseWriteBankFinish[bankId]))
						{
							if (_spmBuffer.queueNotFull(_spmBuffer.memReadBankFinish[bankId]))
							{
								_spmBuffer.memReadBankFinish[bankId].push(1);
								_spmBuffer.lseWriteBankFinish[bankId].pop();  // clear current status
								_spmBuffer.bankInLoad[bankId].pop();  // clear current status
							}

							//_spmBuffer.bankInLoad[bankId].pop();  // indicate current bank has been loaded completely
							_spmBuffer.resetRdPtrMem(bankId);
						}
					}
				}
			}
		}
	}
}

void Spm::mem2Spm(const Port_inout_lsu data)
{
	uint bankId = data.bankId;
	uint rowId = data.rowId;

	if (data.valid)
	{
		_spmBuffer.writeMem2Spm(bankId, rowId, data);
	}

	// check memory write bank finish
	if (_spmBuffer.queueNotEmpty(_spmBuffer.memReadBankFinish[bankId]))  // when all the addr. have been sent to SPM from LSE
	{
		for (size_t i = _spmBuffer.getWrPtrLse[bankId]; i < bankDepth; ++i)  // traverse from wrPtrLse, in order to exclude the next round lse write;
		{
			Port_inout_lsu data = _spmBuffer.getSpmData(bankId, uint(i));

			if (data.valid && data.dataReady != 1)
			{
				//_spmBuffer.resetMemWriteBankFinish(bankId);
				break;
			}

			if (i = bankDepth - 1)
			{
				if (_spmBuffer.queueNotFull(_spmBuffer.memWriteBankFinish[bankId]))
				{
					_spmBuffer.memWriteBankFinish[bankId].push(1);
					_spmBuffer.memReadBankFinish[bankId].pop();
				}
			}
		}
	}
}

void Spm::spm2Lse(uint contextId)
{
	// send last when bankReadFinish!!!
	// data match

	BranchMode lseBranMode = _lseConfig[contextId][0]._branchMode;
	// check branchMode consistance
	for (auto lseContext : _lseConfig[contextId])
	{
		if (lseContext._branchMode != lseBranMode)
			throw std::runtime_error("LSE branchMode configure error -> All the LSE belong to the same context must have the same branchMode");
	}

	//**** check rdPtrLse consistance  ***//
	uint rdPtrLse;
	bool hasLoad = 0;  // signify there is at least one LSE in the load mode
	for (auto lseContext : _lseConfig[contextId])
	{
		if (lseContext._lseMode == LSMode::load)
		{
			uint bankId = lseContext.lseVirtualTag;
			rdPtrLse = _spmBuffer.getRdPtrLse(bankId);
			hasLoad = 1;
			break;
		}
	}

	if (hasLoad)
	{
		for (auto lseContext : _lseConfig[contextId])
		{
			if (lseContext._lseMode == LSMode::load)
			{
				uint bankId = lseContext.lseVirtualTag;

				if (_spmBuffer.getRdPtrLse(bankId) != rdPtrLse)
					throw std::runtime_error("read data from SPM to LSE error -> rdPtrLse in each bank in load mode must be the same");
			}
		}
	}
	//***********************//

	for (size_t rowId = rdPtrLse; rowId < bankDepth; ++rowId)  // roundrobin start at the rdPtrLse, stop at the bank tail
	{
		bool dataMatch = 1;
		bool bankFinish = 1;
		bool bankReadEmpty = 0;

		for (auto lseContext : _lseConfig[contextId])
		{
			uint bankId = lseContext.lseVirtualTag;
			Port_inout_lsu data = _spmBuffer.getSpmData(bankId, rowId);

			if (lseContext._lseMode == LSMode::load)
			{
				if (lseBranMode == BranchMode::none ||
					(lseBranMode == BranchMode::falsePath && !data.condition) ||
					(lseBranMode == BranchMode::truePath && data.condition))
				{
					dataMatch = dataMatch & (data.valid & data.dataReady);  // due to SPM data contain load data and temp data, so only when valid & dataReady all set to 1 signify the data is valid;  
				}
			}

			if (lseContext._lseMode == LSMode::load && lseContext._memAccessMode == MemAccessMode::temp)
			{
				bankFinish = bankFinish & _spmBuffer.queueNotEmpty(_spmBuffer.lseWriteBankFinish[bankId]);
			}
			else if (lseContext._lseMode == LSMode::load && lseContext._memAccessMode == MemAccessMode::load && lseContext._daeMode == DaeMode::get_data)
			{
				bankFinish = bankFinish & _spmBuffer.queueNotEmpty(_spmBuffer.memWriteBankFinish[bankId]);
			}

		}

		if (dataMatch) // if match successfully
		{
			for (auto lseContext : _lseConfig[contextId])
			{
				uint bankId = lseContext.lseVirtualTag;

				if (lseContext._lseMode == LSMode::load)
				{
					//sendData2Lse(_spmBuffer.readSpm2Lse(bankId, rowId));///////////////////////////////////////////////////////////

					if (rowId == rdPtrLse)
					{
						bankReadEmpty = _spmBuffer.updateRdPtrLse(rdPtrLse, _lseConfig[contextId]);  // 1)update rdPtrLse and 2)check whether the bank is read empty
					}
				}
			}

			break;
		}

		if (bankReadEmpty && bankFinish)  // there is no data match
		{
			for (auto& lseContext : _lseConfig[contextId])
			{
				if (lseContext._lseMode == LSMode::load)
				{
					uint bankId = lseContext.lseVirtualTag;

					if (lseContext.contextFinish != 1)
					{
						if (lseContext._lseMode == LSMode::load && lseContext._memAccessMode == MemAccessMode::temp)
						{
							_spmBuffer.lseWriteBankFinish[bankId].pop();  // clear lse write status
						}

						if (lseContext._lseMode == LSMode::load && lseContext._memAccessMode == MemAccessMode::load && lseContext._daeMode == DaeMode::get_data)
						{
							_spmBuffer.memWriteBankFinish[bankId].pop();  // clear mem write status
						}

						lseContext.contextFinish = 1;
					}

					// send last data to each LSE, to indicate current context is over
					Port_inout_lsu data;
					data.last = 1;
					//sendData2Lse(data);/////////////////////////////////////////////////
				}
			}
		}
	}
}

void Spm::attachLsu(Lsu* lsu_) 
{
	lsu = lsu_;
}


LseConfig::LseConfig()
{
	lseTag = 0;
	lseVirtualTag = 0;
	_lseMode = LSMode::null;
	_memAccessMode = MemAccessMode::none;
	_daeMode = DaeMode::none;
	_branchMode = BranchMode::none;

	contextFinish = 1;  // initial value set 1, scheduler push a context in contextQueue only when its contextFinish equal to 1
	hasSetBankInLoad = 0;  // for only push BankInLoad Queue once in each context
}
LseConfig::LseConfig(const Simulator::Preprocess::DFGNodeInterface * attr_)
{
	auto attribution = dynamic_cast<const Preprocess::DFGNode<Simulator::NodeType::ls>*>(attr_);
	lseTag = attribution->index;
	lseVirtualTag = attribution->cluster;
	_lseMode = attribution->ls_mode;
	_memAccessMode = attribution->mem_access_mode;
	_daeMode = attribution->dae_mode;
	_branchMode = attribution->branch_mode;

	contextFinish = 1;  // initial value set 1, scheduler push a context in contextQueue only when its contextFinish equal to 1
	hasSetBankInLoad = 0;  // for only push BankInLoad Queue once in each context
}