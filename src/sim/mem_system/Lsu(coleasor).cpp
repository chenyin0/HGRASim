#include "Lsu.h"                                        
#include <vector>
#include "../Node/LSE.h"

//#define leNums    2  
//#define seNums    2

using namespace DRAMSim;

//extern bool print_screen;
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
		bypass = 0;
	}



	void TabLine::reset()
	{
		ADDR_ = 0;
		rdwr = false;
		valid = 0;
		pref = 0;
		transid = 0;
		complete = 0;
	}


	ArbitratorLine::ArbitratorLine(const Simulator::Preprocess::ArrayPara para, Simulator::Array::Loadstore_element * lse, uint32_t TAG) :Node(para)
	{
		lse_ = lse;
		//	se_ = lse;
		TAG_ = TAG;
		pe_tag = 0;
	//	pe_round = 0;
	}


	bool ArbitratorLine::AddTrans(Simulator::Array::Port_inout_lsu input, uint32_t TAG,bool bypass)
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
			cout << "error occurs in ArbitratorLine::returnACK()" << endl;
	}


	Arbitrator::Arbitrator(const Simulator::Preprocess::ArrayPara para, map<uint, Simulator::Array::Loadstore_element*> lse_map) :Node(para)                //out_num unused
	{
		//	lsize = leNums;                                    //as a pointer the return value of sizeof() is fixed as 8
		//	ssize = seNums;
		pointer = 0;
		uint relse_counter = 0;
		//std::map<uint, uint> lse2relse;

		for (uint32_t i = 0; i < lse_map.size(); i++)
		{
			if (lse_map[i]->getAttr()->ls_mode != Simulator::LSMode::dummy) {
				lse2relse[i] = relse_counter;
				ArbitratorLine* arbline = new ArbitratorLine(para, lse_map[i], relse_counter++);                       //������ArbitratorLines���±꼴ΪTAG����˼
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
	bool Arbitrator::AddTrans(Simulator::Array::Port_inout_lsu input, uint32_t lse_index,bool bypass)
	{
		if ((lse_index >= system_parameter.lse_num))
		{
			cout << "error occurs in Arbitrator:: AddTrans, the tag is " << lse_index << endl;
			return false;
		}
		else
			return ArbitratorLines[lse2relse[lse_index]]->AddTrans(input, lse2relse[lse_index],bypass);
	}


	Coleasor::Coleasor(const Simulator::Preprocess::ArrayPara para,Lsu* lsu):Node(para)
	{
		size = COLEASORSIZE;
		depth = COLEASORDEPTH;
		Coleasor_Line.resize(size);
		for (int i = 0; i < size; i++)
			Coleasor_Line[i].line.resize(depth);
		counter.resize(size);
		this->lsu = lsu;
	}

	Coleasor::~Coleasor(){}

	bool Coleasor::AddTrans(TabLine* line)
	{
		bool complete = 0;
		//find place to match
		for (short i = 0; i < size; i++)
		{
			if (Coleasor_Line[i].valid && (Coleasor_Line[i].addr / COLEASORDEPTH == line->ADDR_ / COLEASORDEPTH))
			{
				short offset = line->ADDR_ % COLEASORDEPTH;
				if (!Coleasor_Line[i].line[offset].valid)
				{
					Coleasor_Line[i].line[offset].valid = 1;
					Coleasor_Line[i].line[offset].pointer = line->TAG_;//如果不止一个要取这个地址的lse呢，就没有能加进去，complete就是false
					Coleasor_Line[i].line[offset].tag = line->pe_tag;
					complete = 1;
					break;
				}
			}
		}
		//find a new line
		if (!complete)
		{
			for (short i = 0; i < size; i++)
			{
				if (!Coleasor_Line[i].valid)
				{
					short offset = line->ADDR_ % COLEASORDEPTH;
					Coleasor_Line[i].addr = line->ADDR_ - offset;
					Coleasor_Line[i].valid = 1;
					Coleasor_Line[i].line[offset].valid = 1;
					Coleasor_Line[i].line[offset].pointer = line->TAG_;
					Coleasor_Line[i].line[offset].tag = line->pe_tag;
					complete = 1;
					break;
				}
			}
		}
		//try to kick out an old line
		if (!complete)
		{
			for (int i = 0; i < lsu->post_table.size(); i++)
			{
				if (!lsu->post_table[i]->valid)
				{
					short maxindex;
					int maxcount = 0;
					for (short j = 0; j < size; j++)
					{
						if (counter[j] > maxcount)
						{
							maxcount = counter[j];//找一个最大的counter
							maxindex = j;
						}
					}

					if (maxcount > KICKCOUNT)
					{
						if (!system_parameter.bus_enable)
						{
							lsu->mem->addTransaction(false, uint64_t(Coleasor_Line[maxindex].addr) << 2);    //一个字4字节，所以左移2位
							lsu->poped_addr.push_back(Coleasor_Line[maxindex].addr);
						}
						else if (system_parameter.bus_enable)
						{
							Bus_Tran tran;
							tran.addr = uint64_t(Coleasor_Line[maxindex].addr << 2);
							tran.rdwr = 0;
							tran.cycle = 0;
							lsu->bus.push_back(tran);
							lsu->poped_addr.push_back(Coleasor_Line[maxindex].addr);
						}

						//mem->addTransaction(false, uint64_t(Coleasor_Line[maxindex].addr) << 2);
						lsu->post_table[i]->valid = 1;
						lsu->post_table[i]->ADDR_ = Coleasor_Line[maxindex].addr;

						for (int k = 0; k < COLEASORDEPTH; k++)
						{
							lsu->post_offset[i][k].pointer = Coleasor_Line[maxindex].line[k].pointer;
							lsu->post_offset[i][k].tag = Coleasor_Line[maxindex].line[k].tag;
							lsu->post_offset[i][k].valid = Coleasor_Line[maxindex].line[k].valid;
						}
						reset(maxindex);
						complete = 1;

						short offset = line->ADDR_ % COLEASORDEPTH;
						Coleasor_Line[maxindex].addr = line->ADDR_ - offset;
						Coleasor_Line[maxindex].valid = 1;
						Coleasor_Line[maxindex].line[offset].valid = 1;
						Coleasor_Line[maxindex].line[offset].pointer = line->TAG_;
						Coleasor_Line[maxindex].line[offset].tag = line->pe_tag;
						break;
					}
				}
			}
		}

		if (!complete)
			return 0;
		else
			return 1;
	}

	void Coleasor::update()
	{
		bool sended = 0;
		for (int i = 0; i < size; i++)
		{
			if (coleasor_full(i))
			{
				for (int j = 0; j < lsu->post_table.size(); j++)
				{
					if (!lsu->post_table[j]->valid)
					{
						if (!system_parameter.bus_enable)
						{
							lsu->mem->addTransaction(false, uint64_t(Coleasor_Line[i].addr) << 2);    //一个字4字节，所以左移2位
							lsu->poped_addr.push_back(Coleasor_Line[i].addr);
						}
						else if (system_parameter.bus_enable)
						{
							Bus_Tran tran;
							tran.addr = uint64_t(Coleasor_Line[i].addr << 2);
							tran.rdwr = 0;
							tran.cycle = 0;
							lsu->bus.push_back(tran);
							lsu->poped_addr.push_back(Coleasor_Line[i].addr);
						}

						//mem->addTransaction(false, uint64_t(Coleasor_Line[i].addr) << 2);
						lsu->post_table[j]->valid = 1;
						lsu->post_table[j]->ADDR_ = Coleasor_Line[i].addr;
						for (int k = 0; k < COLEASORDEPTH; k++)
						{
							lsu->post_offset[j][k].pointer = Coleasor_Line[i].line[k].pointer;
							lsu->post_offset[j][k].tag = Coleasor_Line[i].line[k].tag;
							lsu->post_offset[j][k].valid = Coleasor_Line[i].line[k].valid;
						}
						reset(i);
						sended = 1;
						break;
					}
				}
				if (sended)
					break;
			}

			if (counter[i] >= SENDCOUNT && Coleasor_Line[i].valid)
			{
				for (int j = 0; j < lsu->post_table.size(); j++)
				{
					if (!lsu->post_table[j]->valid)
					{
						if (!system_parameter.bus_enable)
						{
							lsu->mem->addTransaction(false, uint64_t(Coleasor_Line[i].addr) << 2);    //一个字4字节，所以左移2位
							lsu->poped_addr.push_back(Coleasor_Line[i].addr);
						}
						else if (system_parameter.bus_enable)
						{
							Bus_Tran tran;
							tran.addr = uint64_t(Coleasor_Line[i].addr << 2);
							tran.rdwr = 0;
							tran.cycle = 0;
							lsu->bus.push_back(tran);
							lsu->poped_addr.push_back(Coleasor_Line[i].addr);
						}

						//mem->addTransaction(false, uint64_t(Coleasor_Line[i].addr) << 2);
						lsu->post_table[j]->valid = 1;
						lsu->post_table[j]->ADDR_ = Coleasor_Line[i].addr;
						for (int k = 0; k < system_parameter.post_offset_depth; k++)
						{
							lsu->post_offset[j][k].pointer = Coleasor_Line[i].line[k].pointer;
							lsu->post_offset[j][k].tag = Coleasor_Line[i].line[k].tag;
							lsu->post_offset[j][k].valid = Coleasor_Line[i].line[k].valid;
						}
						reset(i);
						sended = 1;
						break;
					}
				}
			}
			if (sended)
				break;
		}
		for (int i = 0; i < size; i++)
		{
			if (Coleasor_Line[i].valid && counter[i] < MAXCOUNT)
				counter[i]++;
		}
	}

	void Coleasor::print()
	{
		for (int bank = 0; bank < COLEASORSIZE; bank++)
		{
			cout << "-----line " << bank << "-----valid " << Coleasor_Line[bank].valid << "-----addr " << Coleasor_Line[bank].addr << "-----counter " << counter[bank] << endl;
			for (uint32_t i = 0; i < COLEASORDEPTH; i++)
				cout << Coleasor_Line[bank].line[i].valid << "_" << Coleasor_Line[bank].line[i].pointer << "_" << Coleasor_Line[bank].line[i].tag << " ";
			cout << endl;
		}
	}

	void Coleasor::reset(short line)
	{
		counter[line] = 0;
		Coleasor_Line[line].valid = 0;
		Coleasor_Line[line].addr = 0;
		for (int i = 0; i < COLEASORDEPTH; i++)
		{
			Coleasor_Line[line].line[i].valid = 0;
			Coleasor_Line[line].line[i].tag = 0;
			Coleasor_Line[line].line[i].pointer = 0;
		}
	}

	bool Coleasor::coleasor_full(short index)
	{
		for (short i = 0; i < COLEASORDEPTH; i++)
		{
			if (!Coleasor_Line[index].line[i].valid)
				return 0;
		}
		return 1;
	}

	Lsu::Lsu(const Simulator::Preprocess::ArrayPara para, map<uint, Simulator::Array::Loadstore_element*> lse_map) :Node(para)
	{
		cachefile.open("Cache.txt");                  //��¼�ô��cache��Ϣ
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
		delete coleasor;
	}


	bool Lsu::AddTrans(Simulator::Array::Port_inout_lsu input,uint TAG, bool bypass)
	{
//		return arbitrator->AddTrans(input.value_addr, TAG, input.valid, input.tag);
		return arbitrator->AddTrans(input, TAG, bypass);
	}


	void Lsu::AttachMem(DRAMSim::MultiChannelMemorySystem* memory)
	{
		mem = memory;
		coleasor = new Coleasor(system_parameter,this);
	}


	void Lsu::update()   //从后往前，并加入timing信息
	{
		memset(channel_occupy, 0, system_parameter.lse_num * sizeof(bool));
		ClockCycle++;
		mem->update();
		bus_update();
		//std::cout << "update!" << " " << ClockCycle << std::endl;
		//this_index = next_index;
		//this_index_v = next_index_v;

		cache->update();
		coleasor->update();


	// MSHR_FSM_achievable


    //software way used for sim; 
    //每个mshr line中都有256 bit用来keep完成的数据来保证不会发生重新miss
	switch (MSHR_STATE)
	{
		case 0:                            //空闲状态
			break;

		case 1:                            //块处理状态
		{
			bool sended = 0;                   //发过数的cycle不能变为状态0
			for (int index = 0; index < post_table.size(); index++)
			{
				if (post_table[index]->complete && (post_table[index]->valid))
				{
					for (int i = 0; i < system_parameter.post_offset_depth; i++)
					{					
						if (post_offset[index][i].valid)
						{
							bool reset = 1;
							if (post_offset[index][i].valid && !channel_occupy[post_offset[index][i].pointer])
							{
								(arbitrator->ArbitratorLines[post_offset[index][i].pointer]->lse_)->LSEcallback(post_table[index]->ADDR_ + i, ClockCycle, post_offset[index][i].tag);
								post_offset[index][i].valid = 0;
								channel_occupy[post_offset[index][i].pointer] = 1;          //不论是OOO还是DAE，都视为一个cycle只能接受一个数
								sended = 1;
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
				{
					bool NotEmpty = 0;
					for (short j = 0; j < system_parameter.post_offset_depth; j++)
					{
						if (post_offset[i][j].valid)
						{
							NotEmpty = 1;
							break;
						}
					}

					if (!NotEmpty)
					{
						miss_finished_cnter--;
						release_poped_addr(post_table[i]->ADDR_);
						post_table[i]->reset();
					}

					if (NotEmpty)
						AllEmpty = 0;
				}
			}
			//如果这周期完成过请求，为了避免和inflightreg冲突，状态不能变为状态0
			if (AllEmpty && !sended)
			{
				MSHR_STATE = 0;
			}
			break;
		}
	//完成inflight_reg中被阻塞的hit请求
	}

	//from fifo to cache/mem                       第1步:发送fifo顶部的请求
	//实现优先级的round robin
	short bank = bank_round;
	for (int times = 0; times < CACHE_BANK; times++)
	{
		if (pre_fifo[bank].size())
		{
			if (pre_fifo[bank].front()->rdwr && pre_fifo[bank].front()->valid)                  //写请求
			{
				if (!inflight_reg[bank]->valid && cache->trans_v[bank] == 0)
				{
					delete inflight_reg[bank];
					inflight_reg[bank] = pre_fifo[bank].front();                            //正在访问cache的请求进入inflight_reg
					if (!cache->addtranscation(pre_fifo[bank].front()->ADDR_, pre_fifo[bank].front()->rdwr))
					{
						cout << "error occurs for cache is busy when add trans!" << endl;
						system("pause");
					}
					//poped_addr.push_back(pre_fifo.front()->ADDR_);
					pre_fifo[bank].erase(pre_fifo[bank].begin());

					if (system_parameter.print_screen)
						cout << "WR transcation poped to cache from fifo " << endl;
				}
				else
				{
					if (system_parameter.print_screen)
					{
						cout << "inflight_reg blocking!" << endl;
					}
				}
			}
			else                                                             //读请求
			{
				if (pre_fifo[bank].front()->valid && !pre_fifo[bank].front()->pref && !pre_fifo[bank].front()->rdwr)
				{
					if (NotSameBlock((pre_fifo[bank].front()->ADDR_)))                                      //非reserve请求发送到cache和inflight_rag
					{
						if (!inflight_reg[bank]->valid && cache->trans_v[bank] == 0)
						{
							delete inflight_reg[bank];
							inflight_reg[bank] = pre_fifo[bank].front();                                          //正在访问cache的请求进入inflight_reg
							if (!cache->addtranscation(pre_fifo[bank][0]->ADDR_, pre_fifo[bank][0]->rdwr))
							{
								cout << "error occurs for cache is busy when add trans!" << endl;
								system("pause");
							}
							//poped_addr.push_back(pre_fifo[0]->ADDR_);
							pre_fifo[bank].erase(pre_fifo[bank].begin());
						}
						else                                                  //blocking的情况下不做操作
						{
							if (system_parameter.print_screen)
							{
								cout << "inflight_reg blocking!" << endl;
								//system("pause");
							}
						}
					}
					else                                                                               //reserve����ֱ�ӷ��͵�table������Ҫȥinflight_reg
					{
						if (addreserve2post(pre_fifo[bank][0]))
						{
							delete pre_fifo[bank].front();
							pre_fifo[bank].erase(pre_fifo[bank].begin());
						}
						else                                        //offset满,回到尾部
						{
							pre_fifo[bank].push_back(pre_fifo[bank].front());
							pre_fifo[bank].erase(pre_fifo[bank].begin());
						}
					}
				}
			}
		}
		//被阻塞的inflight请求更新！
		if (inflight_reg[bank]->valid && inflight_reg[bank]->complete)
		{                                                        //当inflight请求阻塞且完成时，检测MSHR的next_index,若不冲突，则将阻塞的inflight请求完成
			if (!channel_occupy[inflight_reg[bank]->TAG_])
			{
				//cbk_addr在步长为1时才有效，用于debug
				(arbitrator->ArbitratorLines[inflight_reg[bank]->TAG_]->lse_)->LSEcallback(inflight_reg[bank]->ADDR_, ClockCycle, inflight_reg[bank]->pe_tag);
				inflight_reg[bank]->valid = 0;
				channel_occupy[inflight_reg[bank]->TAG_] = 1;
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

	//update the table-fifo and send trans from arbitrator to table	                    ��2��
	//uint32_t cyclepointer = arbitrator->pointer;
	 uint32_t cyclepointer = ClockCycle % arbitrator->ArbitratorLines.size();
	 arbitrator->pointer = cyclepointer;
	uint32_t blank_tabline = system_parameter.fifoline_num - pre_fifo.size();////////////////////20-8??????????????//////////////
	uint32_t in_num = blank_tabline < system_parameter.in_num ? blank_tabline : system_parameter.in_num;       //����vec�������������һ��׼






	list<int> pchannel;
	while (1)                           //Ѱ�Һ���������trans����table
	{
		bool add_iter = false;
		if (arbitrator->ArbitratorLines[arbitrator->pointer]->valid == 1)
		{
			add_iter = true;
			if (pchannel.size() < in_num)
			{
				pchannel.push_back(arbitrator->pointer);
			}
			else if (pchannel.size() == in_num)                              //pointer停在上次满足入口要求后的地方！
				break;
		}
		if (arbitrator->pointer < (arbitrator->ArbitratorLines.size() - 1))
			arbitrator->pointer++;
		else if (arbitrator->pointer == (arbitrator->ArbitratorLines.size() - 1))
			arbitrator->pointer = 0;
		if (arbitrator->pointer == cyclepointer) {                            //扫描完一遍，结束
			break;
		}
	}
	if (pchannel.size() == 0 && system_parameter.print_screen)
		cout << "no trans moved to table from ArbitratorLine" << endl;

	while (!pchannel.empty())  //table满不能阻止请求进入v_table              
	{
		int index = pchannel.front();
		int vec_index;
		uint32_t addr = arbitrator->ArbitratorLines[index]->ADDR_;
		short bank = (addr & BANK_BITS) >> ADDR_BANK;
		if (system_parameter.fifoline_num > pre_fifo[bank].size())
		{
			TabLine* new_line = new TabLine();
			new_line->ADDR_ = arbitrator->ArbitratorLines[index]->ADDR_;
			new_line->rdwr = arbitrator->ArbitratorLines[index]->rdwr;
			new_line->valid = 1;
			new_line->pe_tag = arbitrator->ArbitratorLines[index]->pe_tag;
			new_line->TAG_ = arbitrator->ArbitratorLines[index]->TAG_;
			new_line->bypass = arbitrator->ArbitratorLines[index]->bypass;


			new_line->transid = transid;                          //这两句绑死，每个请求拥有唯一的transid标识！
			transid++;                                             //只有一开始table入数的时候赋值
			if (!new_line->bypass)
			{
				pre_fifo[bank].push_back(new_line);
				arbitrator->ArbitratorLines[index]->returnACK();     //take the data and return ACK to LE/SE
				arbitrator->ArbitratorLines[index]->valid = 0;
				if (system_parameter.print_screen)
					cout << "trans send from arbitratorline " << index <<"_"<< arbitrator->ArbitratorLines[index]->ADDR_  << "_" << arbitrator->ArbitratorLines[index]->pe_tag << " to fifo!" << endl;
			}
			else
			{   //这样实现对coleasor来说相当于一个cycle接收多个输入
				//物理上可能多通道对应多个小的coleasor进行实现
				if (coleasor->AddTrans(new_line))
				{
					//coleasor接收成功
					arbitrator->ArbitratorLines[index]->returnACK();     //take the data and return ACK to LE/SE
					arbitrator->ArbitratorLines[index]->valid = 0;
					if (system_parameter.print_screen)
						cout << "trans send from arbitratorline " << index << "_" << arbitrator->ArbitratorLines[index]->ADDR_  << "_" << arbitrator->ArbitratorLines[index]->pe_tag << " to fifo!" << endl;
				}
				else {
					if (system_parameter.print_screen)
						cout << " fail trans " << index << "_" << arbitrator->ArbitratorLines[index]->ADDR_  << "_" << arbitrator->ArbitratorLines[index]->pe_tag << " to fifo!" << endl;
				}
			}
		}
		else
		{
			if (system_parameter.print_screen)
				cout << " fail trans " << index << "_" << arbitrator->ArbitratorLines[index]->ADDR_  << "_" << arbitrator->ArbitratorLines[index]->pe_tag << " to fifo!" << endl;
		}
		pchannel.pop_front();
	}
	if (!pchannel.empty())
	{
		cout << "error occurs for !pchannel.empty() after step3!" << endl;
		cachefile << "error occurs for !pchannel.empty() after step3!" << endl;
	}

	if (system_parameter.print_screen)
	{
		cout << "MSHR_STATE = " << MSHR_STATE << endl;
		if (MSHR_STATE == 1)
			//cout << "finishing the post_table " << miss_index << endl;

		cout << "------------FIFO-------------" << endl;
		for (int bank = 0; bank < CACHE_BANK; bank++)
		{
			cout << "-----BANK " << bank << "-----"<< endl;
			for (uint32_t i = 0; i < pre_fifo[bank].size(); i++)
			{
				cout << "fifo " << i << " transid " << pre_fifo[bank][i]->transid << " valid =" << pre_fifo[bank][i]->valid << " addr "
				<< "=" << pre_fifo[bank][i]->ADDR_ << " rdwr " << "=" << pre_fifo[bank][i]->rdwr << " pointer = " << pre_fifo[bank][i]->TAG_ << " pref = " << pre_fifo[bank][i]->pref << endl;
			}
		}

		cout << "------------COLEASOR-------------" << endl;
		coleasor->print();

		cout << "------------INFLIGHT-------------" << endl;
		for (int bank = 0; bank < CACHE_BANK; bank++)
			cout << "reg " << bank << " transid " << inflight_reg[bank]->transid << " valid =" << inflight_reg[bank]->valid << " addr "
			<< "=" << inflight_reg[bank]->ADDR_ <<" complete= "<< inflight_reg[bank]->complete <<" pref = " << inflight_reg[bank]->pref << endl;


		cout << endl << "------------TABLE-------------" << endl;
		for (uint32_t i = 0; i < post_table.size(); i++)
		{
			cout << "table " << i << " transid " << post_table[i]->transid << " valid =" << post_table[i]->valid << " addr "
				<< "=" << post_table[i]->ADDR_ << " rdwr " << "=" << post_table[i]->rdwr << " complete = " << post_table[i]->complete;
			cout << " offset valid = ";
			for (int j = 0; j < system_parameter.post_offset_depth; j++)
			{
				cout << post_offset[i][j].valid << " ";
			}
			cout << endl;
		}

		cout << "poped_addr size = " << poped_addr.size() << "; ";
		for (vector<uint32_t>::iterator it = poped_addr.begin(); it < poped_addr.end(); it++)
		{
			cout << *it << " ";
		}
		cout << endl;

		cout << "WB_table size = " << cache->WB_table.size() << "; ";
		for (vector<uint32_t>::iterator it = cache->WB_table.begin(); it < cache->WB_table.end(); it++)
		{
			cout << *it << " ";
		}
		cout << endl;

		cout << "miss_finished_cnter = " << miss_finished_cnter << endl;

		cout << "----------" << ClockCycle << "CYCLE END----------" << endl;
		cout << endl;
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


bool Lsu::NotSameBlock(uint32_t addr_)
{
	for (vector<uint32_t>::iterator iter = poped_addr.begin(); iter != poped_addr.end(); ++iter)
	{
		if (int(addr_ / CACHE_LINE_SIZE) == int(*iter / CACHE_LINE_SIZE))
			return 0;
	}
	return 1;
}

void Lsu::read_hit_complete(uint32_t addr)
{
	short bank = (addr & BANK_BITS) >> ADDR_BANK;
	if (inflight_reg[bank]->ADDR_ == addr && inflight_reg[bank]->valid && !inflight_reg[bank]->rdwr)
	{
		for (short j = 0; j < CACHE_LINE_SIZE; j++)
		{
			if (inflight_reg[bank]->valid && !channel_occupy[inflight_reg[bank]->TAG_])
			{
				//cbk_addr在步长为1时才有效，用于debug
				(arbitrator->ArbitratorLines[inflight_reg[bank]->TAG_]->lse_)->LSEcallback(inflight_reg[bank]->ADDR_ + j, ClockCycle, inflight_reg[bank]->pe_tag);
				inflight_reg[bank]->valid = 0;
				channel_occupy[inflight_reg[bank]->TAG_] = 1;
			}
		}

		inflight_reg[bank]->complete = 1;//这个channel被占住了,暂时不能发出去,只能放在inflight_reg里面,inflight_reg里面还有数,下一个就进不来
		if (!inflight_reg[bank]->valid)
			inflight_reg[bank]->reset();
	}
	else
	{
		cout << "error occurs for that readhit trans and inflight reg dont match!" << endl;
		system("pause");
	}
}

void Lsu::write_hit_complete(uint32_t addr)
{
	short bank = (addr & BANK_BITS) >> ADDR_BANK;

	if (inflight_reg[bank]->ADDR_ == addr && inflight_reg[bank]->valid && inflight_reg[bank]->rdwr)
	{
		//release_poped_addr(addr);
		inflight_reg[bank]->reset();
	}
	else
	{
		cout << "error occurs for that writehit trans and inflight reg dont match!" << endl;
		system("pause");
	}
}

void Lsu::read_miss_complete(uint32_t addr)    //被动式的miss，通知MSHR中的同块请求去访问cache!
{
	uint32_t temp1 = 0;

	for (uint32_t i = 0; i < system_parameter.tabline_num; i++)
	{
		if ((addr / CACHE_LINE_SIZE) * CACHE_LINE_SIZE == post_table[i]->ADDR_ && post_table[i]->rdwr == 0 &&
			post_table[i]->valid == 1 && post_table[i]->complete == 0)
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
		cout << "prf finished or error occurs in Lsu::mem_read_complete() for no tableline finished when mem rdcallback at cycle"<<ClockCycle << endl;
		cout << "returned addr is " << addr << endl;
		release_poped_addr(addr);   //若未找到，说明为pref请求
	}
	else if (temp1 > 1)
	{
		cout << "error occurs in Lsu::mem_read_complete() for more than one tableline finished when mem rdcallback " << endl;
		cachefile << "error occurs in Lsu::mem_read_complete() for more than one tableline finished when mem rdcallback " << endl;
	}
}

void Lsu::write_miss_complete(uint32_t addr)
{
	release_poped_addr(addr);
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
				cout << "error occurs in bus!" << endl;
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
				cout << "error occurs in bus!" << endl;
				system("pause");
			}
			else
				(*it).cycle++;
		}

		if (system_parameter.print_bus)
		{
			cout << "-------fifo-------" << endl;
			for (list<Bus_Tran>::iterator it = bus.begin(); it != bus.end(); it++)
			{
				cout << (*it).rdwr << " " << (*it).addr << " " << (*it).cycle << endl;
			}
			cout << endl;
		}
	}
}

/*
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
			cout << "error occurs in void Lsu::config_in!" << endl;
			std::system("pause");
		}
	}
}
*/

bool Lsu::addTrans2post(TabLine* line)
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
				post_table[i]->ADDR_ = (line->ADDR_ / CACHE_LINE_SIZE) * CACHE_LINE_SIZE;
				post_table[i]->complete = line->complete;
				post_table[i]->valid = line->valid;
				post_offset[i][line->ADDR_- post_table[i]->ADDR_].pointer = line->TAG_;
				post_offset[i][line->ADDR_ - post_table[i]->ADDR_].tag = line->pe_tag;
				post_offset[i][line->ADDR_ - post_table[i]->ADDR_].valid = 1;
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
		if (post_table[i]->valid && (line->ADDR_)/CACHE_LINE_SIZE == (post_table[i]->ADDR_)/CACHE_LINE_SIZE)
		{
			short offset = line->ADDR_ - post_table[i]->ADDR_;
			if (!post_offset[i][offset].valid)
			{
				post_offset[i][offset].valid = 1;
				post_offset[i][offset].pointer = line->TAG_;
				post_offset[i][offset].tag = line->pe_tag;
				return 1;
			}
		}
	}
	return 0;
}


void Lsu::release_poped_addr(uint32_t addr)
{
	for (vector<uint32_t>::iterator it = poped_addr.begin(); it < poped_addr.end(); it++)
	{
		if ((*it / CACHE_LINE_SIZE) * CACHE_LINE_SIZE == (addr / CACHE_LINE_SIZE) * CACHE_LINE_SIZE)
		{
			poped_addr.erase(it);
			return;
		}
	}
	DEBUG_ASSERT(false);
	cout << "error occurs in LSUnit::release_poped_addr! addr = " << addr <<"at "<<ClockCycle<< endl;
	system("pause");
}
}