#pragma once
#include "../define/define.hpp"
#include "../sim/memory/MemoryData.h"
using namespace std;

namespace Simulator
{
	class TxtRead
	{
	public:
		template<typename Type>
		static void readFile2UnifiedVector(vector<Type>&vec, ifstream& infile)
		{
			uint k = 0;
			string temp_str;
			while (getline(infile, temp_str))
			{
				Type temp_data = std::stoi(temp_str);
				vec[k]=temp_data;
				Simulator::Array::MemoryData::getInstance()->setInitialized(k);
				k++;
			}
		}
	};
}