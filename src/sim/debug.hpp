#pragma once

#include <iomanip>
#include "../define/define.hpp"
#include "../util/address.hpp"
#include "./Buffer/buffer.h"

namespace Simulator::Array
{
	/**
	 * 重新设计Debug，不再是单例类
	 * 修正命名规范
	 * 修正构造函数，去掉了默认构造函数
	 * 不要在默认构造函数里面采用默认地址的设计
	 * 默认地址不属于这个类应当负责的“功能”
	 * debug level不要用数字，对代码的使用者不直观，改成了枚举类
	 *
	 * 行 2019.10.6
	 * 
	 */
	class Debug : public AddressLoader<3>
	{
	public:
		enum class DebugLevel
		{
			// TODO 补充debug level - 行 2019.10.6
			base
		};

	private:
		enum Addr : uint
		{
			RegFileAddr = 0,
			PortFileAddr = 1,
			LseRegAddr = 2
		};

	public:
		Debug(DebugLevel level, const string& reg_file_, const string& port_file_, const string& lse_reg_file_)
			: debug_level(level)
			, print_screen_begin(0)
			, print_screen_end(100)
			, print_screen_interval(1)
			, print_file_begin(0)
			, print_file_end(100)
			, print_file_interval(1)
		{
			loadAddress(RegFileAddr, reg_file_);
			loadAddress(PortFileAddr, port_file_);
			loadAddress(LseRegAddr, lse_reg_file_);

			_reg_file.open(getAddress(RegFileAddr));
			_port_file.open(getAddress(PortFileAddr));
			_lse_reg_file.open(getAddress(LseRegAddr));
		}

		~Debug()
		{
			_reg_file.close();
			_port_file.close();
			_lse_reg_file.close();
		}

	public:
		DebugLevel debug_level;

		//screen set
		uint print_screen_begin;
		uint print_screen_end;
		uint print_screen_interval;

		//file set
		uint print_file_begin;
		uint print_file_end;
		uint print_file_interval;

		template <typename T>
		void onePrint(const T& wire, string name)
		{
		}

		template <>
		void onePrint<Port_inout>(const Port_inout& wire, string name)
		{
			_port_file.width(15);
			_port_file << name << ".valid";
			_port_file.width(15);
			_port_file << name << ".control";
			_port_file.width(15);
			_port_file << name << ".d32";
			_port_file.width(15);
			_port_file << name << ".dbool";
			_port_file << std::endl;
			_port_file << std::setw(15) << wire.valid;
			_port_file << std::setw(20) << wire.condition << " " << wire.last;
			_port_file << std::setw(20) << wire.value_data;
			_port_file << std::setw(20) << wire.value_bool;
			_port_file << std::endl;
		}

		template <>
		void onePrint<Port_inout_lsu>(const Port_inout_lsu& wire, string name)
		{
			_port_file.width(20);
			_port_file << name << ".valid";
			_port_file.width(20);
			_port_file << name << ".rdwr";
			_port_file.width(20);
			_port_file << name << ".tag";
			_port_file.width(20);
			_port_file << name << ".dae";
			_port_file.width(20);
			_port_file << name << ".value_addr";
			_port_file.width(20);
			_port_file << name << ".value_data";
			_port_file << std::endl;
			_port_file << std::setw(15) << wire.valid;
			_port_file << std::setw(25) << wire.rdwr;
			_port_file << std::setw(25) << wire.tag;
			_port_file << std::setw(25) << wire.dae;
			_port_file << std::setw(25) << wire.value_addr;
			_port_file << std::setw(25) << wire.value_data;
			_port_file << std::endl;
		}
		//	template<>
		//	void onePrint<Port_inout>(const Port_inout& wire, string name) {
		//for (uint i = 0; i < reg.size(); i++)
		//{
		//	file.width(15);
		//	file << "reg_input" << i << ".valid";
		//	file.width(15);
		//	file << "reg_input" << i << ".control";
		//	file.width(15);
		//	file << "reg_input" << i << ".d32";
		//	file.width(15);
		//	file << "reg_input" << i << ".dbool";
		//	file << std::endl;
		//	file << std::setw(15) << reg_input[i].valid;
		//	file << std::setw(20) << reg_input[i].condition << " " << reg_input[i].last;
		//	file << std::setw(20) << reg_input[i].value_data;
		//	file << std::setw(20) << reg_input[i].value_bool;
		//	file << std::endl;
		//}
		//	}
		template <typename T>
		void vecPrint(const vector<T>& wirevec, string name)
		{
		}

