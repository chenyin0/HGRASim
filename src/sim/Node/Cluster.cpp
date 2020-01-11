#include <iomanip>
#include "Cluster.h"


using namespace Simulator::Array;
Cluster::Cluster(shared_ptr<Simulator::Preprocess::ClusterInterface> attr):attribution(attr){
	//attribution = attr;
}
void Cluster::enable() {
	update_en = true;
}
void Cluster::update()
{
	uint size = attribution->end_node - attribution->start_node + 1;
	if (update_en) {
		update_en = false;
		counter += attribution->step;
		if (counter > size - 1)
		{
			counter = (counter - size) % size;
		}
	}
}
ClusterGroup::ClusterGroup():attribution(Preprocess::DFG::getInstance()->getDfgCluster()){
	for (auto& [cluster, cluster_attr] : attribution.getAllClusters()) {
		clusters.insert({ cluster,make_shared<Cluster>(cluster_attr) });
	}
}
void ClusterGroup::insert(std::map<std::pair<Simulator::NodeType, uint>, uint>& index2order_, map<uint, Simulator::Array::Loadstore_element*>& lse_map_){
	index2order = index2order_;
	lse_map = lse_map_;
}
void ClusterGroup::update()
{
	for (auto& [clusterId, cluster] : clusters)
	{
		cluster->update();
	}
}
void ClusterGroup::enable(Simulator::NodeType nodetype_, uint cluster_id_)
{
	clusters[std::make_pair(nodetype_, cluster_id_)]->enable();
}
int ClusterGroup::index2Id(Simulator::NodeType nodetype_, uint index)
{
	if (attribution.getClusterID().find({ nodetype_ ,index }) == attribution.getClusterID().end()) {
		return -1;
	}
	else {
		auto ls_cluster = attribution.getClusterID();
		return ls_cluster[{nodetype_, index }];
	}
}
bool ClusterGroup::exists(Simulator::NodeType nodetype_, uint index) {
	if (index2Id(nodetype_, index) != -1) {
		return true;
	}
	else {
		return false;
	}
}
bool ClusterGroup::canRecv(Simulator::NodeType nodetype_, uint index) {
	if (clusters.find({ nodetype_ ,index2Id(nodetype_,index) })->second->counter == index) {
		return true;
	}
	else {
		return false;
	}
}
//Simulator::Array::Loadstore_element* ClusterGroup::getEle(NodeType nodetype_, uint lse_tag) {
//	clusters.find({ nodetype_ ,index2Id(nodetype_,index) })->second
//}