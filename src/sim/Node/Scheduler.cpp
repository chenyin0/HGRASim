#include "Scheduler.h"
#include <iomanip>
using namespace Simulator::Array;
Scheduler::Scheduler(vector<uint> _cdq, Spm* _spm) : cdq(_cdq), spm(_spm)
{
	schedulerInit();
}

void Scheduler::schedulerInit()
{
	cdqPtr = 0;
}
bool Scheduler::contextIsFinish(uint contextId)
{
	bool finish = 1;
	Context& lseConfig = spm->_lseConfig;

	for (auto config : lseConfig[contextId])
	{
		uint bankId = config.lseVirtualTag;
		bool finishTemp = 0;

		// LSE to SPM
		if ((config._lseMode == LSMode::load && config._memAccessMode == MemAccessMode::load && config._daeMode == DaeMode::send_addr) ||
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
			(config._lseMode == LSMode::load && config._memAccessMode == MemAccessMode::load && config._daeMode == DaeMode::get_data))
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

bool Scheduler::nextContextIsReady(uint contextId)
{
	bool ready = 1;
	Context& lseConfig = spm->_lseConfig;

	for (auto config : lseConfig[contextId])
	{
		uint bankId = config.lseVirtualTag;
		bool readyTemp = 0;

		// SPM to LSE (read SPM)
		if ((config._lseMode == LSMode::load && config._memAccessMode == MemAccessMode::load && config._daeMode == DaeMode::get_data) ||
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

void Scheduler::updateCdqPtr()
{
	(++cdqPtr) % cdq.size();
}
void Scheduler::schedulerUpdate()
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