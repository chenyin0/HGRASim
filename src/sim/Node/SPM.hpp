#pragma once

#include "iostream"                        
#include "stdlib.h"                       
#include "math.h"                         //pay attention to ACK
#include <vector>
#include <deque>
#include <queue>
#include <map>
#include "../mem_system/MultiChannelMemorySystem.h"
#include "../mem_system/Cache.h"
#include "../inout.h"
//#include "../Node/LSE.h" 
#include "../Node/Node.h"
#include "../../preprocess/para.hpp"

namespace Simulator::Array
{
	class SpmBuffer;
	class Spm;
	struct LseConfig;

	class SpmBuffer 
	{
	public:
		SpmBuffer(uint _bankNum, uint _bankDepth) : bankNum(_bankNum), bankDepth(_bankDepth)
		{
			spmInit();
		}

		void spmInit()
		{
			/*spmInput.resize(bankNum);
			spmOutput.resize(bankNum);*/
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

			//// initial bank id
			//bankId.resize(bankNum);
			//for (size_t i = 0; i < bankId.size(); ++i)
			//{
			//	bankId[i] = i;
			//}

			//// initial row id
			//rowId.resize(bankDepth);
			//for (size_t i = 0; i < rowId.size(); ++i)
			//{
			//	rowId[i] = i;
			//}

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

		// reset SPM stauts
		void spmReset()
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


		// get SPM rdPtrLse
		uint getRdPtrLse(const uint bankId)
		{
			return rdPtrLse[bankId];
		}

		// get SPM wrPtrLse
		uint getWrPtrLse(const uint bankId)
		{
			return wrPtrLse[bankId];
		}

		uint getRdPtrMem(const uint bankId)
		{
			return rdPtrMem[bankId];
		}

		bool updateRdPtrLse(uint rdPtrLse, vector<LseConfig> lseConfig)
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

				if(findNextPtr)
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

		void updateWrPtrLse(const uint bankId)  // only sequentially write, update wrPtrLse
		{
			if (wrPtrLse[bankId] < bankDepth - 1)
			{
				++wrPtrLse[bankId];
			}
		}

		void updateRdPtrMem(const uint bankId)
		{
			if (rdPtrMem[bankId] < bankDepth - 1)
			{
				++rdPtrMem[bankId];
			}
		}

		void resetWrPtrLse(uint bankId)
		{
			wrPtrLse[bankId] = 0;
		}

		void resetRdPtrLse(uint bankId)
		{
			rdPtrLse[bankId] = 0;
		}

		void resetRdPtrMem(uint bankId)
		{
			rdPtrMem[bankId] = 0;
		}


		// get SPM bank full status
		bool isBankFull(const uint bankId)
		{
			return bankFull[bankId];
		}

		// get SPM bank empty status
		bool isBankEmpty(const uint bankId)
		{
			return bankEmpty[bankId];
		}

		bool checkBankFull(const uint bankId)
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

		bool checkBankEmpty(const uint bankId)
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

		bool checkBankEmptyWithCond(const uint bankId, bool cond)
		{
			bool empty = 0;

			for (size_t i = 0; i < bankDepth; ++i)
			{
				if (_spmBuffer[bankId][i].cond == cond)
				{
					empty = empty | _spmBuffer[bankId][i].valid;
				}
			}

			return ~empty;
		}


		// get SPM data, provide to "class SPM" 
		Port_inout_lsu getSpmData(const uint bankId, const uint rowId)
		{
			return _spmBuffer[bankId][rowId];
		}

		// set data to SPM, provide to "class SPM"
		void setSpmData(const uint bankId, const uint rowId, const Port_inout_lsu data)
		{
			_spmBuffer[bankId][rowId] = data;
		}

		bool queueNotEmpty(queue<bool> q)
		{
			return ~q.empty();
		}

		bool queueNotFull(queue<bool> q)
		{
			if (q.size() < 2)  // most record two read/write records simultaneously 
				return true;
			else
				return false;
		}

		//// interface for "class SPM"
		//void setLseReadBankFinish(uint bankId)
		//{
		//	lseReadBankFinish[bankId] = 1;
		//}

		//void setMemReadBankFinish(uint bankId)
		//{
		//	memReadBankFinish[bankId] = 1;
		//}

		//void setLseWriteBankFinish(uint bankId)
		//{
		//	lseWriteBankFinish[bankId] = 1;
		//	wrPtrLse[bankId] = 0;  // reset wrPtrLse for next bank write
		//}

		//void setMemWriteBankFinish(uint bankId)
		//{
		//	memWriteBankFinish[bankId] = 1;
		//}

		//void setLseWriteBankFinishHistory(uint bankId)
		//{
		//	lseWriteBankFinishHistory[bankId] = 1;
		//}

		//void resetLseReadBankFinish(uint bankId)
		//{
		//	lseReadBankFinish[bankId] = 0;
		//}

		//void resetMemReadBankFinish(uint bankId)
		//{
		//	memReadBankFinish[bankId] = 0;
		//}

		//void resetLseWriteBankFinish(uint bankId)
		//{
		//	lseWriteBankFinish[bankId] = 0;
		//}

		//void resetMemWriteBankFinish(uint bankId)
		//{
		//	memWriteBankFinish[bankId] = 0;
		//}

		//void resetLseWriteBankFinishHistory(uint bankId)
		//{
		//	lseWriteBankFinishHistory[bankId] = 0;
		//}

		//// interface function to get SPM bank write/read status
		//bool getLseReadBankFinish(uint bankId)
		//{
		//	return lseReadBankFinish[bankId];
		//}

		//bool getMemReadBankFinish(uint bankId)
		//{
		//	return memReadBankFinish[bankId];
		//}

		//bool getLseWriteBankFinish(uint bankId)
		//{
		//	return lseWriteBankFinish[bankId];
		//}

		//bool getMemWriteBankFinish(uint bankId)
		//{
		//	return memWriteBankFinish[bankId];
		//}
		//
		//bool getLseWriteBankFinishHistory(uint bankId)
		//{
		//	return lseWriteBankFinishHistory[bankId];
		//}

		//// interface function for Spm2Mem
		//void setBankInLoad(uint bankId, bool flag)
		//{
		//	bankInLoad[bankId] = flag;
		//}

		//bool getBankInLoad(uint bankId)
		//{
		//	return bankInLoad[bankId];
		//}


		Port_inout_lsu readData(const uint bankId, const uint rowId)
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

		Port_inout_lsu readAddr(const uint bankId, const uint rowId)
		{
			if (_spmBuffer[bankId][rowId].valid == 0)
				throw std::runtime_error("read SPM address error -> try to read an invalid address");
			else if(_spmBuffer[bankId][rowId].dataReady)
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

		void writeData(const uint bankId, const uint rowId, Port_inout_lsu data)
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

		void writeMemAck(const uint bankId, const uint rowId, Port_inout_lsu data)
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
		Port_inout_lsu readSpm2Lse(const uint bankId, const uint rowId)
		{
			if (_spmBuffer[bankId][rowId].valid != 1)
				throw std::runtime_error("try to read a invalid data in SPM -> the memory load has't been completed");

			return readData(bankId, rowId);
		}

		// write data from LSE to SPM; 
		// use for 1) store temp data in branch or non-branch; 2) store address in stream/irregular-memory-access mode; 
		void writeLse2Spm(const uint bankId, const uint rowId, const Port_inout_lsu data)
		{
			if(checkBankFull(bankId))
				throw std::runtime_error("try to write a full SPM bank");

			writeData(bankId, rowId, data);
			updateWrPtrLse(bankId);  // update wrPtrLse, duo to sequentially write
		}

		// read addr. from SPM to Memory
		// use for send addr to memory when load data in DAE
		Port_inout_lsu readSpm2Mem(const uint bankId, const uint rowId)
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
		void writeMem2Spm(const uint bankId, const uint rowId, const Port_inout_lsu data)
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

	public:
		vector<queue<bool>> memReadBankFinish;
		vector<queue<bool>> memWriteBankFinish;
		vector<queue<bool>> lseReadBankFinish;
		vector<queue<bool>> lseWriteBankFinish;
		//vector<bool> lseWriteBankFinishHistory;  // record last write status for bank read, due to bank read finish check need bank write finish history
		vector<queue<bool>> bankInLoad;  // if a bank is in the sending addr. to memory status, set the flag to 1; If all the addr. has been sent to the memory, reset it to 0

	private:
		/*vector<Port_inout_lsu> spmInput;
		vector<Port_inout_lsu> spmOutput;*/
		vector<uint> rdPtrLse;  // record the row number in last LSE read, for roundrobin 
		vector<uint> wrPtrLse;  // record the row number in last LSE write, for roundrobin 
		vector<uint> rdPtrMem;  // record the row number in last memory read, due to memory read SPM sequentially
		//vector<uint> basePtr;  // write/read data to SPM base on this ptr, which bank is read empty, reset its basePtr;
		vector<int> cnt;
		vector<bool> bankFull;
		vector<bool> bankEmpty;
		//vector<queue<bool>> memReadBankFinish;
		//vector<queue<bool>> memWriteBankFinish;
		//vector<queue<bool>> lseReadBankFinish;
		//vector<queue<bool>> lseWriteBankFinish;
		////vector<bool> lseWriteBankFinishHistory;  // record last write status for bank read, due to bank read finish check need bank write finish history
		//vector<queue<bool>> bankInLoad;  // if a bank is in the sending addr. to memory status, set the flag to 1; If all the addr. has been sent to the memory, reset it to 0
		uint bankNum;
		uint bankDepth;
		vector<vector<Port_inout_lsu>> _spmBuffer;  // vector_1<vector_2>; vector_1 = bank, vector_2 = row;
		/*vector<uint> bankId;
		vector<uint> rowId;*/
	};


	using Context = vector<vector<LseConfig>>;  // vector1<vector2<LseConfig>>  vector1:each context  vector2:each LSE configured in current context

	class Spm
	{
	public:
		Spm(Context lseConfig) : _lseConfig(lseConfig)
		{
			bankNum = Preprocess::Para::getInstance()->getArrayPara().lse_virtual_num;  // SPM bank number is equal to LSE virtual number
			bankDepth = Preprocess::Para::getInstance()->getArrayPara().SPM_depth;  // initial SPM buffer depth

			spmInput.resize(bankNum);
			spmOutput.resize(bankNum);
		}

		// provide for Scheduler to add a new context to the SPM
		void addContext(uint contextId)
		{
			contextQueue.push_back(contextId);
		}

		// provide for Scheduler to delete a finished context in the SPM
		void removeContext(uint contextId)
		{
			if (contextId != contextQueue.front())
				throw std::runtime_error("delete context in contextQueue error -> the context to delete is not the first one in the queue");
			else
				contextQueue.pop_front();
		}

		// interface for Scheduler
		uint getContextQueueHead()
		{
			if(!contextQueue.empty())
				return contextQueue.front();
			else 
				throw std::runtime_error("try to read an empty contextQueue");
		}

		uint getContextQueueTail()
		{
			if(!contextQueue.empty())
				return contextQueue.back();
			else
				throw std::runtime_error("try to read an empty contextQueue");
		}
		

		//// interface for "class Scheduler"
		//void setLseReadBankFinish(uint bankId)
		//{
		//	_spmBuffer.setLseReadBankFinish(bankId);
		//}

		//void setMemReadBankFinish(uint bankId)
		//{
		//	_spmBuffer.setMemReadBankFinish(bankId);
		//}

		//void setLseWriteBankFinish(uint bankId)
		//{
		//	_spmBuffer.setLseWriteBankFinish(bankId);
		//}

		//void setMemWriteBankFinish(uint bankId)
		//{
		//	_spmBuffer.setMemWriteBankFinish(bankId);
		//}

		//void resetLseReadBankFinish(uint bankId)
		//{
		//	_spmBuffer.resetLseReadBankFinish(bankId);
		//}

		//void resetMemReadBankFinish(uint bankId)
		//{
		//	_spmBuffer.resetMemReadBankFinish(bankId);
		//}

		//void resetLseWriteBankFinish(uint bankId)
		//{
		//	_spmBuffer.resetLseWriteBankFinish(bankId);
		//}

		//void resetMemWriteBankFinish(uint bankId)
		//{
		//	_spmBuffer.resetMemWriteBankFinish(bankId);
		//}

		//// interface function for "class Scheduler" to get SPM bank write/read status
		//bool getLseReadBankFinish(uint bankId)
		//{
		//	return _spmBuffer.getLseReadBankFinish(bankId);
		//}

		//bool getMemReadBankFinish(uint bankId)
		//{
		//	return _spmBuffer.getMemReadBankFinish(bankId);
		//}

		//bool getLseWriteBankFinish(uint bankId)
		//{
		//	return _spmBuffer.getLseWriteBankFinish(bankId);
		//}

		//bool getMemWriteBankFinish(uint bankId)
		//{
		//	return _spmBuffer.getMemWriteBankFinish(bankId);
		//}


		// update SPM in each cycle
		void spmUpdate()
		{
			for (auto contextId : contextQueue)  // traverse each context
			{
				for (auto lseContext : _lseConfig[contextId])  // traverse each LSE's configuration in the current context
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

		void lse2Spm(const LseConfig context)
		{
			Port_inout_lsu data = lseGetData(lseTag);
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
						if ((context._branchMode == BranchMode::truePath && data.cond == 1) ||
							(context._branchMode == BranchMode::falsePath && data.cond == 0) ||
							(context._branchMode == BranchMode::none))
						{
							if (~data.lastData)
							{
								// once a valid addr written in the bank, it begin to load from memory immediately
								if (context._lseMode == LSMode::load && context._memAccessMode == MemAccessMode::load && context._daeMode == DaeMode::send_addr)
								{
									data.dataReady = 0;  // for addr, dataReady set to 0
									_spmBuffer.writeLse2Spm(bankId, rowId, data);

									if (_spmBuffer.queueNotFull(_spmBuffer.bankInLoad[bankId]))
									{
										_spmBuffer.bankInLoad[bankId].push(1);
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
								if (_spmBuffer.queueNotFull(_spmBuffer.lseWriteBankFinish[bankId]))
								{
									_spmBuffer.lseWriteBankFinish[bankId].push(1);
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
						if (_spmBuffer.queueNotFull(_spmBuffer.lseWriteBankFinish[bankId]))
						{
							_spmBuffer.lseWriteBankFinish[bankId].push(1);
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

		void spm2Mem()
		{
			for (size_t bankId = 0; bankId < bankNum; ++bankId)
			{
				if (_spmBuffer.queueNotEmpty(_spmBuffer.bankInLoad[bankId]))  // when current bank is in load status
				{
					if (!memory_ACK)  // when the datapath to memory not stall, due to the memory is only 8 banks
					{
						uint rowId = _spmBuffer.getRdPtrMem(bankId);

						for (size_t i = rowId; i < bankDepth; ++rowId)
						{
							Port_inout_lsu data = _spmBuffer.getSpmData(bankId, rowId);

							if (data.valid && data.dataReady != 1 && data.inflight != 1)
							{
								data = _spmBuffer.readSpm2Mem(bankId, rowId);
								sendData2Mem(data);  // send this addr. to memory
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
									}

									_spmBuffer.bankInLoad[bankId].pop();  // indicate current bank has been loaded completely
									_spmBuffer.resetRdPtrMem(bankId);
								}
							}
						}
					}
				}
			}
		}

		void mem2Spm(const Port_inout_lsu data)
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
				for (size_t i = 0; i < bankDepth; ++i)
				{
					Port_inout_lsu data = _spmBuffer.getSpmData(bankId, uint(i));

					if (data.valid && data.dataReady != 1)
					{
						//_spmBuffer.resetMemWriteBankFinish(bankId);
						break;
					}
					
					if (i = bankDepth - 1)
					{
						if(_spmBuffer.queue)
					}
				}
			}
		}

		void spm2Lse(uint contextId)
		{
			// send lastData when bankReadFinish!!!
			// data match

			BranchMode lseBranMode = _lseConfig[contextId][0]._branchMode;
			// check branchMode consistance
			for (auto lseContext : _lseConfig[contextId])
			{
				if (lseContext._branchMode != lseBranMode)
					throw std::runtime_error("LSE branchMode configure error -> All the LSE belong to the same context must have the same branchMode");
			}

			// check rdPtrLse consistance
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

						if(_spmBuffer.getRdPtrLse(bankId) != rdPtrLse)
							throw std::runtime_error("read data from SPM to LSE error -> rdPtrLse in each bank in load mode must be the same");
					}
				}
			}

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
							(lseBranMode == BranchMode::falsePath && !data.cond) ||
							(lseBranMode == BranchMode::truePath && data.cond))
						{
							dataMatch = dataMatch & (data.valid & data.dataReady);  // due to SPM data contain load data and temp data, so only when valid & dataReady all set to 1 signify the data is valid;  
						}
					}

					if (lseContext._lseMode == LSMode::load && lseContext._memAccessMode == MemAccessMode::temp)
					{
						bankFinish = bankFinish & _spmBuffer.getLseWriteBankFinishHistory(bankId);
					}
					else if (lseContext._lseMode == LSMode::load && lseContext._memAccessMode == MemAccessMode::load && lseContext._daeMode == DaeMode::get_data)
					{
						bankFinish = bankFinish & _spmBuffer.getMemWriteBankFinish(bankId);
					}
						
				}

				if (dataMatch) // if match successfully
				{
					for (auto lseContext : _lseConfig[contextId])
					{
						uint bankId = lseContext.lseVirtualTag;

						if (lseContext._lseMode == LSMode::load)
						{
							sendData2Lse(_spmBuffer.readSpm2Lse(bankId, rowId));

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
					for (auto lseContext : _lseConfig[contextId])
					{
						if (lseContext._lseMode == LSMode::load)
						{
							uint bankId = lseContext.lseVirtualTag;

							_spmBuffer.setLseReadBankFinish(bankId);
							_spmBuffer.resetLseWriteBankFinishHistory(bankId);  // reset lseWriteBankFinishHistory, indicate the data written by last LSE write has been read empty

							// send last data to each LSE, to indicate current context is over
							Port_inout_lsu data;
							data.lastData = 1;
							sendData2Lse(data);
						}
					}
				}
			}
		}

	private:
		vector<Port_inout_lsu> spmInput;
		vector<Port_inout_lsu> spmOutput;
		uint bankNum;
		uint bankDepth;
		SpmBuffer _spmBuffer = SpmBuffer(bankNum, bankDepth);

		Context _lseConfig;  // vector1<vector2<LseConfig>>  vector1:each context  vector2:each LSE configured in current context
		deque<uint> contextQueue;  // add/delete valid context by Scheduler, SPM may work under several contexts simultaneously
	};


	struct LseConfig
	{
	public:
		uint lseTag;  // lse node tag, number in .xml
		uint lseVirtualTag; // indicate this lse connect to which SPM bank
		LSMode _lseMode; // indicate current LSE mode

		MemAccessMode _memAccessMode;  // configure in .xml
		DaeMode _daeMode;  // configure in .xml
		BranchMode _branchMode;  // configure in .xml

		LseConfig()
		{
			lseTag = 0;
			lseVirtualTag = 0;
			_lseMode = LSMode::null;
			_memAccessMode = MemAccessMode::none;
			_daeMode = DaeMode::none;
			_branchMode = BranchMode::none;
		}
	};

}