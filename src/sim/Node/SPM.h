#pragma once


#include "../debug.h"
#include "../ClkDomain.h"
#include "Cluster.h"
#include "../../preprocess/preprocess.h"
#include "LSE.h"
#include "../mem_system/Lsu.h"

namespace Simulator::Array
{
	class SpmBuffer;
	class Spm;
	class LseConfig;

	class SpmBuffer
	{
	public:
		vector<queue<bool>> memReadBankFinish;
		vector<queue<bool>> memWriteBankFinish;
		vector<queue<bool>> lseReadBankFinish;
		vector<queue<bool>> lseWriteBankFinish;
		//vector<bool> lseWriteBankFinishHistory;  // record last write status for bank read, due to bank read finish check need bank write finish history
		vector<queue<bool>> bankInLoad;  // if a bank is in the sending addr. to memory status, set the flag to 1; If all the addr. has been sent to the memory, reset it to 0
		SpmBuffer(uint _bankNum, uint _bankDepth);
		void spmInit();
		// reset SPM stauts
		void spmReset();
		// get SPM rdPtrLse
		uint getRdPtrLse(const uint bankId);

		// get SPM wrPtrLse
		uint getWrPtrLse(const uint bankId);

		uint getRdPtrMem(const uint bankId);

		bool updateRdPtrLse(uint rdPtrLse, vector<LseConfig> lseConfig);

		void updateWrPtrLse(const uint bankId);

		void updateRdPtrMem(const uint bankId);

		void resetWrPtrLse(uint bankId);

		void resetRdPtrLse(uint bankId);

		void resetRdPtrMem(uint bankId);


		// get SPM bank full status
		bool isBankFull(const uint bankId);

		// get SPM bank empty status
		bool isBankEmpty(const uint bankId);

		bool checkBankFull(const uint bankId);

		bool checkBankEmpty(const uint bankId);

		bool checkBankEmptyWithCond(const uint bankId, bool condition);


		// get SPM data, provide to "class SPM" 
		Port_inout_lsu getSpmData(const uint bankId, const uint rowId);

		// set data to SPM, provide to "class SPM"
		void setSpmData(const uint bankId, const uint rowId, const Port_inout_lsu data);

		bool queueNotEmpty(queue<bool> q);

		bool queueNotFull(queue<bool> q);


		Port_inout_lsu readData(const uint bankId, const uint rowId);

		Port_inout_lsu readAddr(const uint bankId, const uint rowId);

		void writeData(const uint bankId, const uint rowId, Port_inout_lsu data);

		void writeMemAck(const uint bankId, const uint rowId, Port_inout_lsu data);


		// read data from SPM to LSE
		Port_inout_lsu readSpm2Lse(const uint bankId, const uint rowId);

		// write data from LSE to SPM; 
		// use for 1) store temp data in branch or non-branch; 2) store address in stream/irregular-memory-access mode; 
		void writeLse2Spm(const uint bankId, const uint rowId, const Port_inout_lsu data);

		// read addr. from SPM to Memory
		// use for send addr to memory when load data in DAE
		Port_inout_lsu readSpm2Mem(const uint bankId, const uint rowId);

		// write memory ack back to SPM
		// use for 1) load data in stream/irregular-memory-access mode;
		void writeMem2Spm(const uint bankId, const uint rowId, const Port_inout_lsu data);
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
		//Spm(Context lseConfig) : _lseConfig(lseConfig)
		//{
		//	bankNum = Preprocess::Para::getInstance()->getArrayPara().lse_virtual_num;  // SPM bank number is equal to LSE virtual number
		//	bankDepth = Preprocess::Para::getInstance()->getArrayPara().SPM_depth;  // initial SPM buffer depth

		//	spmInput.resize(bankNum);
		//	spmOutput.resize(bankNum);
		//}
		Context _lseConfig;  // vector1<vector2<LseConfig>>  vector1:each context  vector2:each LSE configured in current context
		ClusterGroup& cluster_group;
		map<uint, Simulator::Array::Loadstore_element*> lse_map;
		std::map<std::pair<NodeType, uint>, uint>& index2order;
		Spm(unordered_map<NodeType, vector<vector<const Simulator::Preprocess::DFGNodeInterface*>>> context_attr_, ClusterGroup& cluster_group_, map<uint, Simulator::Array::Loadstore_element*>& lse_map_, std::map<std::pair<NodeType, uint>, uint>& index2order_);
		// provide for Scheduler to add a new context to the SPM
		void addContext(uint contextId);

		// provide for Scheduler to delete a finished context in the SPM
		void removeContext();

		// interface for Scheduler
		uint getContextQueueHead();

		uint getContextQueueTail();

		bool contextQueueEmpty();


		// update SPM in each cycle
		void spmUpdate();
		Simulator::Array::Loadstore_element* getLse(uint lse_tag);
		void lse2Spm(LseConfig& context);

		void spm2Mem();
		void attachLsu(DRAMSim::Lsu *lsu_);
		void mem2Spm(const Port_inout_lsu data);

		void spm2Lse(uint contextId);
		DRAMSim::Lsu* lsu;
	private:
		vector<Port_inout_lsu> spmInput;
		vector<Port_inout_lsu> spmOutput;
		uint bankNum;
		//bool memory_ACK;
		uint bankDepth;
		SpmBuffer _spmBuffer = SpmBuffer(bankNum, bankDepth);
		std::deque<uint> contextQueue;  // add/delete valid context by Scheduler, SPM may work under several contexts simultaneously
	};


	class LseConfig
	{
	public:
		uint lseTag;  // lse node tag, number in .xml
		uint lseVirtualTag; // indicate this lse connect to which SPM bank
		LSMode _lseMode; // indicate current LSE mode

		MemAccessMode _memAccessMode;  // configure in .xml
		DirectMode _daeMode;  // configure in .xml
		BranchMode _branchMode;  // configure in .xml

		bool contextFinish;
		bool hasSetBankInLoad;

		LseConfig();
		LseConfig(const Simulator::Preprocess::DFGNodeInterface* attr_);
	};

}