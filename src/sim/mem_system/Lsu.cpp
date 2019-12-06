#include "Lsu.h"                                        
#include <vector>
#include "../Node/LSE.h"

//#define leNums    2  
//#define seNums    2

using namespace DRAMSim;

//extern bool system_parameter.print_screen;
namespace DRAMSim {
	TabLine::TabLine()
	{
		//tabTAG_ = tabtag;
		ADDR_ = 0;
		rdwr = false;
		valid = 0;
		pref = 0;
		transid = 0;
//		pe_round = 0;
		vec = 0;
		complete = 0;
		bypass = false;
		offset.resize(CACHE_LINE_SIZE);
		//	offset_4.resize(OFFSET_DEPTH);
		for (vector<Comb_Offset>::iterator it = offset.begin(); it < offset.end(); it++)
		{
			(*it).valid = 0;
		}
		/*	for (vector<Comb_Offset_4>::iterator it = offset_4.begin(); it < offset_4.end(); it++)
			{
				(*it).valid = 0;
			}*/
	}



	void TabLine::reset()
	{
		ADDR_ = 0;
		rdwr = false;
		valid = 0;
		pref = 0;
		transid = 0;
		complete = 0;
		bypass = false;
		for (int i = 0; i < CACHE_LINE_SIZE; i++)         //清小valid
			offset[i].valid = 0;

		/*	for (int i = 0; i < OFFSET_DEPTH; i++)
				offset_4[i].valid = 0;*/
	}


	ArbitratorLine::ArbitratorLine(const Simulator::Preprocess::ArrayPara para, Simulator::Array::Loadstore_element * lse, uint32_t TAG) :Node(para)
	{
		lse_ = lse;
		//	se_ = lse;
		TAG_ = TAG;
		pe_tag = 0;
	//	pe_round = 0;
	}


	bool ArbitratorLine::AddTrans(Simulator::Array::Port_inout_lsu input,uint32_t TAG , bool bypass)
	{
		valid = input.valid;
		ADDR_ = input.value_addr;
		pe_tag = input.tag;
		rdwr = input.rdwr;
	//	pe_round = PE_ROUND;
		TAG_ = TAG;
		this->bypass = bypass;
		pref = 0;
		return true;
	}

	void ArbitratorLine::returnACK()
	{
		if (TAG_ < system_parameter.lse_num)
			lse_->callbackACK();    //pay attention to this name
	/*	else if (TAG_ >= leNums && TAG_ < (leNums + seNums))
			se_->callbackACK();*/
		else
			PRINTM("error occurs in ArbitratorLine::returnACK()");
	}


	Arbitrator::Arbitrator(const Simulator::Preprocess::ArrayPara para, map<uint, Simulator::Array::Loadstore_element*> lse_map) :Node(para)                //out_num unused
	{
		//	lsize = leNums;                                    //as a pointer the return value of sizeof() is fixed as 8
		//	ssize = seNums;
	//	lseSize = lse_map.size();
		pointer = 0;
		uint relse_counter = 0;

		for (uint32_t i = 0; i < lse_map.size(); i++)
		{
			if (lse_map[i]->getAttr()->ls_mode != Simulator::LSMode::dummy) {
				lse2relse[i] = relse_counter;
				ArbitratorLine* arbline = new ArbitratorLine(para, lse_map[i], relse_counter++);                       //隐含了ArbitratorLines的下标即为TAG的意思
				ArbitratorLines.push_back(arbline);
			}
		}

		/*	for (uint32_t i = 0; i < ssize; i++)
			{
				ArbitratorLine* arbline = new ArbitratorLine(NULL, sep[i], i + lsize);
				ArbitratorLines.push_back(arbline);
			}*/
	}


	Arbitrator::~Arbitrator()
	{
		for (uint32_t i = 0; i < ArbitratorLines.size(); i++)
		{
			delete ArbitratorLines[i];
		}

		/*	for (uint32_t i = 0; i < ssize; i++)
			{
				delete ArbitratorLines[lsize + i];
			}*/
	}

	//利用TAG寻找相应进行动作的tabline
	bool Arbitrator::AddTrans(Simulator::Array::Port_inout_lsu input, uint32_t lse_index, bool bypass)
	{
		if ((lse_index >= system_parameter.lse_num))
		{
			PRINTM("error occurs in Arbitrator:: AddTrans, the tag is " << lse2relse[lse_index]);
			return false;
		}
		else
			return ArbitratorLines[lse2relse[lse_index]]->AddTrans(input, lse2relse[lse_index],bypass);
	}

	void Lsu::addpoped_addr(int addr, bool bypass)
	{
		if (poped_addr.find(addr) != poped_addr.end()) {
			if (bypass == true) {
				if (poped_addr[addr] == Simulator::RecallMode::cached) {
					poped_addr[addr] = Simulator::RecallMode::both;
				}
				else {
					poped_addr[addr] = Simulator::RecallMode::nocache;
				}
			}
			else {
				if (poped_addr[addr] == Simulator::RecallMode::nocache) {
					poped_addr[addr] = Simulator::RecallMode::both;
				}
				else {
					poped_addr[addr] = Simulator::RecallMode::cached;
				}
			}
		}
		else {
			if (bypass == true) {
				poped_addr[addr] = Simulator::RecallMode::nocache;
			}
			else {
				poped_addr[addr] = Simulator::RecallMode::cached;
			}
		}
	}

	Lsu::Lsu(const Simulator::Preprocess::ArrayPara para, map<uint, Simulator::Array::Loadstore_element*> lse_map) :Node(para)
	{
		cachefile.open("Cache.txt");                  //记录访存的cache信息
		overlapfile_1.open("Overlap1.txt");
		ClockCycle = 0;
		finished_trans = 0;
		total_trans = 0;
		finished_rd = 0;
		finished_wr = 0;
		finished_pref = 0;
		total_wr = 0;
		total_rd = 0;
		transid = 0;
		lseSize = lse_map.size();
		Arbitrator* arb = new Arbitrator(para, lse_map);
		arbitrator = arb;

		cache = new Cache(para);
		cache->attach(this);
		//pre_fifo.resize(system_parameter.fifoline_num);
		post_table.resize(system_parameter.tabline_num);
//		inflight_reg = new TabLine();
		inflight_reg.resize(CACHE_BANK);
		for (int i = 0; i < CACHE_BANK; i++)
			inflight_reg[i] = new TabLine();
		for (short i = 0; i < post_table.size(); i++)
		{
			post_table[i] = new TabLine();
		}

		last_pref.resize(0);
		send_pointer = 0;

		cache_rd_reg = 0;
		MSHR_STATE = 0;
		miss_finished_cnter = 0;
		//fifo_pause_signal = 0;
		state3_reg = 0;

		post_offset.resize(system_parameter.tabline_num);
		for (int i = 0; i < system_parameter.tabline_num; i++)
			post_offset[i].resize(system_parameter.post_offset_depth);

		pre_fifo.resize(CACHE_BANK);
		config_in(lse_map);
//		for(auto &i)
	}


