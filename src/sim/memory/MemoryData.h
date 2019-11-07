#pragma once
#include "../../define/define.hpp"
#include <iostream>
#include <sstream>
#include "../../util/txt_read.hpp"
#include "../../preprocess/preprocess.h"

namespace Simulator::Array
{
	class MemoryData:public Singleton<MemoryData>
	{
	private:
		vector<int> mem;
		vector<Bool> mem_state;		//是否被初始化

		bool isInitialized(unsigned int addr) { return mem_state[addr]; }
	//	void setInitialized(unsigned int addr) { mem_state[addr] = true; }

	public:
		friend Singleton<MemoryData>;
		MemoryData(unsigned int depth);
		void setInitialized(unsigned int addr) { mem_state[addr] = true; }
		MemoryData();
		~MemoryData() = default;
		uint depth_;		//mem的深度
		int read(const uint& addr);
		void write(const uint& addr, const int& data);
		void readFromFile(std::ifstream& infile);
		void writeToFile(std::ofstream& outfile);
		//int random_int(unsigned int min, unsigned int max, unsigned int seed);
		//void readFromIostream(istream& in);	
	};
}
