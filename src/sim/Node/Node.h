#pragma once
#include "../../define/define.hpp"
#include "../inout.h"
#include "../Buffer/buffer.h"
#include "../../preprocess/preprocess.h"
#include "../factory/factory.h"
#include "../debug.h"
namespace Simulator::Array
{
	//Node��Ӧ�������нڵ�ĳ������
	//�ݶ����нڵ����ӵ������λ����������
	class Node
	{
	protected:
		Preprocess::ArrayPara system_parameter;
		queue<std::vector<uint>> config_reg;
		bool print_screen;
		//std::vector<Protocol*> ptcs;
		//��������˿ڳ���������ʱ����������չ

	public:
		explicit Node(const Preprocess::ArrayPara &para);    //��ȡsystem.xml��ȷ�������������
		//virtual void print() = 0;
		virtual ~Node() = default;
		virtual void getInput(uint port, Port_inout input) = 0;
		virtual void getBp(uint port, bool input) = 0;
		//virtual void setOutput(Port_inout& output, uint& port) = 0;

	};
}
