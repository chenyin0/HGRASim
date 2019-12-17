#include "simulate_anneal.h"
#if DEBUG_CONSOLE
#include <iostream>
#endif

namespace Simulator::Mpr
{
	SimulateAnnealBase::SimulateAnnealBase(const AppGraph& app_graph_)
		: _app_graph(app_graph_)
		, _result(app_graph_)
		, _cost_fun_database(app_graph_)
		, _temperature_fun_database()
	{
	}

	vector<Cord> SimulateAnnealBase::getPlacementResult() const
	{
		return _result.generateVectorCord();
	}

	void SimulateAnnealBase::savePlacementResult(const string& addr_) const
	{
		vector<Cord> all_result = getPlacementResult();

		const vector<AppNode>& nodes = _app_graph.getNodes();

		const char* declaration = R"(<?xml version="1.0" encoding="UTF-8" standalone="no"?>)";
		XMLDocument doc;
		doc.Parse(declaration);

		XMLElement* root = doc.NewElement("PlacementResult");
		doc.InsertEndChild(root);

		for (uint i = 0; i < all_result.size(); i++)
		{
			BlockType type = nodes[i].type;
			uint index = nodes[i].type_num;
			Cord cord = all_result[i];

			XMLElement* node_xml = doc.NewElement("node");
			node_xml->SetAttribute("type", BlockTypeConverter::toString(type).c_str());
			node_xml->SetAttribute("num", std::to_string(index).c_str());
			node_xml->SetAttribute("placement", static_cast<string>(cord).c_str());

			root->InsertEndChild(node_xml);
		}

		XMLElement* array_xml = doc.NewElement("array");
		root->InsertEndChild(array_xml);

		uint row_size = Preprocess::Para::getInstance()->getArchPara().row_num;
		uint col_size = Preprocess::Para::getInstance()->getArchPara().col_num;

		for (uint row = 0; row < row_size; row++)
		{
			string row_str;
			for (uint col = 0; col < col_size; col++)
			{
				Cord cord{ row, col };
				uint node_num = Util::findIndex(all_result, cord);
				string temp;
				if (node_num != UINT_MAX)
				{
					BlockType type = nodes[node_num].type;
					uint index = nodes[node_num].type_num;
					temp = BlockTypeConverter::toString(type) + std::to_string(index);
					while (temp.size() < 5)
						temp += "-";
					temp += " ";
				}
				else
				{
					temp = "*****";
					temp += " ";
				}
				row_str += temp;
			}
			row_str.pop_back();

			XMLElement* row_xml = doc.NewElement("row");
			row_xml->SetText(row_str.c_str());
			array_xml->InsertEndChild(row_xml);
		}

		doc.SaveFile(addr_.c_str());
	}

	bool SimulateAnnealStage::StagePara::check() const
	{
		return cost_fun != nullptr && temperature_fun != nullptr;
	}

	SimulateAnnealStage::SimulateAnnealStage(const AppGraph& app_graph_)
		: SimulateAnnealBase(app_graph_)
	{
		initStage();
	}

	void SimulateAnnealStage::goSimulateAnneal()
	{
		// prepare
		DEBUG_ASSERT(_ls_stage.check());
		DEBUG_ASSERT(_other_stage.check());
		
		SimulateAnnealEngine sa_engine(_app_graph);
		sa_engine.loadPlacement(&_result);

		// LS stage
		SimulateAnnealPara ls_para(
			_ls_stage.inner_number,
			_ls_stage.lambda,
			_ls_stage.epsilon,
			_ls_stage.nodes_to_place,
			_ls_stage.temperature_fun,
			_ls_stage.cost_fun
		);

		sa_engine.loadPara(&ls_para);
		sa_engine.goSimulatedAnneal();

		// other stage
		SimulateAnnealPara other_para(
			_other_stage.inner_number,
			_other_stage.lambda,
			_other_stage.epsilon,
			_other_stage.nodes_to_place,
			_other_stage.temperature_fun,
			_other_stage.cost_fun
		);
		
		sa_engine.detachPara();
		sa_engine.loadPara(&other_para);
		sa_engine.goSimulatedAnneal();

	}

