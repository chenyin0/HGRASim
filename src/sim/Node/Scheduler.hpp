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
			//lseFinish.resize(lseConfig.size());
			//lseReady.resize(lseConfig.size());

			//for (size_t i = 0; i < lseFinish.size(); ++i)
			//{
			//	lseFinish[i].resize(lseConfig[i].size());

			//	for (size_t j = 0; j < lseFinish[i].size(); ++j)
			//	{
			//		lseFinish[i][j] = 0;
			//	}
			//}

			//for (size_t i = 0; i < lseReady.size(); ++i)
			//{
			//	lseReady[i].resize(lseConfig[i].size());

			//	for (size_t j = 0; j < lseReady[i].size(); ++j)
			//	{
			//		lseReady[i][j] = 0;
			//	}
			//}
		}

	private:
		bool contextIsFinish(uint contextId)
		{
			bool finish = 1;

			for (size_t i = 0; i < lseConfig[contextId].size(); ++i)
			{
				LseConfig config = lseConfig[contextId][i];
				uint bankId = config.lseVirtualTag;

				bool finishTemp = 0;

				// LSE to SPM
				if ((config._lseMode == LSMode::load && config._memAccessMode == MemAccessMode::load && config._daeMode == DaeMode::send_addr) ||
					(config._lseMode == LSMode::store_data && config._memAccessMode == MemAccessMode::temp))
				{
					if (spm->getLseWriteBankFinish(bankId))
					{
						//lseFinish[contextId][i] = 1;
						finishTemp = 1;
					}
					else
					{
						//lseFinish[contextId][i] = 0;
						finishTemp = 0;
					}
				}
				// SPM to LSE (temp data & load data)
				else if ((config._lseMode == LSMode::load && config._memAccessMode == MemAccessMode::temp) ||
						 (config._lseMode == LSMode::load && config._memAccessMode == MemAccessMode::load && config._daeMode == DaeMode::get_data))
				{
					if (spm->getLseReadBankFinish(bankId))
					{
						//lseFinish[contextId][i] = 1;
						finishTemp = 1;
						spm->resetLseWriteBankFinish(bankId);  // clear lseWriteBankFinish after read a bank finish
					}
					else
					{
						finishTemp = 0;
						//lseFinish[contextId][i] = 0;
					}
				}
				else
				{
					finishTemp = 1;
					//lseFinish[contextId][i] = 1;
				}

				//finish = finish & lseFinish[contextId][i];
				finish = finish & finishTemp;
			}

			return finish;
		}

		bool contextIsReady(uint contextId)
		{
			bool ready = 1;

			for (size_t i = 0; i < lseConfig[contextId].size(); ++i)
			{
				LseConfig config = lseConfig[contextId][i];
				uint bankId = config.lseVirtualTag;

				bool readyTemp = 0;

				// LSE to SPM
				if ((config._lseMode == LSMode::load && config._memAccessMode == MemAccessMode::load && config._daeMode == DaeMode::send_addr) ||
					(config._lseMode == LSMode::store_data && config._memAccessMode == MemAccessMode::temp))
				{
					if (!(spm->getLseWriteBankFinish(bankId)))  // current bank's data hasn't been read empty
					{
						readyTemp = 1;
						//lseReady[contextId][i] = 1;
					}
					else
					{
						readyTemp = 0;
						//lseReady[contextId][i] = 0;
					}
				}
				else
				{
					readyTemp = 1;
					//lseReady[contextId][i] = 1;
				}
			}
		}

	public:
		void schedulerUpdate()
		{

		}

	private:
		Context lseConfig;
		//vector<vector<bool>> lseFinish;  // set 1, signify this LSE executes finishes in the current context
		//vector<vector<bool>> lseReady;   // set 1, signify this LSE get ready in the current context to push in the contextQueue
		vector<uint> cdq;  // context dependency queue, current version is static schedule
		Spm* spm;
	};
}