		template <>
		void vecPrint<Port_inout>(const vector<Port_inout>& wirevec, string name)
		{
			for (uint i = 0; i < wirevec.size(); i++)
			{
				_port_file.width(15);
				_port_file << name << i << ".valid";
				_port_file.width(15);
				_port_file << name << i << ".control";
				_port_file.width(15);
				_port_file << name << i << ".d32";
				_port_file.width(15);
				_port_file << name << i << ".dbool";
				_port_file << std::endl;
				_port_file << std::setw(15) << wirevec[i].valid;
				_port_file << std::setw(20) << wirevec[i].condition << " " << wirevec[i].last;
				_port_file << std::setw(20) << wirevec[i].value_data;
				_port_file << std::setw(20) << wirevec[i].value_bool;
				_port_file << std::endl;
			}
		}

		template <>
		void vecPrint<Bool>(const vector<Bool>& bp, string name)
		{
			for (uint i = 0; i < bp.size(); i++)
			{
				_port_file.width(15);
				_port_file << name << i;
			}
			_port_file << std::endl;
			for (auto& i : bp)
				_port_file << std::setw(15) << i;
			_port_file << std::endl;
		}

		template <>
		void vecPrint<uint>(const vector<uint>& bp, string name)
		{
			/*			for (uint i = 0; i < bp.size(); i++)
						{
							_port_file.width(15);
							_port_file << name << i;
						}*/
			//			_port_file << std::endl;
			for (auto& i : bp)
				_lse_reg_file << std::setw(2) << i;
			_lse_reg_file << std::endl;
		}

		template <>
		void vecPrint<Bufferd>(const vector<Bufferd>& wirevec, string name)
		{
			for (uint i = 0; i < wirevec.size(); i++)
			{
				_reg_file.width(15);
				_reg_file << name << i << ".valid";
				_reg_file.width(15);
				_reg_file << name << i << ".control";
				_reg_file.width(15);
				_reg_file << name << i << ".d32";
				_reg_file.width(15);
				_reg_file << name << i << ".dbool";
				_reg_file << std::endl;
				_reg_file << std::setw(15) << wirevec[i].valid;
				_reg_file << std::setw(20) << wirevec[i].valued;
				_reg_file << std::setw(20) << wirevec[i].condition << " " << wirevec[i].last;
				_reg_file << std::setw(20) << wirevec[i].valueb;
				_reg_file << std::endl;
			}
		}


		//void bpPrint(const vector<Bool>& bp, string name) {
		//	for (uint i = 0; i < bp.size(); i++)
		//	{
		//		_port_file.width(15);
		//		_port_file << name << i;
		//	}
		//	_port_file << std::endl;
		//	for (auto& i : bp)
		//		_port_file << std::setw(15) << i;
		//	_port_file << std::endl;
		//}
		void linePrint(string name)
		{
			_port_file << name;
			_port_file << std::endl;
		}
		
		std::ofstream& getRegFile()
		{
			return _reg_file;
		}

		std::ofstream& getPortFile()
		{
			return _port_file;
		}

		std::ofstream& getLseFile()
		{
			return _lse_reg_file;
		}

	private:
		std::ofstream _reg_file;
		std::ofstream _port_file;
		std::ofstream _lse_reg_file;
	};
}