	void SimulateAnnealStage::setParaNumber(Stage stage_, int inner_number_, double lambda_, double epsilon_)
	{
		if (stage_ == Stage::LS)
		{
			_ls_stage.inner_number = inner_number_;
			_ls_stage.lambda = lambda_;
			_ls_stage.epsilon = epsilon_;
		}
		else if (stage_ == Stage::other)
		{
			_other_stage.inner_number = inner_number_;
			_other_stage.lambda = lambda_;
			_other_stage.epsilon = epsilon_;
		}
	}

	void SimulateAnnealStage::setParaCostFun(Stage stage_, CostFunType cost_fun_type_)
	{
		if (stage_ == Stage::LS)
		{
			_ls_stage.cost_fun = _cost_fun_database.getFunction(cost_fun_type_);
		}
		else if (stage_ == Stage::other)
		{
			_other_stage.cost_fun = _cost_fun_database.getFunction(cost_fun_type_);
		}
	}

	void SimulateAnnealStage::setParaTemperatureFun(Stage stage_, TemperatureFunType temperature_fun_type_)
	{
		if (stage_ == Stage::LS)
		{
			_ls_stage.temperature_fun = _temperature_fun_database.getFunction(temperature_fun_type_);
		}
		else if (stage_ == Stage::other)
		{
			_other_stage.temperature_fun = _temperature_fun_database.getFunction(temperature_fun_type_);
		}
	}

	void SimulateAnnealStage::initStage()
	{
		uint index = 0;
		const vector<AppNode>& app_nodes = _app_graph.getNodes();
		for_each(begin(app_nodes), end(app_nodes), [&](const AppNode& app_node_)
		{
			BlockType type = app_node_.type;

			if (type == BlockType::ls)
				_ls_stage.nodes_to_place.push_back(index);
			else
				_other_stage.nodes_to_place.push_back(index);
			
			index++;
		});
	}

	bool SimulateAnnealOverall::Para::check() const
	{
		return cost_fun != nullptr && temperature_fun != nullptr;
	}

	SimulateAnnealOverall::SimulateAnnealOverall(const AppGraph& app_graph_)
		: SimulateAnnealBase(app_graph_)
	{
		initPara();
	}

	void SimulateAnnealOverall::goSimulateAnneal()
	{
		// prepare
		DEBUG_ASSERT(_para.check());

		SimulateAnnealEngine sa_engine(_app_graph);
		sa_engine.loadPlacement(&_result);

		SimulateAnnealPara para(
			_para.inner_number,
			_para.lambda,
			_para.epsilon,
			_para.nodes_to_place,
			_para.temperature_fun,
			_para.cost_fun
		);

		sa_engine.detachPara();
		sa_engine.loadPara(&para);
		sa_engine.goSimulatedAnneal();
	}

	void SimulateAnnealOverall::setParaNumber(int inner_number_, double lambda_, double epsilon_)
	{
		_para.inner_number = inner_number_;
		_para.lambda = lambda_;
		_para.epsilon = epsilon_;
	}

	void SimulateAnnealOverall::setParaCostFun(CostFunType cost_fun_type_)
	{
		_para.cost_fun = _cost_fun_database.getFunction(cost_fun_type_);
	}

	void SimulateAnnealOverall::setParaTemperatureFun(TemperatureFunType temperature_fun_type_)
	{
		_para.temperature_fun = _temperature_fun_database.getFunction(temperature_fun_type_);
	}

	void SimulateAnnealOverall::initPara()
	{
		uint index = 0;
		const vector<AppNode>& app_nodes = _app_graph.getNodes();
		for_each(begin(app_nodes), end(app_nodes), [&](const AppNode& app_node_)
		{
			BlockType type = app_node_.type;
			_para.nodes_to_place.push_back(index);
			index++;
		});
	}
}
