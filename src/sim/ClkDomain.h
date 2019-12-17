#pragma once
#include "../util/util.hpp"

namespace Simulator::Array
{
	/**
	 * ��- �޸�Ϊ��ȷ�ĵ����࣬clk��ȫ�־�̬������ʵ��
	 * ��- ʹ��clk����������
	 * 
	 * ʹ��ǰ�ȳ�ʼ�� ClkDomain::initClk()
	 * ÿ�����ڵ����Լ� ClkDomain::selfAdd()
	 * ����Ҫ�˹����ó�ʼֵ����ClkDomain::setClk(xxx)
	 * ����ʱ���� ClkDomain::getClk()
	 *
	 * �� 2019.10.6
	 */
	class ClkDomain : public Singleton<ClkDomain>
	{
	private:
		friend Singleton<ClkDomain>;

		ClkDomain() = default;

	public:
		static uint getClk();

		static void selfAdd();

		static void setClk(uint clk_);

		static void initClk();

	private:
		static uint _clk;
	};

	
}