	Lsu::~Lsu()
	{
		delete arbitrator;
		delete cache;
		for (short i = 0; i < post_table.size(); i++)
		{
			delete post_table[i];
		}

	}


	bool Lsu::AddTrans(Simulator::Array::Port_inout_lsu input,uint TAG,bool bypass)
	{
//		return arbitrator->AddTrans(input.value_addr, TAG, input.valid, input.tag);
		return arbitrator->AddTrans(input, TAG,bypass);
	}


	void Lsu::AttachMem(DRAMSim::MultiChannelMemorySystem* memory)
	{
		mem = memory;
	}


	void Lsu::update()   //从后往前，并加入timing信息
	{
		print_enable = system_parameter.print_screen && Simulator::Array::ClkDomain::getInstance()->getClk() >= Simulator::Array::Debug::getInstance()->print_screen_begin
			&& Simulator::Array::ClkDomain::getInstance()->getClk() < Simulator::Array::Debug::getInstance()->print_screen_end;
		if (print_enable) {
			PRINTM("--------------------arbitrtor-----------------------");
			for (auto& i : arbitrator->ArbitratorLines)
			{
				PRINTM(i->valid << "_" << i->ADDR_ << "_" << i->pe_tag);
			}
		}
		memset(channel_occupy, 0, system_parameter.lse_num * sizeof(bool));
		ClockCycle++;
		mem->update();
		bus_update();
		//std::cout << "update!" << " " << ClockCycle << std::endl;
		//this_index = next_index;
		//this_index_v = next_index_v;

		cache->update();


	switch (MSHR_STATE)
	{
		case 0:                            //空闲状态
										   //
			break;

		case 1:                            //块处理状态   miss的请求完成了，需要返回到le上
		{
			bool sended = 0;                   //发过数的cycle不能变为状态0
			for (int index = 0; index < post_table.size(); index++)
			{
				if (post_table[index]->complete)
				{
					if (post_table[index]->valid)
					{
						bool reset = 1;
						for (short i = 0; i < CACHE_LINE_SIZE; i++)
						{
							if (post_table[index]->offset[i].valid)
							{
								if (!channel_occupy[post_table[index]->offset[i].pointer])
								{
									(arbitrator->ArbitratorLines[post_table[index]->offset[i].pointer]->lse_)->LSEcallback(post_table[index]->ADDR_ + i, ClockCycle, post_table[index]->offset[i].tag);
									if(print_enable)
										PRINTM("return mshr data to " << arbitrator->ArbitratorLines[post_table[index]->offset[i].pointer]->lse_->attribution->index << " " << post_table[index]->ADDR_ + i);
									post_table[index]->offset[i].valid = 0;
									channel_occupy[post_table[index]->offset[i].pointer] = 1;
									sended = 1;
								}
								else
									reset = 0;
							}
						}
						if (reset)
							post_table[index]->valid = 0;

					}
					else
					{
						for (int i = 0; i < system_parameter.post_offset_depth; i++)
						{
							if (post_offset[index][i].valid)
							{
								bool reset = 1;
								for (short j = 0; j < CACHE_LINE_SIZE; j++)
								{
									if (post_offset[index][i].offset[j].valid)
									{
										if (!channel_occupy[post_offset[index][i].offset[j].pointer])
										{
											(arbitrator->ArbitratorLines[post_offset[index][i].offset[j].pointer]->lse_)->LSEcallback(post_table[index]->ADDR_ + j, ClockCycle, post_offset[index][i].offset[j].tag);
											if (print_enable)
												PRINTM("return mshrpost_offset data to " << arbitrator->ArbitratorLines[post_offset[index][i].offset[j].pointer]->lse_->attribution->index << " " << post_table[index]->ADDR_ + j );
											post_offset[index][i].offset[j].valid = 0;
											channel_occupy[post_offset[index][i].offset[j].pointer] = 1;
											sended = 1;
										}
										else
											reset = 0;
									}
								}
								if (reset)
									post_offset[index][i].valid = 0;
							}
						}
					}
				}
			}
			//检测MSHR中是否仍有数据，以及是否有行全部完成需要reset
			bool AllEmpty = 1;
			for (short i = 0; i < system_parameter.tabline_num; i++)
			{
				if (post_table[i]->valid)
					AllEmpty = 0;
				else if (!post_table[i]->valid)
				{
					bool NotEmpty = 0;
					for (short j = 0; j < system_parameter.post_offset_depth; j++)
					{
						if (post_offset[i][j].valid)//这个valid代表还需要处理吗////////////////////////////////
						{
							NotEmpty = 1;
							break;
						}
					}

					if (!NotEmpty && post_table[i]->complete)
					{
						miss_finished_cnter--;
						release_poped_addr(post_table[i]->ADDR_);
						post_table[i]->reset();
					}

					if (NotEmpty)
						AllEmpty = 0;
				}
			}
			//如果这周期完成过请求，为了避免和inflightreg冲突，状态不能变为状态0//////////////////////////////////为什么变成了状态0//////////
			if (AllEmpty && !sended)
			{
				MSHR_STATE = 0;
			}

			break;
		}
	//完成inflight_reg中被阻塞的hit请求
	}



		//from fifo to cache/mem                       第1步:发送fifo顶部的请求
	short bank = bank_round;
	for (int times = 0; times < CACHE_BANK; times++)
	{
		if (!pre_fifo[bank].empty())
		{
			if (pre_fifo[bank].front()->rdwr && pre_fifo[bank].front()->valid)              //写请求
			{
				if (!pre_fifo[bank].front()->bypass) {
					if (!inflight_reg[bank]->valid && cache->trans_v[bank] == 0)
					{
						delete inflight_reg[bank];
						inflight_reg[bank] = pre_fifo[bank].front();                            //正在访问cache的请求进入inflight_reg
						if (!cache->addtranscation(pre_fifo[bank].front()->ADDR_, pre_fifo[bank].front()->rdwr,bank))
						{
							PRINTERRORM("error occurs for cache is busy when add trans!");
							system("pause");
						}
						//poped_addr.push_back(pre_fifo.front()->ADDR_);
						pre_fifo[bank].erase(pre_fifo[bank].begin());

						if (print_enable)
							PRINTM("WR transcation poped to cache from fifo ");
					}
					else
					{
						if (print_enable)
						{
							PRINTM("inflight_reg blocking!");
						}
					}
				}
				else {
					DEBUG_ASSERT(false);
					if (print_enable)
						PRINTM("WR transcation poped to mem from fifo addr"<< pre_fifo[bank].front()->ADDR_);
					mem->addTransaction(true, uint64_t(pre_fifo[bank].front()->ADDR_) << 2);////////////////////////////////////////有问题!!!!!!!!!!!!!!!!!!!!!!//////////////
					//addpoped_addr(pre_fifo[bank].front()->ADDR_,false);
					delete(pre_fifo[bank].front());
					pre_fifo[bank].erase(pre_fifo[bank].begin());
				}
			}
			else                                                             //读请求
			{
				if (pre_fifo[bank].front()->valid && !pre_fifo[bank].front()->pref && !pre_fifo[bank].front()->rdwr)
				{
					if (!(pre_fifo[bank].front()->bypass)) {
						if (NotSameBlock(pre_fifo[bank].front()->ADDR_, pre_fifo[bank].front()->bypass))                                      //非reserve请求发送到cache和inflight_rag
						{
							if (!inflight_reg[bank]->valid && cache->trans_v[bank] == 0)
							{
								delete inflight_reg[bank];
								inflight_reg[bank] = pre_fifo[bank].front();                                          //正在访问cache的请求进入inflight_reg
								if (!cache->addtranscation(pre_fifo[bank][0]->ADDR_, pre_fifo[bank][0]->rdwr,bank))
								{
									PRINTERRORM("error occurs for cache is busy when add trans!");
									system("pause");
								}
								//poped_addr.push_back(pre_fifo[0]->ADDR_);
								pre_fifo[bank].erase(pre_fifo[bank].begin());
							}
							else                                                  //blocking的情况下不做操作
							{
								if (print_enable)
								{
									PRINTM("inflight_reg blocking!");
									//system("pause");
								}
							}
						}
						else                                                                               //reserve请求直接发送到table而不需要去inflight_reg
						{
							if (addreserve2post(pre_fifo[bank][0]))//是同一个block
							{
								delete pre_fifo[bank].front();
								pre_fifo[bank].erase(pre_fifo[bank].begin());
							}
							else                                        //offset满,回到尾部          //mshr没有空位了，因此只能从fifo头回到fifo尾
							{
								pre_fifo[bank].push_back(pre_fifo[bank].front());
								pre_fifo[bank].erase(pre_fifo[bank].begin());
							}
						}
					}
					else {
						bool sended = false;
						if (!NotSameBlock(pre_fifo[bank].front()->ADDR_, pre_fifo[bank].front()->bypass)) {
							for (int i = 0; i < post_table.size(); i++) {
								if (post_table[i]->valid && (post_table[i]->ADDR_ / CACHE_LINE_SIZE == pre_fifo[bank].front()->ADDR_ / CACHE_LINE_SIZE) && post_table[i]->bypass) {
									for (int j = 0; j < system_parameter.post_offset_depth; j++) {
										if (!post_offset[i][j].valid) {
											//if (!system_parameter.bus_enable)
											//{
											//	//if (NotSameBlock(pre_fifo[bank].front()->ADDR_)) {
											//	//	mem->addTransaction(false, uint64_t(pre_fifo[bank].front()->ADDR_) << 2);    //一个字4字节，所以左移2位
											//	//}
											//	addpoped_addr(pre_fifo[bank].front()->ADDR_, true);
											//}
											//else if (system_parameter.bus_enable)
											//{
											//	//if (NotSameBlock(pre_fifo[bank].front()->ADDR_)) {
											//	//	Bus_Tran tran;
											//	//	tran.addr = uint64_t(pre_fifo[bank].front()->ADDR_ << 2);
											//	//	tran.rdwr = 0;
											//	//	tran.cycle = 0;
											//	//	bus.push_back(tran);
											//	//}
											//	addpoped_addr(pre_fifo[bank].front()->ADDR_, true);
											//}
											post_offset[i][j].valid = true;
											post_offset[i][j].offset = pre_fifo[bank].front()->offset;
											sended = true;
											break;
										}
									}
									if (sended)
										break;
								}
							}
						}
						else {
							for (int i = 0; i < post_table.size(); i++) {
								if (!post_table[i]->valid) {
									bool blank = true;
									for (short j = 0; j < CACHE_LINE_SIZE; j++)
									{
										if (post_offset[i][j].valid)
											blank = false;
									}
									if (blank)
									{
										post_table[i]->ADDR_ = pre_fifo[bank].front()->ADDR_;
										post_table[i]->complete = pre_fifo[bank].front()->complete;
										post_table[i]->offset = pre_fifo[bank].front()->offset;
										//				post_table[i]->pe_round = line->pe_round;
										post_table[i]->pe_tag = pre_fifo[bank].front()->pe_tag;
										post_table[i]->rdwr = pre_fifo[bank].front()->rdwr;
										post_table[i]->bypass = pre_fifo[bank].front()->bypass;
										post_table[i]->TAG_ = pre_fifo[bank].front()->TAG_;
										post_table[i]->transid = pre_fifo[bank].front()->transid;
										post_table[i]->valid = pre_fifo[bank].front()->valid;
										post_table[i]->vec = pre_fifo[bank].front()->vec;
										if (!system_parameter.bus_enable)
										{
											mem->addTransaction(false, uint64_t(pre_fifo[bank].front()->ADDR_) << 2);    //一个字4字节，所以左移2位
											addpoped_addr(pre_fifo[bank].front()->ADDR_, true);
										}
										else if (system_parameter.bus_enable)
										{
											bus_tran tran;
											tran.addr = uint64_t(pre_fifo[bank].front()->ADDR_ << 2);
											tran.rdwr = 0;
											tran.cycle = 0;
											bus.push_back(tran);
											addpoped_addr(pre_fifo[bank].front()->ADDR_, true);
										}
										sended = true;
										break;
									}
								}
							}
						}
						if (sended) {
							delete pre_fifo[bank].front();
							pre_fifo[bank].erase(pre_fifo[bank].begin());
						}
						else {
							pre_fifo[bank].push_back(pre_fifo[bank].front());
							pre_fifo[bank].erase(pre_fifo[bank].begin());
						}
					}
				}
				else if (pre_fifo[bank].front()->valid && pre_fifo[bank].front()->pref && !pre_fifo[bank].front()->rdwr)       //pref
				{
					if (NotSameBlock(pre_fifo[bank].front()->ADDR_, pre_fifo[bank].front()->bypass))///////////////////////////////非同一block?????/////////
					{
						if (!inflight_reg[bank]->valid && MSHR_STATE != 3)
						{
							delete inflight_reg[bank];
							inflight_reg[bank] = pre_fifo[bank].front();                                                    //正在访问cache的请求进入inflight_reg
							if (!cache->addtranscation(pre_fifo[bank].front()->ADDR_, pre_fifo[bank].front()->rdwr,bank))
							{
								PRINTERRORM("error occurs for cache is busy when add trans!");
								system("pause");
							}
							//poped_addr.push_back(pre_fifo.front()->ADDR_);
							pre_fifo[bank].erase(pre_fifo[bank].begin());

							if (print_enable)
								PRINTM("PRF transcation send to cache from table ");
						}
						else
						{
							if (print_enable)
							{
								PRINTM("inflight_reg blocking!");
							}
						}
					}
					else     //当有同块正在访存时预取无意义，直接reset
					{
						pre_fifo[bank].erase(pre_fifo[bank].begin());
						invalid_pref++;
					}
				}
			}
		}
		//被阻塞的inflight请求更新！
		if (inflight_reg[bank]->valid && inflight_reg[bank]->complete)
		{                                                        //当inflight请求阻塞且完成时，检测MSHR的next_index,若不冲突，则将阻塞的inflight请求完成
			if (!inflight_reg[bank]->pref)                       //非预取
			{
				for (short j = 0; j < CACHE_LINE_SIZE; j++)
				{
					if (inflight_reg[bank]->offset[j].valid && !channel_occupy[inflight_reg[bank]->offset[j].pointer])
					{
						//cbk_addr在步长为1时才有效，用于debug
						(arbitrator->ArbitratorLines[inflight_reg[bank]->offset[j].pointer]->lse_)->LSEcallback(inflight_reg[bank]->ADDR_ + j, ClockCycle, inflight_reg[bank]->offset[j].tag);
						if(print_enable)
							PRINTM("return inflight_reg data to " << arbitrator->ArbitratorLines[inflight_reg[bank]->offset[j].pointer]->lse_->attribution->index << " " << inflight_reg[bank]->ADDR_ + j);
						inflight_reg[bank]->offset[j].valid = 0;
						channel_occupy[inflight_reg[bank]->offset[j].pointer] = 1;
					}
				}
				//release_poped_addr(inflight_reg->ADDR_);
				inflight_reg[bank]->complete = 1;
				bool remain = 0;
				for (short j = 0; j < CACHE_LINE_SIZE; j++)
				{
					if (inflight_reg[bank]->offset[j].valid)
						remain = 1;
				}
				if (!remain)
					inflight_reg[bank]->reset();
			}
			else if (inflight_reg[bank]->pref)
			{
				inflight_reg[bank]->reset();
			}
		}

		bank++;
		if (bank == CACHE_BANK)
			bank = 0;
	}

	bank_round++;
	if (bank_round == CACHE_BANK)
		bank_round = 0;

	//update the table-fifo and send trans from arbitrator to table	                    第2步
	//uint32_t cyclepointer = arbitrator->pointer;
	uint32_t cyclepointer = ClockCycle % arbitrator->ArbitratorLines.size();
	arbitrator->pointer = cyclepointer;
	uint32_t blank_tabline = system_parameter.fifoline_num - pre_fifo.size();////////////////////20-8??????????????//////////////
//	uint32_t in_num = blank_tabline < system_parameter.in_num ? blank_tabline : system_parameter.in_num;       //由于vec请求会产生多项，不一定准
	uint32_t in_num = system_parameter.in_num;






	list<int> p_order;
	while (1)                           //寻找合适数量的trans进入table
	{
		if (arbitrator->ArbitratorLines[arbitrator->pointer]->valid == 1&&(!IsRTran(arbitrator->pointer)))
		{
			uint seek_index = 0;
			if ((seek_index=IsVecTran(arbitrator->pointer)) != -1) {
				if (p_order.size() < in_num)
				{
					bool can_add = true;
					for (uint this_pointer = arbitrator->pointer; this_pointer < arbitrator->pointer + vec_pointer[seek_index].size; this_pointer++)
					{
						if (arbitrator->ArbitratorLines[this_pointer]->valid != 1) {
							can_add = false;
						}
					}
					if(can_add)
						p_order.push_back(arbitrator->pointer);
				}
				else if (p_order.size() == in_num)                              //pointer停在上次满足入口要求后的地方！
					break;
			}
			else {
				if (p_order.size() < in_num)
				{
					p_order.push_back(arbitrator->pointer);
				}
				else if (p_order.size() == in_num)                              //pointer停在上次满足入口要求后的地方！
					break;
			}
		}
		if (arbitrator->pointer < (arbitrator->ArbitratorLines.size() - 1))
			arbitrator->pointer++;
		else if (arbitrator->pointer == (arbitrator->ArbitratorLines.size() - 1))
			arbitrator->pointer = 0;
		if (arbitrator->pointer == cyclepointer)                             //扫描完一遍，结束
			break;
	}
	if (p_order.size() == 0 && print_enable)
		PRINTM("no trans moved to table from ArbitratorLine");



	set<short> exist_banks;
	while (!p_order.empty())  //table满不能阻止请求进入v_table              
	{
		int index = p_order.front();
		int vec_index;
		uint32_t addr = arbitrator->ArbitratorLines[index]->ADDR_;
		if (arbitrator->ArbitratorLines[index]->valid&& arbitrator->ArbitratorLines[index]->rdwr&& arbitrator->ArbitratorLines[index]->bypass) {
			//if (!system_parameter.bus_enable)
			//{
			//	mem->addTransaction(true, uint64_t(arbitrator->ArbitratorLines[index]->ADDR_) << 2);    //一个字4字节，所以左移2位
			//	//addpoped_addr(pre_fifo[bank].front()->ADDR_, true);
			//}
			//else
			//{
			//	bus_tran tran;
			//	tran.addr = uint64_t(arbitrator->ArbitratorLines[index]->ADDR_ << 2);
			//	tran.rdwr = true;
			//	tran.cycle = 0;
			//	bus.push_back(tran);
			//	//addpoped_addr(pre_fifo[bank].front()->ADDR_, true);
			//}
			arbitrator->ArbitratorLines[index]->returnACK();
			arbitrator->ArbitratorLines[index]->valid=false;
		}
		else {
			short bank;
			if (system_parameter.cache_mode) {
				bank = (addr & BANK_BITS) >> ADDR_BANK;
			}
			else {
				bank = seek_bank();
			}
			//set<short> exist_banks;
			if (IsPrefTran(arbitrator->ArbitratorLines[index]->TAG_))
			{
				if (pre_fifo[bank].size() < system_parameter.fifoline_num)
				{
					bool same_block = 0;
					{
						if (last_pref.size())
						{
							for (list<uint>::iterator it = last_pref.begin(); it != last_pref.end(); it++)
							{
								if (*it / CACHE_LINE_SIZE == arbitrator->ArbitratorLines[index]->ADDR_ / CACHE_LINE_SIZE)
								{
									same_block = 1;
									break;
								}
							}
						}
					}
					if (same_block)
					{
						//cachefile << "same block pref happend in " << ClockCycle << endl;
					}
					else
					{
						if (system_parameter.fifoline_num > pre_fifo[bank].size())                       //由于有vec请求，需要先检测是否满了
						{
							TabLine* new_line = new TabLine();
							new_line->ADDR_ = arbitrator->ArbitratorLines[index]->ADDR_;
							new_line->rdwr = arbitrator->ArbitratorLines[index]->rdwr;
							new_line->valid = 1;
							new_line->pref = 1;
							new_line->pe_tag = arbitrator->ArbitratorLines[index]->pe_tag;
							new_line->TAG_ = arbitrator->ArbitratorLines[index]->TAG_;
							new_line->bypass = arbitrator->ArbitratorLines[index]->bypass;
							//							new_line->pe_round = arbitrator->ArbitratorLines[index]->pe_round;

							new_line->transid = transid;                          //这两句绑死，每个请求拥有唯一的transid标识！
							transid++;                                            //只有一开始table入数的时候赋值	

							pre_fifo[bank].push_back(new_line);

							if (last_pref.size() && last_pref.size() != pref_history)
							{
								last_pref.push_back(new_line->ADDR_);                //pref_table记录正在table中的预取请求还是记录所有已有的预取请求？
							}
							else if (!last_pref.size())       //第一次pref时触发
							{
								last_pref.push_back(new_line->ADDR_);
							}
							else if (last_pref.size() == pref_history)
							{
								last_pref.pop_front();
								last_pref.push_back(new_line->ADDR_);
							}
							else
							{
								PRINTM("error occurs in last_pref!");
							}

							if (print_enable)
								PRINTM("pref send from arbitratorline " << index << " to fifo!");
						}
					}

					arbitrator->ArbitratorLines[index]->returnACK();     //take the trans and return ACK to LE
					arbitrator->ArbitratorLines[index]->valid = 0;
				}
			}
			else if ((vec_index = IsVecTran(arbitrator->ArbitratorLines[index]->TAG_)) != -1)
			{
				int step = vec_pointer[vec_index].step;
				int size = vec_pointer[vec_index].size;
				short pointer = vec_pointer[vec_index].pointer;

				uint32_t max_addr = arbitrator->ArbitratorLines[index]->ADDR_ + step * size;
				short trans_num = max_addr / CACHE_LINE_SIZE - arbitrator->ArbitratorLines[index]->ADDR_ / CACHE_LINE_SIZE + 1;

				//检测fifo是否放得下
				int bank_index = (arbitrator->ArbitratorLines[index]->ADDR_ & BANK_BITS) >> ADDR_BANK;
				int trannum[CACHE_BANK] = { 0 };
				bool size_ok = 1;
				if (system_parameter.cache_mode) {
					for (int num = 0; num < trans_num; num++)
					{
						if (bank_index == CACHE_BANK)
							bank_index = 0;
						trannum[bank_index]++;
						bank_index++;
					}
					for (int bankind = 0; bankind < CACHE_BANK; bankind++)
					{
						if (system_parameter.fifoline_num - pre_fifo[bankind].size() < trannum[bankind])
							size_ok = 0;
					}
				}
				else {
					if (!receive_enable(trans_num))
					{
						size_ok = false;
					}
				}

				if (size_ok)          //有空
				{
					for (int k = 0; k < size; k++) {
						arbitrator->ArbitratorLines[index + k]->returnACK();     //take the trans and return ACK to LE
						arbitrator->ArbitratorLines[index + k]->valid = 0;
					}
					uint head_addr;                                             //head_addr表示每条合并请求的头部地址
					uint fir_addr = arbitrator->ArbitratorLines[index]->ADDR_;  //fir_addr表示每个VLE请求的基地址

					TabLine* new_line = new TabLine();
					short mask1 = fir_addr % CACHE_LINE_SIZE;
					new_line->offset[mask1].valid = 1;
					new_line->offset[mask1].pointer = arbitrator->ArbitratorLines[index]->TAG_;
					new_line->offset[mask1].tag = arbitrator->ArbitratorLines[index]->pe_tag;

					new_line->ADDR_ = fir_addr - fir_addr % CACHE_LINE_SIZE;
					new_line->TAG_ = arbitrator->ArbitratorLines[index]->TAG_;
					new_line->bypass = arbitrator->ArbitratorLines[index]->bypass;
					new_line->pe_tag = arbitrator->ArbitratorLines[index]->pe_tag;
					new_line->rdwr = arbitrator->ArbitratorLines[index]->rdwr;
					new_line->valid = 1;
					new_line->pref = 0;
					new_line->vec = 1;

					new_line->transid = transid;                          //这两句绑死，每个请求拥有唯一的transid标识！
					transid++;

					head_addr = fir_addr;

					for (int j = 1; j < size; j++)
					{
						uint addr;
						short mask2;
						addr = fir_addr + j * step;
						if (addr / CACHE_LINE_SIZE == head_addr / CACHE_LINE_SIZE)
						{
							mask2 = addr % CACHE_LINE_SIZE;
							new_line->offset[mask2].valid = 1;
							new_line->offset[mask2].pointer = arbitrator->ArbitratorLines[index]->TAG_ + j;
							new_line->offset[mask2].tag = arbitrator->ArbitratorLines[index]->pe_tag;
						}
						else
						{
							pre_fifo[bank].push_back(new_line);
							if (!new_line->rdwr) {
								//	exist_banks.insert(bank);
								if (exist_banks.find(bank) != exist_banks.end())
									conflict_times++;
								exist_banks.insert(bank);
								add_times++;
							}
							if (system_parameter.cache_mode) {
								bank = (addr & BANK_BITS) >> ADDR_BANK;
							}
							else {
								bank = seek_bank();
							}
							new_line = new TabLine();
							head_addr = addr;

							mask2 = addr % CACHE_LINE_SIZE;
							new_line->offset[mask2].pointer = arbitrator->ArbitratorLines[index]->TAG_ + j;
							new_line->offset[mask2].tag = arbitrator->ArbitratorLines[index]->pe_tag;
							new_line->offset[mask2].valid = 1;

							new_line->ADDR_ = addr - addr % CACHE_LINE_SIZE;
							new_line->TAG_ = arbitrator->ArbitratorLines[index]->TAG_;
							new_line->bypass = arbitrator->ArbitratorLines[index]->bypass;
							//							new_line->pe_round = arbitrator->ArbitratorLines[index]->pe_round;
							new_line->pe_tag = arbitrator->ArbitratorLines[index]->pe_tag;
							new_line->rdwr = arbitrator->ArbitratorLines[index]->rdwr;
							new_line->valid = 1;
							new_line->pref = 0;
							new_line->vec = 1;

							new_line->transid = transid;                          //这两句绑死，每个请求拥有唯一的transid标识！
							transid++;
						}
					}
					pre_fifo[bank].push_back(new_line);
					if (!new_line->rdwr) {
						//exist_banks.insert(bank);
						if (exist_banks.find(bank) != exist_banks.end())
							conflict_times++;
						exist_banks.insert(bank);
						add_times++;//跳出循环后还有最后一条
					}
					if (print_enable)
						PRINTM("vec_tran send from arbitratorline " << index << " to fifo ");
				}
			}
			else if (IsRTran(arbitrator->ArbitratorLines[index]->TAG_)) { DEBUG_ASSERT(false); }
			else
			{
				if (system_parameter.fifoline_num > pre_fifo[bank].size())
				{
					TabLine* new_line = new TabLine();
					short addr_offset = arbitrator->ArbitratorLines[index]->ADDR_ % CACHE_LINE_SIZE;
					new_line->ADDR_ = arbitrator->ArbitratorLines[index]->ADDR_ - addr_offset;
					new_line->rdwr = arbitrator->ArbitratorLines[index]->rdwr;
					new_line->valid = 1;
					new_line->pref = arbitrator->ArbitratorLines[index]->pref;
					new_line->pe_tag = arbitrator->ArbitratorLines[index]->pe_tag;
					new_line->bypass = arbitrator->ArbitratorLines[index]->bypass;
					new_line->TAG_ = arbitrator->ArbitratorLines[index]->TAG_;
					//					new_line->pe_round = arbitrator->ArbitratorLines[index]->pe_round;

					new_line->offset[addr_offset].valid = 1;
					new_line->offset[addr_offset].pointer = arbitrator->ArbitratorLines[index]->TAG_;
					new_line->offset[addr_offset].tag = arbitrator->ArbitratorLines[index]->pe_tag;

					new_line->transid = transid;                          //这两句绑死，每个请求拥有唯一的transid标识！
					transid++;                                            //只有一开始table入数的时候赋值

					pre_fifo[bank].push_back(new_line);
					if (!new_line->rdwr) {
						//	exist_banks.insert(bank);
						if (exist_banks.find(bank) != exist_banks.end())
							conflict_times++;
						exist_banks.insert(bank);
						add_times++;
					}

					arbitrator->ArbitratorLines[index]->returnACK();     //take the data and return ACK to LE/SE
					arbitrator->ArbitratorLines[index]->valid = 0;

					if (print_enable)
						PRINTM("trans send from arbitratorline " << index << " to fifo!");
				}
			}
		}

		p_order.pop_front();
	}



		if (!p_order.empty())
		{
			PRINTM("error occurs for !p_order.empty() after step3!");
			cachefile << "error occurs for !p_order.empty() after step3!" << endl;
		}

		if (print_enable)
		{
			PRINTM("conflict times: "<<conflict_times<<" total times: "<< add_times)
			PRINTM("MSHR_STATE = " << MSHR_STATE);
			if (MSHR_STATE == 1)
				//cout << "finishing the post_table " << miss_index << endl;

			PRINTM("------------FIFO-------------" );
			for (int bank = 0; bank < CACHE_BANK; bank++)
			{
				PRINTM("-----BANK " << bank << "-----");
				for (uint32_t i = 0; i < pre_fifo[bank].size(); i++)
				{
					PRINTM("fifo " << i << " transid " << pre_fifo[bank][i]->transid << " valid =" << pre_fifo[bank][i]->valid << " addr "
						<< "=" << pre_fifo[bank][i]->ADDR_ <<" offset "<< pre_fifo[bank][i]->offset[0].valid<<" "<< pre_fifo[bank][i]->offset[0].pointer<<" "<< pre_fifo[bank][i]->offset[0].tag
						<< "  " << pre_fifo[bank][i]->offset[1].valid << " " << pre_fifo[bank][i]->offset[1].pointer << " " << pre_fifo[bank][i]->offset[1].tag
						<< "  " << pre_fifo[bank][i]->offset[2].valid << " " << pre_fifo[bank][i]->offset[2].pointer << " " << pre_fifo[bank][i]->offset[2].tag
						<< "  " << pre_fifo[bank][i]->offset[3].valid << " " << pre_fifo[bank][i]->offset[3].pointer << " " << pre_fifo[bank][i]->offset[3].tag
						<< "  " << pre_fifo[bank][i]->offset[4].valid << " " << pre_fifo[bank][i]->offset[4].pointer << " " << pre_fifo[bank][i]->offset[4].tag
						<< "  " << pre_fifo[bank][i]->offset[5].valid << " " << pre_fifo[bank][i]->offset[5].pointer << " " << pre_fifo[bank][i]->offset[5].tag
						<< "  " << pre_fifo[bank][i]->offset[6].valid << " " << pre_fifo[bank][i]->offset[6].pointer << " " << pre_fifo[bank][i]->offset[6].tag
						<< "  " << pre_fifo[bank][i]->offset[7].valid << " " << pre_fifo[bank][i]->offset[7].pointer << " " << pre_fifo[bank][i]->offset[7].tag
						<< " rdwr " << "=" << pre_fifo[bank][i]->rdwr << " pointer = " << pre_fifo[bank][i]->TAG_ << " pref = " << pre_fifo[bank][i]->pref);
				}
			}

			PRINTM("------------INFLIGHT-------------" );
			for (int bank = 0; bank < CACHE_BANK; bank++)
				PRINTM("reg " << bank << " transid " << inflight_reg[bank]->transid << " valid =" << inflight_reg[bank]->valid << " addr "
				<< "=" << inflight_reg[bank]->ADDR_ << " pref = " << inflight_reg[bank]->pref);


			PRINTM(endl << "------------TABLE-------------");
			for (uint32_t i = 0; i < post_table.size(); i++)
			{
				PRINTMN("table " << i << " transid " << post_table[i]->transid << " valid =" << post_table[i]->valid << " addr "
					<< "=" << post_table[i]->ADDR_ << " rdwr " << "=" << post_table[i]->rdwr << " complete = " << post_table[i]->complete);
				PRINTMN(" offset valid = ");
				for (int j = 0; j < system_parameter.post_offset_depth; j++)
				{
					PRINTMN(post_offset[i][j].valid << " ");
				}
				PRINTMN(endl);
			}

			PRINTMN("poped_addr size = " << poped_addr.size() << "; ");
			for (auto it = poped_addr.begin(); it != poped_addr.end(); it++)
			{
				PRINTMN((*it).first<<"_"<< static_cast<int>((*it).second)<< " ");
			}
			PRINTMN(endl);

			PRINTMN("WB_table size = " << cache->WB_table.size() << "; ");
			for (vector<uint32_t>::iterator it = cache->WB_table.begin(); it < cache->WB_table.end(); it++)
			{
				PRINTMN(*it << " ");
			}
			PRINTMN(endl);

			PRINTM("miss_finished_cnter = " << miss_finished_cnter );

			PRINTMN("----------" << ClockCycle << "CYCLE END----------");
			PRINTM(endl);
		}

		if (profiling)
		{
			double fdepth = system_parameter.fifoline_num;

			for (int i = 0; i < CACHE_BANK; i++)
				lsu_fifo += pre_fifo[i].size() / (fdepth * CACHE_BANK);

			int mshr_size = 0;
			double mshr_rate;
			for (int i = 0; i < system_parameter.tabline_num; i++)
			{
				if (post_table[i]->valid)
					mshr_size++;
			}
			double bdepth = system_parameter.tabline_num;
			mshr_rate = mshr_size / bdepth;
			lsu_mshr += mshr_rate;

			//max
			for (int i = 0; i < CACHE_BANK; i++)
			{
				if (pre_fifo[i].size() > lsu_fifo_max)
					lsu_fifo_max = pre_fifo[i].size();
			}
			if (mshr_size > lsu_mshr_max)
				lsu_mshr_max = mshr_size;
		}

}


bool Lsu::NotSameBlock(uint32_t addr_,bool bypass)
{
	for (auto iter = poped_addr.begin(); iter != poped_addr.end(); ++iter)
	{
		if (int(addr_ / CACHE_LINE_SIZE) == int((*iter).first / CACHE_LINE_SIZE)) {
			if(bypass&&((*iter).second==Simulator::RecallMode::both|| (*iter).second == Simulator::RecallMode::nocache))
				return 0;
			if (!bypass && ((*iter).second == Simulator::RecallMode::both || (*iter).second == Simulator::RecallMode::cached))
				return 0;
		}
	}
	return 1;
}
bool Lsu::NotSameBlock(uint32_t addr_)
{
	for (auto iter = poped_addr.begin(); iter != poped_addr.end(); ++iter)
	{
		if (int(addr_ / CACHE_LINE_SIZE) == int((*iter).first / CACHE_LINE_SIZE)) {
				return 0;
		}
	}
	return 1;
}
void Lsu::read_hit_complete(uint32_t addr,uint32_t i)
{
	short bank;
	if (system_parameter.cache_mode) {
		bank = (addr & BANK_BITS) >> ADDR_BANK;
	}
	else {
		bank = i;
	}
	if (inflight_reg[bank]->ADDR_ == addr && inflight_reg[bank]->valid && !inflight_reg[bank]->rdwr)
	{
		if (!inflight_reg[bank]->pref)
		{
			for (short j = 0; j < CACHE_LINE_SIZE; j++)
			{
				if (inflight_reg[bank]->offset[j].valid && !channel_occupy[inflight_reg[bank]->offset[j].pointer])
				{
					//cbk_addr在步长为1时才有效，用于debug
					(arbitrator->ArbitratorLines[inflight_reg[bank]->offset[j].pointer]->lse_)->LSEcallback(inflight_reg[bank]->ADDR_ + j, ClockCycle, inflight_reg[bank]->pe_tag);
					if (print_enable)
					{
						PRINTM("return hit data to " << arbitrator->ArbitratorLines[inflight_reg[bank]->offset[j].pointer]->lse_->attribution->index <<" "<< inflight_reg[bank]->ADDR_ + j );
					}
					inflight_reg[bank]->offset[j].valid = 0;
					channel_occupy[inflight_reg[bank]->offset[j].pointer] = 1;
				}
			}

			inflight_reg[bank]->complete = 1;
			bool remain = 0;
			for (short j = 0; j < CACHE_LINE_SIZE; j++)
			{
				if (inflight_reg[bank]->offset[j].valid)
					remain = 1;
			}
			if (!remain)
				inflight_reg[bank]->reset();
		}
		else if (inflight_reg[bank]->pref)
		{
			inflight_reg[bank]->reset();
			//统计无效的预取
			invalid_pref++;
		}
		//release_poped_addr(addr);
	}
	else
	{
		PRINTERRORM("error occurs for that readhit trans and inflight reg dont match!");
		system("pause");
	}
}

void Lsu::write_hit_complete(uint32_t addr, uint32_t i)
{
	short bank;
	if (system_parameter.cache_mode) {
		bank = (addr & BANK_BITS) >> ADDR_BANK;
	}
	else {
		bank = i;
	}
	if (inflight_reg[bank]->ADDR_ == addr && inflight_reg[bank]->valid && inflight_reg[bank]->rdwr)
	{
		//release_poped_addr(addr);
		inflight_reg[bank]->reset();
	}
	else
	{
		PRINTERRORM("error occurs for that writehit trans and inflight reg dont match!");
		system("pause");
	}
}

void Lsu::read_miss_complete(uint32_t addr)    //被动式的miss，通知MSHR中的同块请求去访问cache!
{
	uint32_t temp1 = 0;

	for (uint32_t i = 0; i < system_parameter.tabline_num; i++)
	{
		if (addr/CACHE_LINE_SIZE == post_table[i]->ADDR_ / CACHE_LINE_SIZE && !post_table[i]->rdwr && post_table[i]->valid == 1 && post_table[i]->complete == 0)
		{
			temp1++;
			post_table[i]->complete = 1;
			miss_finished_cnter++;

			switch (MSHR_STATE)
			{
			case 0:
				MSHR_STATE = 1;
				//next_miss_offset = post_table[i]->offset;
				miss_index = i;
				break;
			case 1:
				break;
			case 2:
				break;
			case 3:
				break;
			}
			break;
		}
	}
	if (temp1 < 1)
	{
		PRINTM("prf finished or error occurs in LSUnit::mem_read_complete() for no tableline finished when mem rdcallback ");
		PRINTM("returned addr is " << addr );
		release_poped_addr(addr);   //若未找到，说明为pref请求
	}
	//else if (temp1 > 1)
	//{
	//	PRINTM("error occurs in LSUnit::mem_read_complete() for more than one tableline finished when mem rdcallback ");
	//	cachefile << "error occurs in LSUnit::mem_read_complete() for more than one tableline finished when mem rdcallback " << endl;
	//}
}

void Lsu::write_miss_complete(uint32_t addr)
{
	//release_poped_addr(addr);
}

void Lsu::bus_update()
{
	if (system_parameter.bus_enable)
	{
		//bool out = 0;
		while (bus.size())
		{
			if (bus.front().cycle == system_parameter.bus_delay)
			{
				mem->addTransaction(bus.front().rdwr, bus.front().addr);
				bus.pop_front();

				if (profiling)
					dram_port += 1;
				//out = 1;
			}
			else if (bus.front().cycle > system_parameter.bus_delay)
			{
				PRINTERRORM ("error occurs in bus!");
				system("pause");
			}
			else if (bus.front().cycle < system_parameter.bus_delay)
			{
				break;
			}
		}

		for (list<Bus_Tran>::iterator it = bus.begin(); it != bus.end(); it++)
		{
			if ((*it).cycle >= system_parameter.bus_delay)
			{
				PRINTERRORM("error occurs in bus!" );
				system("pause");
			}
			else
				(*it).cycle++;
		}

		if (system_parameter.print_bus)
		{
			PRINTM("-------fifo-------");
			for (list<Bus_Tran>::iterator it = bus.begin(); it != bus.end(); it++)
			{
				PRINTM((*it).rdwr << " " << (*it).addr << " " << (*it).cycle);
			}
			PRINTMN(endl);
		}
	}
}


void Lsu::config_in(vector<int> config)
{
	config_reg = config;
	for (vector<int>::iterator it = config.begin() + 1; it != config.end(); it += 4)
	{
		if ((*it) == 0)
		{
			Vec_Msg msg;
			msg.pointer = *(it + 1);
			msg.step = *(it + 2);
			msg.size = *(it + 3);
			vec_pointer.push_back(msg);
		}
		else if ((*it) == 1)
		{
			pref_pointer.push_back(*(it + 1));
		}
		else if ((*it) == 2) {}
		else
		{
			PRINTERRORM("error occurs in void LSUnit::config_in!");
			std::system("pause");
		}
	}
}
void Lsu::config_in(map<uint, Simulator::Array::Loadstore_element*> lse_map)
{
	for (auto &i : lse_map)
	{
		if (i.second->getAttr()->vec_mode == Simulator::VecMode::vect) {
			Vec_Msg msg;
			msg.pointer = arbitrator->lse2relse[i.second->index];
			msg.step = i.second->getAttr()->step;
			msg.size = i.second->getAttr()->vec_size;
			vec_pointer.push_back(msg);
		}
		else if (i.second->getAttr()->vec_mode == Simulator::VecMode::vecr) {
			r_pointer.push_back(arbitrator->lse2relse[i.second->index]);
		}
		
	}
}

bool Lsu::IsPrefTran(uint32_t pointer)
{
	for (vector<uint>::iterator it = pref_pointer.begin(); it != pref_pointer.end(); it++)
	{
		if ((*it) == pointer)
			return true;
	}
	return false;
}

int Lsu::IsVecTran(uint32_t pointer)
{
	for (int i = 0; i < vec_pointer.size(); i++)
	{
		if (vec_pointer[i].pointer == pointer)
			return i;
	}
	return -1;
}

bool Lsu::IsRTran(uint32_t pointer)
{
	for (int i = 0; i < r_pointer.size(); i++)
	{
		if (r_pointer[i] == pointer)
			return true;
	}
	return false;
}

bool Lsu::addTrans2post(TabLine * line)
{
	for (short i = 0; i < system_parameter.tabline_num; i++)
	{
		bool blank = 1;
		if (!post_table[i]->valid && !post_table[i]->complete)
		{
			for (short j = 0; j < CACHE_LINE_SIZE; j++)
			{
				if (post_offset[i][j].valid)
					blank = 0;
			}
			if (blank)
			{
				post_table[i]->ADDR_ = line->ADDR_;
				post_table[i]->complete = line->complete;
				post_table[i]->offset = line->offset;
//				post_table[i]->pe_round = line->pe_round;
				post_table[i]->pe_tag = line->pe_tag;
				post_table[i]->rdwr = line->rdwr;
				post_table[i]->bypass = line->bypass;
				post_table[i]->TAG_ = line->TAG_;
				post_table[i]->transid = line->transid;
				post_table[i]->valid = line->valid;
				post_table[i]->vec = line->vec;
				return 1;
			}
		}
	}
	return 0;
}

bool Lsu::PostNotFull()
{
	for (short i = 0; i < system_parameter.tabline_num; i++)
	{
		if (!post_table[i]->valid)
		{
			return 1;
		}
	}
	return 0;
}

bool Lsu::addreserve2post(TabLine * line)
{
	for (short i = 0; i < system_parameter.tabline_num; i++)
	{
		if (post_table[i]->valid && (line->ADDR_) / CACHE_LINE_SIZE == (post_table[i]->ADDR_) / CACHE_LINE_SIZE&&line->bypass== post_table[i]->bypass)
		{
			for (short j = 0; j < system_parameter.post_offset_depth; j++)
			{
				if (!post_offset[i][j].valid)
				{
					post_offset[i][j].valid = 1;
//					post_offset[i][j].round = line->pe_round;
					post_offset[i][j].offset = line->offset;
					return 1;
				}

				//offset已满	
			}
		}
	}
	return 0;
}
uint Lsu::seek_bank()
{
	uint min_index = 0, min_value = pre_fifo[0].size();
	for (int bank = 0; bank < CACHE_BANK; bank++)
	{
		if (pre_fifo[bank].size() < min_value)
		{
			min_index = bank;
			min_value = pre_fifo[min_index].size();
		}
	}
	return min_index;
}
uint Lsu::receive_enable(uint transnum)
{
	uint total_empty = 0;;
	for (int bank = 0; bank < CACHE_BANK; bank++)
	{
		uint empty_entry = 0;
		empty_entry = system_parameter.fifoline_num - pre_fifo[bank].size();
		total_empty += empty_entry;
		if (total_empty >= transnum)
		{
			return true;
		}
	}
	return false;
}
/*
bool LSUnit::RegMshrConflict()                            //应使用按通道划分的offset来减少这一步的复杂度
{
	if (next_index_v)
	{
		if (next_index != -1)
		{
			for (short i = 0; i < CACHE_LINE_SIZE; i++)
			{
				for (short j = 0; j < CACHE_LINE_SIZE; j++)
				{
					if (post_offset[miss_index][next_index].offset[i].pointer == inflight_reg->offset[j].pointer)
					{
						return 1;
					}
				}
			}
			return 0;
		}
		else           //-1时下一个请求为primary请求
		{
			for (short i = 0; i < CACHE_LINE_SIZE; i++)
			{
				for (short j = 0; j < CACHE_LINE_SIZE; j++)
				{
					if (post_table[miss_index]->offset[i].pointer == inflight_reg->offset[j].pointer)
					{
						return 1;
					}
				}
			}
			return 0;
		}
	}
	else
		return 0;
}
*/
void Lsu::release_poped_addr(uint32_t addr)
{
	for (auto it = poped_addr.begin(); it != poped_addr.end(); it++)
	{
		if ((*it).first == addr)
		{
			poped_addr.erase(it);
			return;
		}
	}
	PRINTERRORM("error occurs in LSUnit::release_poped_addr! addr = " << addr );
//	system("pause");
}
}