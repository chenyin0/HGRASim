#pragma once
#include "../../define/define.hpp"
#include "../inout.h"
#include "../Buffer/buffer.h"
#include "../../preprocess/preprocess.h"
#include "../factory/factory.h"
#include "../debug.h"
namespace Simulator::Array
{
	//Node类应该是所有节点的抽象基类
	//暂定所有节点最多拥有两种位宽的输入输出
	class Node
	{
	protected:
		Preprocess::ArrayPara system_parameter;
		queue<std::vector<uint>> config_reg;
		bool print_screen;
		//std::vector<Protocol*> ptcs;
		//输入输出端口超过这两类时在子类中扩展

	public:
		explicit Node(const Preprocess::ArrayPara &para);    //读取system.xml来确定输入输出类型
		//virtual void print() = 0;
		virtual ~Node() = default;
		virtual void getInput(uint port, Port_inout input) = 0;
		virtual void getBp(uint port, bool input) = 0;
		//virtual void setOutput(Port_inout& output, uint& port) = 0;

	};
}
