#pragma once

#include "../define/global.hpp"

namespace Simulator::FunctionalProgram
{
	enum class OpType
	{
		add,
		sub,
		mul,
		max,
		min
	};

	interface OperationBase
	{
		virtual ~OperationBase() = default;
	};

	template <typename DataType, OpType op>
	class Operation : OperationBase
	{
	};

	template <typename DataType>
	class Operation<DataType, OpType::add> : OperationBase
	{
	public:
		DataType operator() (DataType a_, DataType b_)
		{
			return a_ + b_;
		}
	};

	template <typename DataType>
	class Operation<DataType, OpType::sub> : OperationBase
	{
	public:
		DataType operator() (DataType a_, DataType b_)
		{
			return a_ - b_;
		}
	};

	template <typename DataType>
	class Operation<DataType, OpType::mul> : OperationBase
	{
	public:
		DataType operator() (DataType a_, DataType b_)
		{
			return a_ * b_;
		}
	};

	template <typename DataType>
	class Operation<DataType, OpType::max> : OperationBase
	{
	public:
		DataType operator() (DataType a_, DataType b_)
		{
			return a_ < b_ ? b_ : a_;
		}
	};

	template <typename DataType>
	class Operation<DataType, OpType::min> : OperationBase
	{
	public:
		DataType operator() (DataType a_, DataType b_)
		{
			return a_ < b_ ? a_ : b_;
		}
	};

	template <typename DataType, OpType op>
	class CgraDoubleMap final
	{
	public:
		CgraDoubleMap()
			: _operation(Operation<DataType, op>())
		{
		}

		vector<DataType> operator() (vector<DataType> a_, vector<DataType> b_)
		{
			if (a_.size() != b_.size())
				throw "size not match";

			vector<DataType> temp(a_.size());
			for (int i = 0; i < temp.size(); i++)
			{
				temp[i] = _operation(a_[i], b_[i]);
			}
			return temp;
		}

	private:
		Operation<DataType, op> _operation;
	};

	template<typename DataType, OpType op>
	class ScalarFold final
	{
	public:
		ScalarFold(DataType num1, DataType num2)
			: _op_num_1(num1)
			, _op_num_2(num2)
			, _operation(Operation<DataType, op>())
		{
		}

		template <OpType op1, OpType op2>
		ScalarFold(ScalarFold<DataType, op1> sc1, ScalarFold<DataType, op2> sc2)
			: _op_num_1(sc1())
			, _op_num_2(sc2())
			, _operation(Operation<DataType, op>())
		{
		}

		template <OpType op1>
		ScalarFold(ScalarFold<DataType, op1> sc1, DataType num2)
			: _op_num_1(sc1())
			, _op_num_2(num2)
			, _operation(Operation<DataType, op>())
		{
		}

		template <OpType op2>
		ScalarFold(DataType num1, ScalarFold<DataType, op2> sc2)
			: _op_num_1(num1)
			, _op_num_2(sc2())
			, _operation(Operation<DataType, op>())
		{
		}

		DataType operator() ()
		{
			return op(_op_num_1, _op_num_2);
		}

		template<OpType op2>
		bool operator==(ScalarFold<DataType, op2> sc_) const
		{
			return (*this)() == sc_();
		}

	private:
		DataType _op_num_1;
		DataType _op_num_2;
		Operation<DataType, op> _operation;
	};

	template<typename DataType>
	class VoidMap final
	{
	public:
		VoidMap(const vector<DataType>& vec_, std::function<void(DataType)> fun_)
			: _vec(vec_)
			, _fun(fun_)
		{
		}

		void operator() ()
		{
			for (auto& ele : _vec)
				_fun(ele);
		}

	private:
		const vector<DataType>& _vec;
		std::function<void(DataType)> _fun;
	};

	class FpTool final
	{
	public:
		static auto linspace(int start_, int step_, uint size_) -> vector<int>
		{
			vector<int> result(size_);
			for (int i = 0; i < size_; i++)
				result[i] = start_ + i * step_;
			return result;
		}

		template<typename DataType>
		static auto leVisit(const vector<DataType>& vec_, uint pos_) -> DataType
		{
			return vec_[pos_];
		}

		template<typename DataType, typename DataType2, OpType op>
		static auto leVisit(const vector<DataType>& vec_, ScalarFold<DataType2, op> sc_) -> DataType
		{
			return vec_[sc_()];
		}

		template<typename DataType>
		static auto seStore(vector<DataType>& vec_, uint pos_, DataType ele_) -> void
		{
			vec_[pos_] = ele_;
		}

		template<typename DataType, typename DataType2, OpType op>
		static auto seStore(vector<DataType>& vec_, ScalarFold<DataType2, op> sc_, DataType ele_) -> void
		{
			vec_[sc_()] = ele_;
		}

		template<typename DataType, typename DataType2, OpType op>
		static auto seStore(vector<DataType>& vec_, uint pos_, ScalarFold<DataType2, op> sc_) -> void
		{
			vec_[pos_] = sc_();
		}

		template<typename DataType, typename DataType1, OpType op1, typename DataType2, OpType op2>
		static auto seStore(vector<DataType>& vec_, ScalarFold<DataType1, op1> sc1_, ScalarFold<DataType2, op2> sc2_) -> void
		{
			vec_[sc1_()] = sc2_();
		}
	};
}
