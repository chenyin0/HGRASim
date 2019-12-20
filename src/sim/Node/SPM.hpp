#pragma once

#include "iostream"                        
#include "stdlib.h"                       
#include "math.h"                         //pay attention to ACK
#include <vector>
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

		// reset bank read/write finish signal when switch context
		void spmContextSwitchReset()
		{
			for (size_t i = 0; i < bankNum; ++i)
			{
				bankReadFinish[i] = 0;
				bankWriteFinish[i] = 0;
			}
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

		// read data from SPM to LSE
		// use for reading temp data not in branch, read as fifo-style
		Port_inout_lsu readSpm2Lse_tempWithNonCond(const uint bankId)
		{
			if (bankEmpty[bankId] == 1)
				throw std::runtime_error("try to read an empty SPM bank");

			Port_inout_lsu data = _spmBuffer[bankId][rdPtr[bankId]];
			++rdPtr[bankId];

			if (rdPtr[bankId] == bankDepth)
				rdPtr[bankId] = 0;

			if (rdPtr[bankId] == wrPtr[bankId])
				bankEmpty[bankId] = 1;

			return data;
		}

		// use for loading irregular-memory-access data, in an OoO way 
		Port_inout_lsu readSPM_OOO()
		{
			if (bankEmpty[bankId] == 1)
				throw std::runtime_error("try to read an empty SPM bank");
			return _spmBuffer[bankId][rowId];
		}

		// write data from LSE to SPM; 
		// use for 1) store temp data in branch or non-branch; 2) store address in stream/irregular-memory-access mode; 
		void writeLse2Spm(const uint bankId, const Port_inout_lsu data)
		{
			if(bankFull[bankId] == 1)
				throw std::runtime_error("try to write a full SPM bank");

			if(_spmBuffer[bankId][wrPtr[bankId]].valid == 1)
				throw std::runtime_error("write SPM address conflict -> there is already a valid data");
			else
			{
				_spmBuffer[bankId][wrPtr[bankId]] = data;
				++wrPtr[bankId];
			}

			if (wrPtr[bankId] == bankDepth)
				wrPtr[bankId] = 0;

			if (wrPtr[bankId] == rdPtr[bankId])
				bankFull[bankId] = 1;  // this bank is written full

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

		// write memory ack back to SPM
		// use for 1) load data in stream/irregular-memory-access mode;
		void writeMem2Spm(const uint bankId, const uint rowId, const Port_inout_lsu data)
		{
			if (_spmBuffer[bankId][rowId].valid == 1)
				throw std::runtime_error("write SPM address conflict -> there is already a valid data");
			else
			{
				_spmBuffer[bankId][rowId] = data;
			}

			// only for stream data load, for dynamic schedule
			checkBankFull(data.bankId);

			if (data.lastData == 1 && bankFull[data.bankId] == 1)
			{
				bankWriteFinish[data.bankId];
			}
			//************//
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


	//class spmStatusTable
	//{
	//public:
	//	spmStatusTable()
	//	{

	//	}

	//private:

	//};


	class Spm
	{
	public:
		Spm()
		{
			bankNum = Preprocess::Para::getInstance()->getArrayPara().lse_virtual_num;  // SPM bank number is equal to LSE virtual number
			bankDepth = Preprocess::Para::getInstance()->getArrayPara().SPM_depth;  // initial SPM buffer depth

			spmInput.resize(bankNum);
			spmOutput.resize(bankNum);
		}

	private:
		uint bankNum;
		uint bankDepth;
		SpmBuffer _spmBuffer = SpmBuffer(bankNum, bankDepth);
		vector<Port_inout_lsu> spmInput;
		vector<Port_inout_lsu> spmOutput;
	};

}