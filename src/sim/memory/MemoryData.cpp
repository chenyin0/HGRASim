#include "MemoryData.h"

using namespace Simulator::Array;

MemoryData::MemoryData(unsigned int depth)
{
	this->depth_ = depth;
	mem.resize(depth);
	mem_state.resize(depth, false);
}

MemoryData::MemoryData()
{
	const auto para = Simulator::Preprocess::Para::getInstance()->getArrayPara();
	depth_ = para.max_memory_depth;
	mem.resize(100000);
	mem_state.resize(depth_, false);
}

int MemoryData::read(const uint& addr)
{
	if (addr < depth_)
	{
		if (isInitialized(addr))
			return mem[addr];
		else {
			DEBUG_ASSERT(false);
			std::cout << " "<<addr;
			return UINT_MAX;
		}
	}
	else
	{
	//	std::cout << " " << addr<<"depth";
		DEBUG_ASSERT(false);
		return UINT_MAX;
	}
}

void MemoryData::write(const uint& addr, const int& data)
{
	if (addr < depth_)
	{
		mem[addr] = data;
		setInitialized(addr);
	}
	else
		DEBUG_ASSERT(false);
}

void MemoryData::readFromFile(std::ifstream& infile)
{
	TxtRead::readFile2UnifiedVector<int>(mem, infile);
	//for (uint i = 0; i < mem.size(); i++)
		//mem_state[i] = true;
}

void MemoryData::writeToFile(std::ofstream& outfile)
{
	for (vector<bool>::size_type addr__ = 0; addr__ < mem_state.size(); addr__++)
	{
		if (mem_state[addr__])
		{
			int temp = mem[addr__];
			outfile << temp << "  @" << addr__ << std::endl;
		}
	}	
}

