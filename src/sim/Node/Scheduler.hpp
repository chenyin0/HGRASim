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

#include "./SPM.hpp"

namespace Simulator::Array
{
	class Scheduler
	{
	public:
		Scheduler(Context _lseConfig, vector<uint> _cdq, Spm* _spm) : lseConfig(_lseConfig), cdq(_cdq), spm(_spm)
		{
			schedulerInit();
		}

		void schedulerInit()
		{
			lseFinish.resize(lseConfig.size());

			for (size_t i = 0; i < lseFinish.size(); ++i)
			{
				lseFinish[i].resize(lseConfig[i].size());

				for (auto &lse : lseFinish[i])
				{
					lse = 0;
				}
			}

		}

	private:
		bool contextIsFinish(uint contextId)
		{
			bool finish = 1;

			for (size_t i = 0; i < lseConfig[contextId].size(); ++i)
			{
				LseConfig config = lseConfig[contextId][i];
				uint bankId = config.lseVirtualTag;

				if ((config._lseMode == LSMode::load && config._memAccessMode == MemAccessMode::load && config._daeMode == DaeMode::send_addr) ||
					(config._lseMode == LSMode::store_data && config._memAccessMode == MemAccessMode::temp))
				{
					if (spm->getLseWriteBankFinish(bankId))
					{
						lseFinish[contextId][i] = 1;
					}
					else
						lseFinish[contextId][i] = 0;
				}
				else if()
				else
				{
					lseFinish[contextId][i] = 1;
				}

				finish = finish & lseFinish[contextId][i];
			}

			return finish;
		}

		bool contextIsReady(uint contextId)
		{

		}

	public:
		void schedulerUpdate()
		{

		}

	private:
		Context lseConfig;
		vector<vector<uint>> lseFinish;  // set 1, signify this LSE executes finishes in the current context
		vector<uint> cdq;  // context dependency queue, current version is static schedule
		Spm* spm;
	};
}