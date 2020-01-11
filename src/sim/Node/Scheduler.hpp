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

#include "SPM.hpp"

namespace Simulator::Array
{
	class Scheduler
	{
	public:
		Scheduler(vector<uint> _cdq, Spm* _spm) : cdq(_cdq), spm(_spm)
		{
			schedulerInit();
		}

		void schedulerInit()
		{
			cdqPtr = 0;
		}

	private:
		bool contextIsFinish(uint contextId)
		{
			bool finish = 1;
			Context& lseConfig = spm->_lseConfig;

			for (auto config : lseConfig[contextId])
			{
				uint bankId = config.lseVirtualTag;
				bool finishTemp = 0;

				// LSE to SPM
				if ((config._lseMode == LSMode::load && config._memAccessMode == MemAccessMode::load && config._DirectionMode == DirectionMode::send) ||
					(config._lseMode == LSMode::store_data && config._memAccessMode == MemAccessMode::temp))
				{
					if (config.contextFinish)
					{
						finishTemp = 1;
					}
					else
					{
						finishTemp = 0;
					}
				}
				// SPM to LSE (temp data & load data)
				else if ((config._lseMode == LSMode::load && config._memAccessMode == MemAccessMode::temp) ||
						 (config._lseMode == LSMode::load && config._memAccessMode == MemAccessMode::load && config._DirectionMode == DirectionMode::get))
				{
					if (config.contextFinish)
					{
						finishTemp = 1;
					}
					else
					{
						finishTemp = 0;
					}
				}
				else
				{
					finishTemp = 1;
				}

				finish = finish & finishTemp;
			}

			return finish;
		}

		bool nextContextIsReady(uint contextId)
		{
			bool ready = 1;
			Context& lseConfig = spm->_lseConfig;

			for (auto config : lseConfig[contextId])
			{
				uint bankId = config.lseVirtualTag;
				bool readyTemp = 0;

				// SPM to LSE (read SPM)
				if ((config._lseMode == LSMode::load && config._memAccessMode == MemAccessMode::load && config._DirectionMode == DirectionMode::get) ||
					(config._lseMode == LSMode::load && config._memAccessMode == MemAccessMode::temp))
				{
					if (config.contextFinish)  // current bank's data has been read empty
					{
						readyTemp = 1;
					}
					else
					{
						readyTemp = 0;
					}
				}
				else
				{
					readyTemp = 1;
				}

				ready = ready & readyTemp;
			}

			return ready;
		}

		void updateCdqPtr()
		{
			(++cdqPtr) % cdq.size();
		}

	public:
		void schedulerUpdate()
		{
			// For each context in contextQueue, if all of its LSEs in read SPM mode finish, pop this context from the queue;
			// Because each context will read and write a SPM simutaneously, so the maximum number of context in the context queue is 2
			// When push a new context in the context queue, reset its contextFinish flag!

			if (spm->contextQueueEmpty())
			{
				if (!cdq.empty())
				{
					spm->addContext(cdq.front());
					updateCdqPtr();
				}
			}

			if (nextContextIsReady(spm->getContextQueueHead()))
			{
				spm->addContext(cdq[cdqPtr]);
			}

			if (contextIsFinish(spm->getContextQueueHead()))
			{
				spm->removeContext();
			}
		}

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