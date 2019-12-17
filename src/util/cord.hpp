#pragma once

/** 
 * ʹ��˵��
 * 
 * CordΪ�����࣬�ڲ�һ��xһ��y����Ϊuint�ͣ��޲�Ĭ��Ϊ0
 * ������Ϊmap/set��key����������Ƚ����ȱȽ�x���ٱȽ�y
 * ������Ϊunordered_map/unordered_set��key����ϣֵΪx��y��ϣֵ�����
 * ���Ա�ǿ������ת��Ϊһ��string�����ؽ���Ǵ������ŵ�һ�����꣬����"[4, 5]"
 * 
 * �ڲ���������̬����
 * stringAnalysis����һ���ַ���������һ�����꣬��ʽ����
 * ��������"[4, 5]"������Cord{4, 5}
 * chessboardDist�����������꣬�������ǵ����̾���
 * euclidDist�����������꣬�������ǵ�ŷ����þ���
 * 
 * �� 2019.7.4
 * 
 *  */

#include "../define/define.hpp"
#include "util.hpp"

namespace Simulator
{
	struct Cord final
	{
		uint x{};
		uint y{};

		explicit Cord(uint a_ = 0, uint b_ = 0)
			: x(a_)
			, y(b_)
		{
		}

		Cord(const Cord& b_) = default;

		Cord(Cord&& b_) noexcept
		{
			swap(*this, b_);
		}

		Cord& operator=(const Cord& b_)
		{
			auto copy(b_);
			swap(*this, copy);
			return *this;
		}

		Cord& operator=(Cord&& b_) noexcept
		{
			swap(*this, b_);
			return *this;
		}

		~Cord() = default;

		bool operator==(const Cord& b_) const
		{
			return (x == b_.x && y == b_.y);
		}

		bool operator!=(const Cord& b_) const
		{
			return (x != b_.x || y != b_.y);
		}

		bool operator<(const Cord& b_) const
		{
			return ((x < b_.x) ? true : (y < b_.y));
		}

		friend ostream& operator <<(ostream& output_, Cord& b_)
		{
			output_ << static_cast<string>(b_);
			return output_;
		}

		void swap(Cord& a_, Cord& b_) const noexcept
		{
			using std::swap;
			swap(a_.x, b_.x);
			swap(a_.y, b_.y);
		}

		explicit operator string() const
		{
			auto s = "[" + std::to_string(x) + ", " + std::to_string(y) + "]";
			return s;
		}

		static uint chessboardDist(const Cord& a_, const Cord& b_)
		{
			return abs(static_cast<int>(a_.x - b_.x)) + abs(static_cast<int>(a_.y - b_.y));
		}

		static double euclidDist(const Cord& a_, const Cord& b_)
		{
			return sqrt((a_.x - b_.x) * (a_.x - b_.x) + (a_.y - b_.y) * (a_.y - b_.y));
		}

		// input string = [x, x]
		static auto stringAnalysis(string s_)->Cord
		{
			Util::deleteAllMark(s_, "[");
			Util::deleteAllMark(s_, "]");
			Util::deleteAllMark(s_, " ");
			vector<string> tmp = Util::splitString(s_, ",");
			return Cord{ static_cast<uint>(std::stoi(tmp[0])), static_cast<uint>(std::stoi(tmp[1])) };
		}
	};
}

namespace std
{
	template <>
	struct hash<Simulator::Cord> final
	{
	public:
		size_t operator()(const Simulator::Cord& b_) const noexcept
		{
			return hash<uint>()(b_.x) ^ hash<uint>()(b_.y);
		}
	};
}



