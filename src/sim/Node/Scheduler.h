#pragma once

#include "iostream"                        
#include "stdlib.h"                       
#include "math.h"                         //pay attention to ACK
#include <vector>
#include <deque>
#include <map>
#include "SPM.h"

namespace Simulator::Array
{
	class Scheduler
	{
	public:
		Scheduler(vector<uint> _cdq, Spm* _spm);

		void schedulerInit();

	private:
		bool contextIsFinish(uint contextId);

		bool nextContextIsReady(uint contextId);

		void updateCdqPtr();

	public:
		void schedulerUpdate();

	private:
		//Context lseConfig;
		Context lseConfigInSpm;  // get lseConfigStatus in SPM, only for context finish flag
		//vector<vector<bool>> lseFinish;  // set 1, signify this LSE executes finishes in the current context
		//vector<vector<bool>> lseReady;   // set 1, signify this LSE get ready in the current context to push in the contextQueue
		vector<uint> cdq;  // context dependency queue, current version is static schedule
		uint cdqPtr;
		Spm* spm;
	};
}