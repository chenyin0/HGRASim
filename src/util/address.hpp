#pragma once

/**
 * 使用说明：
 * AddressLoader<>为独立组件，承担设置文件路径的功能
 * 模板值为需要几个路径
 * 例如子类继承AddressLoader<5>表示子类允许设置5个路径
 *
 * 例如：
 * 通过loadAddress(3, "xxxx")设置第三个路径（从0开始）
 * 通过getAddress(2)获取第二个路径
 * 注意路径必须首先被load过才能get，否则会抛出异常
 *
 * 这个组件作为接口独立，可以被多继承
 *
 * 行 2019.10.6
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