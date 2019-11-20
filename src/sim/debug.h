#pragma once
#include <iomanip>
#include "../define/define.hpp"
#include "./Buffer/buffer.h"
//using namespace Simulator::Array;
namespace Simulator::Array
{
	class Debug :public Singleton<Debug>
	{
	private:
		std::ofstream regfile;
		std::ofstream portfile;
		std::ofstream lseRegfile;
		std::ofstream dataFlowfile;
	public:
		friend Singleton<Debug>;
		uint debug_level;

		//screen set
		uint print_screen_begin;
		uint print_screen_end;
		uint print_screen_interval;

		//file set
		uint print_file_begin;
		uint print_file_end;
		uint print_file_interval;
		template<typename T>
		void onePrint(const T& wire, string name) { ; }
		template<>
		void onePrint<Port_inout>(const Port_inout& wire, string name) {
			portfile.width(15);
			portfile << name;
			//portfile.width(15);
			//portfile << name << ".control";
			//portfile.width(15);
			//portfile << name << ".d32";
			//portfile.width(15);
			//portfile << name << ".dbool";
		
			portfile << "  "  << wire.valid;
			portfile << "  " << wire.condition << wire.last;
			portfile << "  " << wire.value_data;
			portfile << "  " << wire.value_bool;
			portfile << std::endl;
		}
		template<>
		void onePrint<Port_inout_lsu>(const Port_inout_lsu& wire, string name) {
//			portfile.width(15);
			portfile << name ;
			//portfile.width(20);
			//portfile << name << ".rdwr";
			//portfile.width(20);
			//portfile << name << ".tag";
			//portfile.width(20);
			//portfile << name << ".dae";
			//portfile.width(20);
			//portfile << name << ".value_addr";
			//portfile.width(20);
			//portfile << name << ".value_data";
			portfile << "  " << wire.valid;
			portfile << "  " << wire.rdwr;
			portfile << "  " << wire.tag;
			portfile << "  " << wire.dae;
			portfile << "  " << wire.value_addr;
			portfile << "  " << wire.value_data;
			portfile << std::endl;
		}
		template<>
		void onePrint<Bool>(const Bool& wire, string name) {
			portfile.width(15);
			portfile << name << wire;
			portfile << std::endl;
		}
		template<typename T>
		void vecPrint(const vector<T>& wirevec, string name) { ; }
		template<>
		void vecPrint<Port_inout>(const vector<Port_inout>& wirevec, string name) {
			for (uint i = 0; i < wirevec.size(); i++)
			{
				portfile.width(14);
				portfile << name << i;
				//portfile.width(15);
				//portfile << name << i << ".control";
				//portfile.width(15);
				//portfile << name << i << ".d32";
				//portfile.width(15);
				//portfile << name << i << ".dbool";
				portfile << "  " << wirevec[i].valid;
				portfile << "  " << wirevec[i].condition << wirevec[i].last;
				portfile << "  " << wirevec[i].value_data;
				portfile << "  " << wirevec[i].value_bool;
				portfile << std::endl;
			}
		}

		template<>
		void vecPrint<Bool>(const vector<Bool>& bp, string name) {
			for (uint i = 0; i < bp.size(); i++)
			{
				portfile.width(15);
				portfile << name << i;
			}
			portfile << std::endl;
			for (auto& i : bp)
				portfile << std::setw(15) << i;
			portfile << std::endl;
		}
		template<>
		void vecPrint<uint>(const vector<uint>& bp, string name) {
/*			for (uint i = 0; i < bp.size(); i++)
			{
				portfile.width(15);
				portfile << name << i;
			}*/
//			portfile << std::endl;
			for (auto& i : bp)
				lseRegfile << std::setw(2) << i;
			lseRegfile << std::endl;
		}
		template<>
		void vecPrint<Simulator::Array::Bufferd>(const vector<Simulator::Array::Bufferd>& wirevec, string name) {
			for (uint i = 0; i < wirevec.size(); i++)
			{
//				regfile.width(15);
				regfile << name << i;
				regfile << "  " << wirevec[i].valid;
				regfile << "  " << wirevec[i].valued;
				regfile << "  " << wirevec[i].condition  << wirevec[i].last;
				regfile << "  " << wirevec[i].valueb;
				regfile << std::endl;
			}
		}


		//void bpPrint(const vector<Bool>& bp, string name) {
		//	for (uint i = 0; i < bp.size(); i++)
		//	{
		//		portfile.width(15);
		//		portfile << name << i;
		//	}
		//	portfile << std::endl;
		//	for (auto& i : bp)
		//		portfile << std::setw(15) << i;
		//	portfile << std::endl;
		//}
		void linePrint(string name) {
			portfile << name;
			portfile << std::endl;
		}
		Debug():print_screen_begin(0), print_screen_end(15000), print_screen_interval(1),
			print_file_begin(0), print_file_end(1000), print_file_interval(1) {
			const auto& system_para = Preprocess::Para::getInstance()->getArrayPara();
			debug_level = system_para.debug_level;
			regfile.open(".\\resource\\output\\DebugReg.txt"); portfile.open(".\\resource\\output\\DebugWire.txt"); lseRegfile.open(".\\resource\\output\\Debuglseout.txt");dataFlowfile.open(".\\resource\\output\\dataflow.txt");
//			regfile.open("F:\\lab\\reconfig\\HGRA5\\resource\\DebugReg.txt"); portfile.open("F:\\lab\\reconfig\\HGRA5\\resource\\DebugWire.txt");
	//	F:\lab\reconfig\HGRA5\resource
		}

		Debug(uint level, string regf_name, string portf_name):debug_level(level), print_screen_begin(0), print_screen_end(100), print_screen_interval(1),
		print_file_begin(0), print_file_end(100), print_file_interval(1) {
			regfile.open(regf_name); portfile.open(portf_name);
		}

		~Debug() = default;

		std::ofstream& getRegFile() { return regfile; }
		std::ofstream& getPortFile() { return portfile; }
		std::ofstream& getLseFile() { return lseRegfile;}
		std::ofstream& getdataFlowFile() { return dataFlowfile; }
	};
}