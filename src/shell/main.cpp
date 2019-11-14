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
	// 设置路径
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

	// 生成DFG图信息
	AppGraph app_graph;

	// 生成资源图信息
	Mpr::ResourceGraph resource_graph;
	resource_graph.showInfo();

	// 如果手动布局，从DFG图中获取布局信息
	//vector<Cord> placement = app_graph.getManualPlacement();
	
	// 自动化阶段布局 - 设置参数
	//Mpr::SimulateAnnealStage stage_simulate_anneal(app_graph);
	//stage_simulate_anneal.setParaNumber(Mpr::SimulateAnnealStage::Stage::LS, 30, 0.7, 0.000001);
	//stage_simulate_anneal.setParaCostFun(Mpr::SimulateAnnealStage::Stage::LS, Mpr::CostFunType::ChessBoard);
	//stage_simulate_anneal.setParaTemperatureFun(Mpr::SimulateAnnealStage::Stage::LS, Mpr::TemperatureFunType::Slow);
	//stage_simulate_anneal.setParaNumber(Mpr::SimulateAnnealStage::Stage::other, 30, 0.7, 0.0000001);
	//stage_simulate_anneal.setParaCostFun(Mpr::SimulateAnnealStage::Stage::other, Mpr::CostFunType::ChessBoard);
	//stage_simulate_anneal.setParaTemperatureFun(Mpr::SimulateAnnealStage::Stage::other, Mpr::TemperatureFunType::Slow);

	// 自动化阶段布局 - 执行布局
	//stage_simulate_anneal.goSimulateAnneal();

	// 自动化阶段布局 - 保存布局结果
	//string placement_addr = GlobalPara::getInstance()->getOutputAddr(OutputAddr::placement_result_xml_addr);
	//stage_simulate_anneal.savePlacementResult(placement_addr);

	// 获得布局结果
	//vector<Cord> placement = stage_simulate_anneal.getPlacementResult();
	
	// 自动化布线 - 准备引擎
	//Mpr::RoutingEngine routing_engine(resource_graph, app_graph);

	// 自动化布线 - 加载布局结果
	//routing_engine.loadPlacement(&placement);

	// 自动化布线 - 执行布线
	//routing_engine.goRouting();

	// 自动化布线 - 保存结果
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