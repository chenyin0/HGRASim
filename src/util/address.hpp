#pragma once

/**
 * ʹ��˵����
 * AddressLoader<>Ϊ����������е������ļ�·���Ĺ���
 * ģ��ֵΪ��Ҫ����·��
 * ��������̳�AddressLoader<5>��ʾ������������5��·��
 *
 * ���磺
 * ͨ��loadAddress(3, "xxxx")���õ�����·������0��ʼ��
 * ͨ��getAddress(2)��ȡ�ڶ���·��
 * ע��·���������ȱ�load������get��������׳��쳣
 *
 * ��������Ϊ�ӿڶ��������Ա���̳�
 *
 * �� 2019.10.6
 */

#include "../define/define.hpp"

namespace Simulator
{
	template<uint Num>
	class AddressLoader
	{
	public:
		AddressLoader()
			: _is_address_loaded()
			, _address()
		{
			_is_address_loaded.fill(false);
		}

		void loadAddress(uint num_, const string& str_)
		{
			_address[num_] = str_;
			_is_address_loaded[num_] = true;
		}

		bool isLoad(uint num_) const
		{
			return _is_address_loaded[num_];
		}

	protected:
		string getAddress(uint num_) const
		{
			if (!_is_address_loaded[num_])
				throw "Address is not initialized";
			else
				return _address[num_];
		}

	private:
		array<bool, Num> _is_address_loaded;
		array<string, Num> _address;
	};
	
}