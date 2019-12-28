#pragma once

#include "iostream"                        
#include "stdlib.h"                       
#include "math.h"                         //pay attention to ACK
#include <vector>
#include <deque>
#include <map>
#include "../mem_system/MultiChannelMemorySystem.h"
#include "../mem_system/Cache.h"
#include "../inout.h"
//#include "../Node/LSE.h" 
#include "../Node/Node.h"
#include "../../preprocess/para.hpp"

namespace Simulator::Array
{
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
			/*rdPtr.resize(bankNum);
			wrPtr.resize(bankNum);*/
			//basePtr.resize(bankNum);
			bankFull.resize(bankNum);
			bankEmpty.resize(bankNum);

			lseReadBankFinish.resize(bankNum);
			lseWriteBankFinish.resize(bankNum);
			memReadBankFinish.resize(bankNum);
			memWriteBankFinish.resize(bankNum);

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
				/*rdPtr[i] = 0;
				wrPtr[i] = 0;*/
				//basePtr[i] = 0;

				lseReadBankFinish[i] = 0;
				lseWriteBankFinish[i] = 0;

				memReadBankFinish[i] = 0;
				memWriteBankFinish[i] = 0;

				cnt[i] = 0;

				bankInLoad[i] = 0;
			}

		}

		// reset SPM stauts
		void spmReset()
		{
			for (size_t i = 0; i < bankNum; ++i)
			{
				/*rdPtr[i] = 0;
				wrPtr[i] = 0;*/
				//basePtr[i] = 0;
				bankFull[i] = 0;
				bankEmpty[i] = 0;
			}
		}

		//// reset bank read finish signal before a new context with SPM read
		//void resetBankReadFinish(uint bankId)
		//{
		//	bankReadFinish[bankId] = 0;
		//}

		//// reset bank write finish signal before a new context with SPM write
		//void resetBankWriteFinish(uint bankId)
		//{
		//	bankWriteFinish[bankId] = 0;
		//}

		/*void rdPtrReset(uint bankId)
		{
			rdPtr[bankId] = 0;
		}

		void wrPtrReset(uint bankId)
		{
			wrPtr[bankId] = 0;
		}*/

		/*void resetBasePtr(uint bankId)
		{
			basePtr[bankId] = 0;
		}*/

		//// get SPM rdPtr
		//uint getRdPtr(const uint bankId)
		//{
		//	return rdPtr[bankId];
		//}

		//// get SPM wrPtr
		//uint getWrPtr(const uint bankId)
		//{
		//	return wrPtr[bankId];
		//}

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

			/*if (empty == 0)
				bankEmpty[bankId] = 1;
			else
				bankEmpty[bankId] = 0;*/

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


		// interface for "class SPM"
		void setLseReadBankFinish(uint bankId)
		{
			lseReadBankFinish[bankId] = 1;
		}

		void setMemReadBankFinish(uint bankId)
		{
			memReadBankFinish[bankId] = 1;
		}

		void setLseWriteBankFinish(uint bankId)
		{
			lseWriteBankFinish[bankId] = 1;
		}

		void setMemWriteBankFinish(uint bankId)
		{
			memWriteBankFinish[bankId] = 1;
		}

		void resetLseReadBankFinish(uint bankId)
		{
			lseReadBankFinish[bankId] = 0;
		}

		void resetMemReadBankFinish(uint bankId)
		{
			memReadBankFinish[bankId] = 0;
		}

		void resetLseWriteBankFinish(uint bankId)
		{
			lseWriteBankFinish[bankId] = 0;
		}

		void resetMemWriteBankFinish(uint bankId)
		{
			memWriteBankFinish[bankId] = 0;
		}

		// interface function to get SPM bank write/read status
		bool getLseReadBankFinish(uint bankId)
		{
			return lseReadBankFinish[bankId];
		}

		bool getMemReadBankFinish(uint bankId)
		{
			return memReadBankFinish[bankId];
		}

		bool getLseWriteBankFinish(uint bankId)
		{
			return lseWriteBankFinish[bankId];
		}

		bool getMemWriteBankFinish(uint bankId)
		{
			return memWriteBankFinish[bankId];
		}
		

		// interface function for Spm2Mem
		void setBankInLoad(uint bankId, bool flag)
		{
			bankInLoad[bankId] = flag;
		}

		bool getBankInLoad(uint bankId)
		{
			return bankInLoad[bankId];
		}


		Port_inout_lsu readData(const uint bankId, const uint rowId)
		{	
			if (_spmBuffer[bankId][rowId].valid == 0)
				throw std::runtime_error("read SPM address error -> try to read an invalid data");
			else 
			{
				_spmBuffer[bankId][rowId].dataReady = 0; // clear data ready both in SPM & current data
				Port_inout_lsu data = _spmBuffer[bankId][rowId];
				_spmBuffer[bankId][rowId].valid = 0;  // clear valid flag after data read (only in SPM, keep valid flag in current data)

				//--cnt[bankId];
				//if(cnt[bankId] < 0 )
				//	throw std::runtime_error("read SPM bank error -> current cnt < 0");

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

		//void wrPtrUpdate(uint bankId)
		//{	
		//	wrPtr[bankId] = (++wrPtr[bankId])%bankDepth;

		//	//if (wrPtr[bankId] == bankDepth)
		//	//	wrPtr[bankId] = 0;

		//	//if (wrPtr[bankId] == rdPtr[bankId])
		//	//	bankFull[bankId] = 1;  // this bank is written full
		//}

		//void rdPtrUpdate(uint bankId)
		//{
		//	rdPtr[bankId] = (++rdPtr[bankId])%bankDepth;

		//	//if (rdptr[bankid] == bankdepth)
		//	//	rdptr[bankid] = 0;

		//	//if (rdptr[bankid] == wrptr[bankid])
		//	//	bankempty[bankid] = 1; // this bank is read empty
		//}


		// reset wrPtr & rdPtr when read SPM context finish
		// due to if current context read SPM, it must read the bank to empty before switch a new context 
		//void resetPtr(uint bankId)
		//{
		//	rdPtr[bankId] = 0;
		//	wrPtr[bankId] = 0;
		//	bankEmpty[bankId] = 1;
		//	bankFull[bankId] = 0;
		//}


		//// read data from SPM to LSE
		//// use for reading temp data not in branch, read as fifo-style
		//Port_inout_lsu readSpm2Lse_tempNoCond(const uint bankId)
		//{
		//	Port_inout_lsu data;

		//	if (bankEmpty[bankId] == 1)
		//		throw std::runtime_error("try to read an empty SPM bank");

		//	data = readData(bankId, rdPtr[bankId]);
		//	rdPtrUpdate(bankId);

		//	// if this is the last data of current context, bank read operation is over
		//	if (/*data.lastData == 1 ||*/ checkBankEmpty(bankId))
		//	{
		//		lseReadBankFinish[bankId] = 1;
		//		rdPtrReset(bankId);
		//		wrPtrReset(bankId);
		//	}
		//	else
		//		lseReadBankFinish[bankId] = 0;

		//	return data;
		//}

		//// read data from SPM to LSE
		//// use for reading temp data in branch, read according to the condition status
		//Port_inout_lsu readSpm2Lse_tempWithCond(const uint bankId, bool cond)
		//{
		//	Port_inout_lsu data;

		//	if (checkBankEmptyWithCond(bankId, cond))
		//		throw std::runtime_error("try to read an empty SPM bank -> there is no data consist with current condition");

		//	for (size_t i = 0; i < bankDepth; ++i)
		//	{
		//		if (_spmBuffer[bankId][rdPtr[bankId]].valid && _spmBuffer[bankId][rdPtr[bankId]].cond == cond)
		//		{
		//			data = readData(bankId, rdPtr[bankId]);
		//			rdPtrUpdate(bankId);

		//			if (/*data.lastData == 1 ||*/ checkBankEmptyWithCond(bankId, cond))
		//			{
		//				lseReadBankFinish[bankId] = 1;
		//				rdPtrReset(bankId);
		//				wrPtrReset(bankId);
		//			}

		//			return data;
		//		}
		//		else
		//		{
		//			rdPtrUpdate(bankId);
		//			lseReadBankFinish[bankId] = 0;
		//		}
		//	}
		//}

		//// token match operate in "class SPM", due to "class SpmBuffer" doesn't know which banks need to match
		//Port_inout_lsu readSpm2Lse_memLoad(const uint bankId, const uint rowId)
		//{
		//	if(_spmBuffer[bankId][rowId].valid != 1)
		//		throw std::runtime_error("try to read a invalid data in SPM -> the memory load has't been completed");

		//	Port_inout_lsu data = readData(bankId, rowId);

		//	if (cnt[bankId] == 0 || checkBankEmpty(bankId))
		//	{
		//		lseReadBankFinish[bankId] = 1;
		//		rdPtrReset(bankId);
		//		wrPtrReset(bankId);
		//	}
		//	else
		//		lseReadBankFinish[bankId] = 0;

		//	return data;
		//}
		
		
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

			//// not write lastData to the SPM
			//if (data.lastData != 1)
			//{
			//	writeData(bankId, rowId, data);
			//	//wrPtrUpdate(bankId);
			//}
		
			//// if this is the last data of current context or bank is full, bank write operation is over
			//if (data.lastData == 1 || checkBankFull(bankId))
			//{
			//	lseWriteBankFinish[bankId] = 1;
			//	//wrPtrReset(bankId);
			//	// don't reset rdPtr, due to read data can be executed before write data finished.
			//}
			//else
			//	lseWriteBankFinish[bankId] = 0;
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


			//for (size_t i = 0; i < bankDepth; ++i)
			//{
			//	Port_inout_lsu data = _spmBuffer[bankId][uint(i)];

			//	if (data.valid)
			//	{
			//		if (data.dataReady != 1 && data.inflight != 1)
			//		{
			//			spmReadBankFinish[bankId] = 0;
			//			break;
			//		}
			//	}
			//	else
			//	{
			//		spmReadBankFinish[bankId] = 1;
			//	}
			//}

			return addr;

			//if (addr.lastData == 1)
			//{
			//	_spmBuffer[bankId][rowId].dataReady = 1;  // don't send last data to the memory, set its dataReady to 1 directly
			//}
			//else
			//{
			//	// record the number of load requests inflight, used for OoO memory access
			//	++cnt[bankId];
			//	if (cnt[bankId] > bankDepth)
			//		throw std::runtime_error("read SPM bank error -> current cnt > bank depth");

			//	return addr;
			//}
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
				memWriteBankFinish[bankId] = 1;
			}
			else
				memWriteBankFinish[bankId] = 0;
		}


	private:
		/*vector<Port_inout_lsu> spmInput;
		vector<Port_inout_lsu> spmOutput;*/
		/*vector<uint> rdPtr;
		vector<uint> wrPtr;*/
		//vector<uint> basePtr;  // write/read data to SPM base on this ptr, which bank is read empty, reset its basePtr;
		vector<int> cnt;
		vector<bool> bankFull;
		vector<bool> bankEmpty;
		vector<bool> memReadBankFinish;
		vector<bool> memWriteBankFinish;
		vector<bool> lseReadBankFinish;
		vector<bool> lseWriteBankFinish;
		vector<bool> bankInLoad;  // if a bank is in the sending addr. to memory status, set the flag to 1; If all the addr. has been sent to the memory, reset it to 0
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
		

		// interface for "class Scheduler"
		void setLseReadBankFinish(uint bankId)
		{
			_spmBuffer.setLseReadBankFinish(bankId);
		}

		void setMemReadBankFinish(uint bankId)
		{
			_spmBuffer.setMemReadBankFinish(bankId);
		}

		void setLseWriteBankFinish(uint bankId)
		{
			_spmBuffer.setLseWriteBankFinish(bankId);
		}

		void setMemWriteBankFinish(uint bankId)
		{
			_spmBuffer.setMemWriteBankFinish(bankId);
		}

		void resetLseReadBankFinish(uint bankId)
		{
			_spmBuffer.resetLseReadBankFinish(bankId);
		}

		void resetMemReadBankFinish(uint bankId)
		{
			_spmBuffer.resetMemReadBankFinish(bankId);
		}

		void resetLseWriteBankFinish(uint bankId)
		{
			_spmBuffer.resetLseWriteBankFinish(bankId);
		}

		void resetMemWriteBankFinish(uint bankId)
		{
			_spmBuffer.resetMemWriteBankFinish(bankId);
		}

		// interface function for "class Scheduler" to get SPM bank write/read status
		bool getLseReadBankFinish(uint bankId)
		{
			return _spmBuffer.getLseReadBankFinish(bankId);
		}

		bool getMemReadBankFinish(uint bankId)
		{
			return _spmBuffer.getMemReadBankFinish(bankId);
		}

		bool getLseWriteBankFinish(uint bankId)
		{
			return _spmBuffer.getLseWriteBankFinish(bankId);
		}

		bool getMemWriteBankFinish(uint bankId)
		{
			return _spmBuffer.getMemWriteBankFinish(bankId);
		}


		// update SPM in each cycle
		void spmUpdate()
		{
			for (auto i : contextQueue)  // traverse each context
			{
				for (auto lseContext : _lseConfig[i])  // traverse each LSE's configuration in the current context
				{
					lse2Spm(lseContext);  // send temp data or addr from LSE to SPM
					//spm2Mem(lseContext);  // send addr from SPM to Mem
					//mem2Spm(lseContext);  // send load data from Mem to SPM
					//spm2Lse(lseContext);  // send temp data or load data from SPM to LSE
				}

				spm2Lse();  // send temp data or load data from SPM to LSE
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
					for (size_t rowId = 0; rowId < bankDepth; ++rowId)
					{
						Port_inout_lsu rowData = _spmBuffer.getSpmData(bankId, rowId);

						if (rowData.valid != 1 && rowData.occupy != 1)  // find a free row in bank; occupy flag is used in branch
						{
							_spmBuffer.resetLseWriteBankFinish(bankId);  // reset bank finish

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

										_spmBuffer.setBankInLoad(bankId, 1);
									}

									if (context._lseMode == LSMode::store_data && context._memAccessMode == MemAccessMode::temp)
									{
										data.dataReady = 1;  // for temp data, dataReady set to 1
										_spmBuffer.writeLse2Spm(bankId, rowId, data);
									}
								}
								else
								{
									_spmBuffer.setLseWriteBankFinish(bankId);
									//_spmBuffer.wrPtrReset(bankId);  // don't reset rdPtr 
								}
							}
							else
							{
								data.valid = 0;
								data.occupy = 1;
								_spmBuffer.writeLse2Spm(bankId, rowId, data); // write an occupy data to SPM, to occupy a buffer's row; Due to the data matching is according to the row; 
							}

							// check LseWriteBankFinish again after an valid write
							//***  Note: if it is ensured that the last_data comes in the last, the code below can be removed  ***//
							for (size_t rowId = 0; rowId < bankDepth; ++rowId)
							{
								if (rowData.valid != 1 && rowData.occupy != 1)
									break;
								else
									_spmBuffer.setLseWriteBankFinish(bankId);
							}
							//*********//

							break;
						}
						else
						{
							_spmBuffer.setLseWriteBankFinish(bankId); // don't find a free row, indicate the bank is full, writeBank finish 
						}
					}

					//if ((context._lseMode == LSMode::load && context._memAccessMode == MemAccessMode::load && context._daeMode == DaeMode::send_addr) && 
					//	_spmBuffer.getLseWriteBankFinish(bankId))
					//{
					//	_spmBuffer.setBankInLoad(bankId, 1);  // set bank is in load mode
					//}
				}
			}
		}

		void spm2Mem()
		{
			for (size_t bankId = 0; bankId < bankNum; ++bankId)
			{
				if (_spmBuffer.getBankInLoad[bankId]) // when current bank is in load status
				{
					if (!memory_ACK)  // when the datapath to memory not stall, due to the memory is only 8 banks
					{
						for (size_t i = 0; i < bankDepth; ++i)
						{
							Port_inout_lsu data = _spmBuffer.getSpmData(bankId, uint(i));

							if (data.valid && data.dataReady != 1 && data.inflight != 1)
							{
								data = _spmBuffer.readSpm2Mem(bankId, uint(i));
								sendData2Mem(data);  // send this addr. to memory
								break;
							}

							if (i == bankDepth - 1)  // execute to the last loop, indicates no addr has been read out
							{
								if (_spmBuffer.getLseWriteBankFinish(bankId))
								{
									_spmBuffer.setMemReadBankFinish[bankId];
									_spmBuffer.setBankInLoad(bankId, 0);  // indicate current bank has been loaded completely
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
			if (_spmBuffer.getLseWriteBankFinish(bankId))  // when all the addr. have been sent to SPM from LSE
			{
				for (size_t i = 0; i < bankDepth; ++i)
				{
					Port_inout_lsu data = _spmBuffer.getSpmData(bankId, uint(i));

					if (data.valid && data.dataReady != 1)
					{
						_spmBuffer.resetMemWriteBankFinish(bankId);
						break;
					}
					else
					{
						_spmBuffer.setMemWriteBankFinish(bankId);
					}
				}
			}
		}

		void spm2Lse()
		{
			// send lastData when bankReadFinish!!!
			// data match

			BranchMode lseBranMode = _lseConfig[i][0]._branchMode;
			// check branchMode consistance
			for (auto lseContext : _lseConfig[i])
			{
				if (lseContext._branchMode != lseBranMode)
					throw std::runtime_error("LSE branchMode configure error -> All the LSE belong to the same context must have the same branchMode");
			}

			for (size_t rowId = 0; rowId < bankDepth; ++rowId)
			{
				bool dataMatch = 1;
				bool bankLoadFinish = 1;  // indicate all the bank in load mode is written completely by memory

				for (auto lseContext : _lseConfig[i])
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

					bankLoadFinish = bankLoadFinish & _spmBuffer.getLseWriteBankFinish[bankId];
				}

				if (dataMatch) // if match successfully
				{
					for (auto lseContext : _lseConfig[i])
					{
						uint bankId = lseContext.lseVirtualTag;

						if (lseContext._lseMode == LSMode::load)
						{
							sendData2Lse(_spmBuffer.readSpm2Lse(bankId, rowId));
						}
					}

					break;
				}

				if (rowId == bankDepth - 1 && bankLoadFinish)  // there is no data match
				{
					for (auto lseContext : _lseConfig[i])
					{
						if (lseContext._lseMode == LSMode::load)
						{
							_spmBuffer.setLseReadBankFinish[lseContext.lseVirtualTag];

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


	//class Scheduler
	//{
	//public:
	//	Scheduler(Context lseConfig) : _lseConfig(lseConfig)
	//	{
	//	}

	//private:
	//	Context _lseConfig;  
	//};


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