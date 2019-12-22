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
			rdPtr.resize(bankNum);
			wrPtr.resize(bankNum);
			bankFull.resize(bankNum);
			bankEmpty.resize(bankNum);
			bankReadFinish.resize(bankNum);
			bankWriteFinish.resize(bankNum);

			// initial bank id
			bankId.resize(bankNum);
			for (size_t i = 0; i < bankId.size(); ++i)
			{
				bankId[i] = i;
			}

			// initial row id
			rowId.resize(bankDepth);
			for (size_t i = 0; i < rowId.size(); ++i)
			{
				rowId[i] = i;
			}

			// initial SPM buffer's bank number
			_spmBuffer.resize(bankNum);

			// initial SPM buffer depth
			for (auto& i : _spmBuffer)
			{
				i.resize(bankDepth);
			}

			// initial SPM status
			for (size_t i = 0; i < bankId.size(); ++i)
			{
				bankFull[i] = 0;
				bankEmpty[i] = 1;
				rdPtr[i] = 0;
				wrPtr[i] = 0;
				bankReadFinish[i] = 0;
				bankWriteFinish[i] = 0;
			}

		}

		// reset SPM stauts
		void spmReset()
		{
			for (size_t i = 0; i < bankNum; ++i)
			{
				rdPtr[i] = 0;
				wrPtr[i] = 0;
				bankFull[i] = 0;
				bankEmpty[i] = 0;
			}
		}

		// reset bank read finish signal before a new context with SPM read
		void resetBankReadFinish(uint bankId)
		{
			bankReadFinish[bankId] = 0;
		}

		// reset bank write finish signal before a new context with SPM write
		void resetBankWriteFinish(uint bankId)
		{
			bankWriteFinish[bankId] = 0;
		}

		// get SPM rdPtr
		uint getRdPtr(const uint bankId)
		{
			return rdPtr[bankId];
		}

		// get SPM wrPtr
		uint getWrPtr(const uint bankId)
		{
			return wrPtr[bankId];
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

		void checkBankFull(const uint bankId)
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
		}

		void checkBankEmpty(const uint bankId)
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

		Port_inout_lsu readData(const uint bankId, const uint rowId)
		{	
			if (_spmBuffer[bankId][rowId].valid == 0)
				throw std::runtime_error("read SPM address error -> try to read an invalid data");
			else 
			{
				_spmBuffer[bankId][rowId].dataReady = 0; // clear data ready both in SPM & current data
				Port_inout_lsu data = _spmBuffer[bankId][rowId];
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
			}
		}

		void wrPtrUpdate(uint bankId)
		{	
			++wrPtr[bankId];

			if (wrPtr[bankId] == bankDepth)
				wrPtr[bankId] = 0;

			if (wrPtr[bankId] == rdPtr[bankId])
				bankFull[bankId] = 1;  // this bank is written full
		}

		void rdPtrUpdate(uint bankId)
		{
			++rdPtr[bankId];

			if (rdPtr[bankId] == bankDepth)
				rdPtr[bankId] = 0;

			if (rdPtr[bankId] == wrPtr[bankId])
				bankEmpty[bankId] = 1; // this bank is read empty
		}

		// reset wrPtr & rdPtr when read SPM context finish
		// due to if current context read SPM, it must read the bank to empty before switch a new context 
		void resetPtr(uint bankId)
		{
			rdPtr[bankId] = 0;
			wrPtr[bankId] = 0;
			bankEmpty[bankId] = 1;
			bankFull[bankId] = 0;
		}

		// read data from SPM to LSE
		// use for reading temp data not in branch, read as fifo-style
		Port_inout_lsu readSpm2Lse_tempNoCond(const uint bankId)
		{
			Port_inout_lsu data;

			if (bankEmpty[bankId] == 1)
				throw std::runtime_error("try to read an empty SPM bank");

			data = readData(bankId, rdPtr[bankId]);
			rdPtrUpdate(bankId);

			// if this is the last data of current context, bank read operation is over
			if (data.lastData == 1)
			{
				bankReadFinish[bankId] = 1;
			}
			else if (bankEmpty[bankId])  // if bank has been read empty, bank read operation is over
			{
				bankReadFinish[bankId] = 1;
			}
			else
				bankReadFinish[bankId] = 0;

			return data;
		}

		// read data from SPM to LSE
		// use for reading temp data in branch, read according to the condition status
		Port_inout_lsu readSpm2Lse_tempWithCond(const uint bankId, bool cond)
		{
			Port_inout_lsu data;

			if (bankEmpty[bankId] == 1)
				throw std::runtime_error("try to read an empty SPM bank");

			while (~checkBankEmptyWithCond(bankId, cond))  // when data with corresponding condition hasn't been read empty
			{	
				if (_spmBuffer[bankId][rdPtr[bankId]].cond == cond)
				{
					data = readData(bankId, rdPtr[bankId]);
					rdPtrUpdate(bankId);

					if (data.lastData == 1)
					{
						bankReadFinish[bankId] = 1;
					}

					return data;
				}
			}

			bankReadFinish[bankId] = 1;
		}

		// token match operate in "class SPM", due to "class SpmBuffer" doesn't know which banks need to match
		Port_inout_lsu readSpm2Lse_memLoad(const uint bankId, const uint rowId)
		{
			Port_inout_lsu data = readData(bankId, rowId);

			checkBankEmpty(bankId);
			if (bankEmpty[bankId])
			{
				bankReadFinish[bankId] = 1;
			}

			return data;
		}

		// write data from LSE to SPM; 
		// use for 1) store temp data in branch or non-branch; 2) store address in stream/irregular-memory-access mode; 
		void writeLse2Spm(const uint bankId, const Port_inout_lsu data)
		{
			if(bankFull[bankId] == 1)
				throw std::runtime_error("try to write a full SPM bank");

			writeData(bankId, wrPtr[bankId], data);
			wrPtrUpdate(bankId);

			// if this is the last data of current context, bank write operation is over
			if (data.lastData == 1)
			{
				bankWriteFinish[bankId] = 1;
			}
			else if (bankFull[bankId])  // if bank has been written full, bank write operation is over
			{
				bankWriteFinish[bankId] = 1;
			}
			else
				bankWriteFinish[bankId] = 0;
		}

		// read addr. from SPM to Memory
		// use for send addr to memory when load data in DAE
		Port_inout_lsu readSpm2Mem(const uint bankId, const uint rowId)
		{
			return readAddr(bankId, rowId);
		}

		// write memory ack back to SPM
		// use for 1) load data in stream/irregular-memory-access mode;
		void writeMem2Spm(const uint bankId, const uint rowId, const Port_inout_lsu data)
		{
			writeMemAck(bankId, rowId, data);
		}


	private:
		/*vector<Port_inout_lsu> spmInput;
		vector<Port_inout_lsu> spmOutput;*/
		vector<uint> rdPtr;
		vector<uint> wrPtr;
		vector<bool> bankFull;
		vector<bool> bankEmpty;
		vector<bool> bankReadFinish;
		vector<bool> bankWriteFinish;
		uint bankNum;
		uint bankDepth;
		vector<vector<Port_inout_lsu>> _spmBuffer;  // vector_1<vector_2>; vector_1 = bank, vector_2 = row;
		vector<uint> bankId;
		vector<uint> rowId;
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
		
		// update SPM in each cycle
		void spmUpdate()
		{
			for (auto i : contextQueue)
			{
				
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


	class Scheduler
	{
	public:
		Scheduler(Context lseConfig) : _lseConfig(lseConfig)
		{
		}

	private:
		Context _lseConfig;  
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
	};

}