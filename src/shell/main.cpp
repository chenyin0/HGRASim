#include "../preprocess/preprocess.h"
#include "../pr/mapping/simulate_anneal/simulate_anneal.h"
#include "../pr/mapping/routing_engine/routing_engine.h"
#include "../bridge/bridge.h"
#include "../sim/HgraSystem.h"
#include "../sim/debug.h"
#include <iostream>
#include <fstream>
#include "../pr/resource_graph/pr_res_graph.h"
#include "../sim/ClkDomain.h"
//#include "../sim/mem_system/Lsu.h"


using namespace Simulator;
using std::cout;
using std::endl;


int main()
{
	// ����·��
//	GlobalPara::getInstance()->setInputAddr(InputAddr::DFG_xml_addr, R"(.\resource\input\break_test.xml)");
//	GlobalPara::getInstance()->setInputAddr(InputAddr::DFG_xml_addr, R"(.\resource\input\NW.xml)");
	GlobalPara::getInstance()->setInputAddr(InputAddr::DFG_xml_addr, R"(.\resource\input\DFG_edge_detect.xml)");
//	GlobalPara::getInstance()->setInputAddr(InputAddr::DFG_xml_addr, R"(.\resource\input\no_balance.xml)");
//	GlobalPara::getInstance()->setInputAddr(InputAddr::DFG_xml_addr, R"(.\resource\input\aluin_test.xml)");
//	GlobalPara::getInstance()->setInputAddr(InputAddr::DFG_xml_addr, R"(.\resource\input\condition.xml)");
//    GlobalPara::getInstance()->setInputAddr(InputAddr::DFG_xml_addr, R"(.\resource\input\continue_test.xml)");
	GlobalPara::getInstance()->setInputAddr(InputAddr::parameter_xml_addr, R"(.\resource\input\system.xml)");
	GlobalPara::getInstance()->setInputAddr(InputAddr::architecture_xml_addr, R"(.\resource\input\ArchConfig.xml)");

	GlobalPara::getInstance()->setOutputAddr(OutputAddr::resource_block_graph_info_xml_addr, R"(.\resource\output\Block.xml)");
	GlobalPara::getInstance()->setOutputAddr(OutputAddr::resource_port_graph_info_xml_addr, R"(.\resource\output\Port.xml)");
	GlobalPara::getInstance()->setOutputAddr(OutputAddr::resource_segment_graph_info_xml_addr, R"(.\resource\output\Segment.xml)");
	GlobalPara::getInstance()->setOutputAddr(OutputAddr::resource_wire_graph_info_xml_addr, R"(.\resource\output\Wire.xml)");

	GlobalPara::getInstance()->setOutputAddr(OutputAddr::placement_result_xml_addr, R"(.\resource\output\Placement.xml)");
	GlobalPara::getInstance()->setOutputAddr(OutputAddr::routing_result_xml_addr, R"(.\resource\output\Routing.xml)");

	// ����DFGͼ��Ϣ
	AppGraph app_graph;

	// ������Դͼ��Ϣ
	Mpr::ResourceGraph resource_graph;
	resource_graph.showInfo();

	// ����ֶ����֣���DFGͼ�л�ȡ������Ϣ
	//vector<Cord> placement = app_graph.getManualPlacement();
	
	// �Զ����׶β��� - ���ò���
	//Mpr::SimulateAnnealStage stage_simulate_anneal(app_graph);
	//stage_simulate_anneal.setParaNumber(Mpr::SimulateAnnealStage::Stage::LS, 30, 0.7, 0.000001);
	//stage_simulate_anneal.setParaCostFun(Mpr::SimulateAnnealStage::Stage::LS, Mpr::CostFunType::ChessBoard);
	//stage_simulate_anneal.setParaTemperatureFun(Mpr::SimulateAnnealStage::Stage::LS, Mpr::TemperatureFunType::Slow);
	//stage_simulate_anneal.setParaNumber(Mpr::SimulateAnnealStage::Stage::other, 30, 0.7, 0.0000001);
	//stage_simulate_anneal.setParaCostFun(Mpr::SimulateAnnealStage::Stage::other, Mpr::CostFunType::ChessBoard);
	//stage_simulate_anneal.setParaTemperatureFun(Mpr::SimulateAnnealStage::Stage::other, Mpr::TemperatureFunType::Slow);

	// �Զ����׶β��� - ִ�в���
	//stage_simulate_anneal.goSimulateAnneal();

	// �Զ����׶β��� - ���沼�ֽ��
	//string placement_addr = GlobalPara::getInstance()->getOutputAddr(OutputAddr::placement_result_xml_addr);
	//stage_simulate_anneal.savePlacementResult(placement_addr);

	// ��ò��ֽ��
	//vector<Cord> placement = stage_simulate_anneal.getPlacementResult();
	
	// �Զ������� - ׼������
	//Mpr::RoutingEngine routing_engine(resource_graph, app_graph);

	// �Զ������� - ���ز��ֽ��
	//routing_engine.loadPlacement(&placement);

	// �Զ������� - ִ�в���
	//routing_engine.goRouting();

	// �Զ������� - ������
	//string routing_addr = GlobalPara::getInstance()->getOutputAddr(OutputAddr::routing_result_xml_addr);
	//routing_engine.saveRoutingResult(routing_addr);

	Bridge b(app_graph);
	
	uint debug_level;
	
//	std::ifstream memoryInFile("");
//	std::ifstream memoryInFile("resource/input/NW_memoryInFile.txt");
	std::ifstream memoryInFile("resource/input/memoryInFile_edge_detect.txt");
//	std::ifstream memoryInFile("resource/input/memoryInFileCond.txt");
	std::ofstream memoryWritefile("resource/output/memorywrite.txt");
//	memoryInFile.open("memoryInFile.txt");
//	Simulator::Array::MemoryData memory(100000);
	Simulator::Array::MemoryData::getInstance()->readFromFile(memoryInFile);
	
	Simulator::Array::ClkDomain clk();
	Simulator::Array::Debug debug(1, ".\\resource\\output\\DebugReg.txt", ".\\resource\\output\\DebugReg.txt");
	Simulator::Array::HgraArray sim_single(app_graph);
	sim_single.run();
	Simulator::Array::MemoryData::getInstance()->writeToFile(memoryWritefile);
	cout << "clk=" << Simulator::Array::ClkDomain::getInstance()->getClk() << endl;
	system("pause");
	return 0;
}