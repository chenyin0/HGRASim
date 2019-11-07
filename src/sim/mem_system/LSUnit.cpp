#include "LSUnit.h"                                        
#include <vector>

//#define leNums    2  
//#define seNums    2

using namespace DRAMSim;

extern bool PRINT_SCREEN;

TabLine::TabLine()
{
	//tabTAG_ = tabtag;
	ADDR_ = 0;
	rdwr = 0;
	valid = 0;
	pref = 0;
	transid = 0;
	pe_round = 0;
	vec = 0;
	complete = 0;
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
	rdwr = 0;
	valid = 0;
	pref = 0;
	transid = 0;
	complete = 0;
	for (int i = 0; i < CACHE_LINE_SIZE; i++)         //清小valid
		offset[i].valid = 0;

/*	for (int i = 0; i < OFFSET_DEPTH; i++)
		offset_4[i].valid = 0;*/
}


ArbitratorLine::ArbitratorLine(Simulator::Array::Loadstore_element* le, Simulator::Array::Loadstore_element* se, uint32_t TAG)
{
	le_ = le;
	se_ = se;
	TAG_ = TAG;
	pe_tag = 0;
	pe_round = 0;
}


bool ArbitratorLine::AddTrans(uint32_t ADDR, uint32_t TAG, uint32_t V, uint32_t PE_TAG, bool PE_ROUND)
{
	valid = V;
	ADDR_ = ADDR;
	pe_tag = PE_TAG;
	pe_round = PE_ROUND;
	TAG_ = TAG;
	pref = 0;
	return true;
}

void ArbitratorLine::returnACK()
{
	if (TAG_ < leNums)
		le_->callbackACK();    //pay attention to this name
	else if (TAG_ >= leNums && TAG_ < (leNums + seNums))
		se_->callbackACK();
	else
		cout << "error occurs in ArbitratorLine::returnACK()" << endl;
}


Arbitrator::Arbitrator(Simulator::Array::Loadstore_element** lep, Simulator::Array::Loadstore_element** sep)                 //out_num unused
{
	lsize = leNums;                                    //as a pointer the return value of sizeof() is fixed as 8
	ssize = seNums;
	pointer = 0;

	if (lsize != leNums || ssize != seNums)
	{
		cout << "error occurs for LE/SE size mismatch!" << "the size of lep is " << lsize << " the size of sep is ssize" << ssize << endl;
		exit(-1);
	}

	for (uint32_t i = 0; i < lsize; i++)
	{
		ArbitratorLine *arbline = new ArbitratorLine(lep[i], NULL, i);                       //隐含了ArbitratorLines的下标即为TAG的意思
		ArbitratorLines.push_back(arbline);
	}

	for (uint32_t i = 0; i < ssize; i++)
	{
		ArbitratorLine *arbline = new ArbitratorLine(NULL, sep[i], i + lsize);
		ArbitratorLines.push_back(arbline);
	}
}


Arbitrator::~Arbitrator()
{
	for (uint32_t i = 0; i < lsize; i++)
	{
		delete ArbitratorLines[i];
	}

	for (uint32_t i = 0; i < ssize; i++)
	{
		delete ArbitratorLines[lsize + i];
	}
}

//利用TAG寻找相应进行动作的tabline
bool Arbitrator::AddTrans(uint32_t ADDR, uint32_t TAG, uint32_t V, uint32_t PE_TAG, bool PE_ROUND)
{
	if ((TAG >= lsize + ssize))
	{
		cout << "error occurs in Arbitrator:: AddTrans, the tag is " << TAG << endl;
		return 0;
	}
	else
		return ArbitratorLines[TAG]->AddTrans(ADDR, TAG, V, PE_TAG, PE_ROUND);
}



LSUnit::LSUnit(Simulator::Array::Loadstore_element** lep, Simulator::Array::Loadstore_element** sep)
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

	Arbitrator *arb = new Arbitrator(lep, sep);
	arbitrator = arb;

	cache = new Cache();
	cache->attach(this);
	//pre_fifo.resize(FIFOLINE_NUM);
	post_table.resize(TABLINE_NUM);
	inflight_reg = new TabLine();

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

	post_offset.resize(TABLINE_NUM);
	for (int i = 0; i < TABLINE_NUM; i++)
		post_offset[i].resize(POST_OFFSET_DEPTH);
}


LSUnit::~LSUnit()
{
	delete arbitrator;
	delete cache;
	for (short i = 0; i < post_table.size(); i++)
	{
		delete post_table[i];
	}

}


bool LSUnit::AddTrans(uint32_t ADDR, uint32_t TAG, uint32_t V, uint32_t PE_TAG, bool PE_ROUND)
{
	return arbitrator->AddTrans(ADDR, TAG, V, PE_TAG, PE_ROUND);
}


void LSUnit::AttachMem(MultiChannelMemorySystem *memory)
{
	mem = memory;
}


void LSUnit::update()   //从后往前，并加入timing信息
{
	ClockCycle++;
	mem->update();
	bus_update();
	//std::cout << "update!" << " " << ClockCycle << std::endl;
	//this_index = next_index;
	//this_index_v = next_index_v;

	cache->update();


	// MSHR_FSM
	switch (MSHR_STATE)
	{
	case 0:                            //空闲状态
									   //
		break;

	case 1:                            //块处理状态
	{
		if (post_table[miss_index]->valid)
		{
			for (short i = 0; i < CACHE_LINE_SIZE; i++)
			{
				if (post_table[miss_index]->offset[i].valid)
				{
					(arbitrator->ArbitratorLines[post_table[miss_index]->offset[i].pointer]->le_)->LEcallback(post_table[miss_index]->ADDR_ + i, ClockCycle, post_table[miss_index]->offset[i].tag, post_table[miss_index]->pe_round);
					post_table[miss_index]->offset[i].valid = 0;
				}
			}
			post_table[miss_index]->valid = 0;
		}
		else
		{
			bool find_offset = 0;
			for (short i = 0; i < POST_OFFSET_DEPTH; i++)
			{
				if (post_offset[miss_index][i].valid)
				{
					find_offset = 1;
					for (short j = 0; j < CACHE_LINE_SIZE; j++)
					{
						if (post_offset[miss_index][i].offset[j].valid)
						{
							(arbitrator->ArbitratorLines[post_offset[miss_index][i].offset[j].pointer]->le_)->LEcallback(post_table[miss_index]->ADDR_ + j, ClockCycle, post_offset[miss_index][i].offset[j].tag, post_offset[miss_index][i].round);
							post_offset[miss_index][i].offset[j].valid = 0;
						}
					}
					post_offset[miss_index][i].valid = 0;
					break;
				}
			}


			if (!find_offset)   //根据计数器来改变状态！
			{
				miss_finished_cnter--;
				release_poped_addr(post_table[miss_index]->ADDR_);
				post_table[miss_index]->reset();

				if (miss_finished_cnter)
					MSHR_STATE = 2;
				else
					MSHR_STATE = 0;
			}
		}
		break;
	}

	case 2:                              //块搜索状态
	{
		for (short i = 0; i < TABLINE_NUM; i++)
		{
			if (post_table[i]->valid && post_table[i]->complete)
			{
				miss_index = i;
				MSHR_STATE = 3;
				//fifo_pause_signal = 1;        //通知fifo停止动作并在下一cycle进行访存
				break;
			}
		}
		if (MSHR_STATE != 3)
		{
			MSHR_STATE = 0;
			cout << "error occurs for cant find the finished block!" << endl;
			system("pause");
		}
		break;
	}

	case 3:                                      //块重访问状态
	{
		//此时需要占用cache读端口重新访存
		//状态变化情况由cache-miss还是hit决定
		if (!state3_reg)
		{
			if (post_table[miss_index]->valid && post_table[miss_index]->complete)   //primary trans complete first
			{
				if (!cache->addtranscation(post_table[miss_index]->ADDR_, post_table[miss_index]->rdwr))
				{
					cout << "error occurs for cache is busy when add trans!" << endl;
					system("pause");
				}                                                     //cache回复请求时也需要特殊对待
			}
			else
			{
				cout << "error occurs in state3_update!" << endl;
				system("pause");
			}
		}
		break;
	}
	//完成inflight_reg中被阻塞的hit请求
	}


	//from fifo to cache/mem                       第1步:发送fifo顶部的请求
	
	if (pre_fifo.size())
	{
		if (pre_fifo.front()->rdwr && pre_fifo.front()->valid)              //写请求
		{
			if (!inflight_reg->valid && cache->trans_v == 0 && MSHR_STATE != 3)
			{
				delete inflight_reg;
				inflight_reg = pre_fifo.front();                            //正在访问cache的请求进入inflight_reg
				if (!cache->addtranscation(pre_fifo.front()->ADDR_, pre_fifo.front()->rdwr))
				{
					cout << "error occurs for cache is busy when add trans!" << endl;
					system("pause");
				}
				//poped_addr.push_back(pre_fifo.front()->ADDR_);
				pre_fifo.erase(pre_fifo.begin());

				if (PRINT_SCREEN)
					cout << "WR transcation poped to cache from fifo " << endl;
			}
			else
			{
				if (PRINT_SCREEN)
				{
					cout << "inflight_reg blocking!" << endl;
				}
			}
		}
		else                                                             //读请求
		{
			if (pre_fifo.front()->valid && !pre_fifo.front()->pref && !pre_fifo.front()->rdwr)
			{
				if (NotSameBlock((pre_fifo.front()->ADDR_)))                                      //非reserve请求发送到cache和inflight_rag
				{
					if (!inflight_reg->valid && cache->trans_v == 0 && MSHR_STATE != 3)
					{
						delete inflight_reg;
						inflight_reg = pre_fifo.front();                                          //正在访问cache的请求进入inflight_reg
						if (!cache->addtranscation(pre_fifo[0]->ADDR_, pre_fifo[0]->rdwr))
						{
							cout << "error occurs for cache is busy when add trans!" << endl;
							system("pause");
						}
						//poped_addr.push_back(pre_fifo[0]->ADDR_);
						pre_fifo.erase(pre_fifo.begin());
					}
					else                                                  //blocking的情况下不做操作
					{
						if (PRINT_SCREEN)
						{
							cout << "inflight_reg blocking!" << endl;
							//system("pause");
						}
					}
				}
				else                                                                               //reserve请求直接发送到table而不需要去inflight_reg
				{
					if (addreserve2post(pre_fifo[0]))
					{
						delete pre_fifo.front();
						pre_fifo.erase(pre_fifo.begin());
					}
					else                                        //offset满,回到尾部
					{
						pre_fifo.push_back(pre_fifo.front());
						pre_fifo.erase(pre_fifo.begin());
					}
				}
			}
			else if (pre_fifo.front()->valid && pre_fifo.front()->pref && !pre_fifo.front()->rdwr)       //pref
			{
				if (NotSameBlock((pre_fifo.front()->ADDR_)))
				{
					if (!inflight_reg->valid && MSHR_STATE != 3)
					{
						delete inflight_reg;
						inflight_reg = pre_fifo.front();                                                    //正在访问cache的请求进入inflight_reg
						if (!cache->addtranscation(pre_fifo.front()->ADDR_, pre_fifo.front()->rdwr))
						{
							cout << "error occurs for cache is busy when add trans!" << endl;
							system("pause");
						}
						//poped_addr.push_back(pre_fifo.front()->ADDR_);
						pre_fifo.erase(pre_fifo.begin());

						if (PRINT_SCREEN)
							cout << "PRF transcation send to cache from table " << endl;
					}
					else
					{
						if (PRINT_SCREEN)
						{
							cout << "inflight_reg blocking!" << endl;
						}
					}
				}
				else     //当有同块正在访存时预取无意义，直接reset
					pre_fifo.erase(pre_fifo.begin());
			}
		}
	}
	//inflight请求更新！
    if (inflight_reg->valid && inflight_reg->complete)  
	{                                                        //当inflight请求阻塞且完成时，检测MSHR的next_index,若不冲突，则将阻塞的inflight请求完成
		if (MSHR_STATE != 1)
		{
			for (short j = 0; j < CACHE_LINE_SIZE; j++)
			{
				if (inflight_reg->offset[j].valid)
				{
					//cbk_addr在步长为1时才有效，用于debug
					(arbitrator->ArbitratorLines[inflight_reg->offset[j].pointer]->le_)->LEcallback(inflight_reg->ADDR_ + j, ClockCycle, inflight_reg->pe_tag, inflight_reg->pe_round);
					inflight_reg->offset[j].valid = 0;
				}
			}
			//release_poped_addr(inflight_reg->ADDR_);
			inflight_reg->reset();
		}
	}

	//update the table-fifo and send trans from arbitrator to table	                    第2步
	//uint32_t cyclepointer = arbitrator->pointer;
	uint32_t cyclepointer = arbitrator->pointer;
	uint32_t blank_tabline = FIFOLINE_NUM - pre_fifo.size();
	uint32_t in_num = blank_tabline < IN_NUM ? blank_tabline : IN_NUM;       //由于vec请求会产生多项，不一定准

	list<int> p_order;
	while (1)                           //寻找合适数量的trans进入table
	{
		if (arbitrator->ArbitratorLines[arbitrator->pointer]->valid == 1)
		{
			if (p_order.size() < in_num)
			{
				p_order.push_back(arbitrator->pointer);
			}
			else if (p_order.size() == in_num)                              //pointer停在上次满足入口要求后的地方！
				break;
		}
		if (arbitrator->pointer < (leNums + seNums - 1))
			arbitrator->pointer++;
		else if (arbitrator->pointer == (leNums + seNums - 1))
			arbitrator->pointer = 0;
		if (arbitrator->pointer == cyclepointer)                             //扫描完一遍，结束
			break;
	}
	if (p_order.size() == 0 && PRINT_SCREEN)
		cout << "no trans moved to table from ArbitratorLine" << endl;

	while (!p_order.empty())  //table满不能阻止请求进入v_table              
	{
		int index = p_order.front();
		int vec_index;
		if (IsPrefTran(arbitrator->ArbitratorLines[index]->TAG_))
		{
			if (pre_fifo.size() < FIFOLINE_NUM)
			{
				bool same_block = 0;
				{
					if (last_pref.size())
					{
						for (list<uint>::iterator it = last_pref.begin(); it != last_pref.end(); it++)
						{
							if (*it / CACHE_LINE_SIZE == arbitrator->ArbitratorLines[index]->ADDR_ / CACHE_LINE_SIZE)
							{
								same_block = 1;///////////////////////////////和所有过去的trans比较吗//////////////////////////
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
					if (FIFOLINE_NUM > pre_fifo.size())                       //由于有vec请求，需要先检测是否满了
					{
						TabLine* new_line = new TabLine();
						new_line->ADDR_ = arbitrator->ArbitratorLines[index]->ADDR_;
						new_line->rdwr = arbitrator->ArbitratorLines[index]->TAG_ < leNums ? 0 : 1;
						new_line->valid = 1;
						new_line->pref = 1;
						new_line->pe_tag = arbitrator->ArbitratorLines[index]->pe_tag;
						new_line->TAG_ = arbitrator->ArbitratorLines[index]->TAG_;
						new_line->pe_round = arbitrator->ArbitratorLines[index]->pe_round;

						new_line->transid = transid;                          //这两句绑死，每个请求拥有唯一的transid标识！
						transid++;                                            //只有一开始table入数的时候赋值	

						pre_fifo.push_back(new_line);

						if (last_pref.size() && last_pref.size() != 1 << TAGBITS)
						{
							last_pref.push_back(new_line->ADDR_);                //pref_table记录正在table中的预取请求还是记录所有已有的预取请求？
						}
						else if (!last_pref.size())       //第一次pref时触发
						{
							last_pref.push_back(new_line->ADDR_);
						}
						else if (last_pref.size() == 1 << TAGBITS)
						{
							last_pref.pop_front();
							last_pref.push_back(new_line->ADDR_);
						}
						else
						{
							cout << "error occurs in last_pref!" << endl;
						}

						if (PRINT_SCREEN)
							cout << "pref send from arbitratorline " << index << " to fifo!" << endl;
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

			if (FIFOLINE_NUM - pre_fifo.size() > trans_num)          //有空
			{
				arbitrator->ArbitratorLines[index]->returnACK();     //take the trans and return ACK to LE
				arbitrator->ArbitratorLines[index]->valid = 0;

				uint head_addr;                                             //head_addr表示每条合并请求的头部地址
				uint fir_addr = arbitrator->ArbitratorLines[index]->ADDR_;  //fir_addr表示每个VLE请求的基地址

				TabLine* new_line = new TabLine();
				short mask1 = fir_addr % CACHE_LINE_SIZE;
				new_line->offset[mask1].valid = 1;
				new_line->offset[mask1].pointer = arbitrator->ArbitratorLines[index]->TAG_ + 1;
				new_line->offset[mask1].tag = arbitrator->ArbitratorLines[index]->pe_tag;

				new_line->ADDR_ = fir_addr - fir_addr % CACHE_LINE_SIZE;
				new_line->TAG_ = arbitrator->ArbitratorLines[index]->TAG_;
				new_line->pe_round = arbitrator->ArbitratorLines[index]->pe_round;
				new_line->pe_tag = arbitrator->ArbitratorLines[index]->pe_tag;
				new_line->rdwr = arbitrator->ArbitratorLines[index]->TAG_ < leNums ? 0 : 1;
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
						new_line->offset[mask2].pointer = arbitrator->ArbitratorLines[index]->TAG_ + j + 1;
						new_line->offset[mask2].tag = arbitrator->ArbitratorLines[index]->pe_tag;
					}
					else
					{
						pre_fifo.push_back(new_line);
						new_line = new TabLine();
						head_addr = addr;

						mask2 = addr % CACHE_LINE_SIZE;
						new_line->offset[mask2].pointer = arbitrator->ArbitratorLines[index]->TAG_ + j + 1;
						new_line->offset[mask2].tag = arbitrator->ArbitratorLines[index]->pe_tag;
						new_line->offset[mask2].valid = 1;

						new_line->ADDR_ = addr - addr % CACHE_LINE_SIZE;
						new_line->TAG_ = arbitrator->ArbitratorLines[index]->TAG_;
						new_line->pe_round = arbitrator->ArbitratorLines[index]->pe_round;
						new_line->pe_tag = arbitrator->ArbitratorLines[index]->pe_tag;
						new_line->rdwr = arbitrator->ArbitratorLines[index]->TAG_ < leNums ? 0 : 1;
						new_line->valid = 1;
						new_line->pref = 0;
						new_line->vec = 1;

						new_line->transid = transid;                          //这两句绑死，每个请求拥有唯一的transid标识！
						transid++;
					}
				}
				pre_fifo.push_back(new_line);                                                   //跳出循环后还有最后一条

				if (PRINT_SCREEN)
					cout << "vec_tran send from arbitratorline " << index << " to fifo " << endl;
			}
		}
		else
		{
			if (FIFOLINE_NUM > pre_fifo.size())
			{

				TabLine* new_line = new TabLine();
				short addr_offset = arbitrator->ArbitratorLines[index]->ADDR_ % CACHE_LINE_SIZE;
				new_line->ADDR_ = arbitrator->ArbitratorLines[index]->ADDR_ - addr_offset;
				new_line->rdwr = arbitrator->ArbitratorLines[index]->TAG_ < leNums ? 0 : 1;
				new_line->valid = 1;
				new_line->pref = arbitrator->ArbitratorLines[index]->pref;
				new_line->pe_tag = arbitrator->ArbitratorLines[index]->pe_tag;
				new_line->TAG_ = arbitrator->ArbitratorLines[index]->TAG_;
				new_line->pe_round = arbitrator->ArbitratorLines[index]->pe_round;

				new_line->offset[addr_offset].valid = 1;
				new_line->offset[addr_offset].pointer = arbitrator->ArbitratorLines[index]->TAG_;
				new_line->offset[addr_offset].tag = arbitrator->ArbitratorLines[index]->pe_tag;

				new_line->transid = transid;                          //这两句绑死，每个请求拥有唯一的transid标识！
				transid++;                                            //只有一开始table入数的时候赋值

				pre_fifo.push_back(new_line);

				arbitrator->ArbitratorLines[index]->returnACK();     //take the data and return ACK to LE/SE
				arbitrator->ArbitratorLines[index]->valid = 0;

				if (PRINT_SCREEN)
					cout << "trans send from arbitratorline " << index << " to fifo!" << endl;
			}
		}

		p_order.pop_front();
	}
	if (!p_order.empty())
	{
		cout << "error occurs for !p_order.empty() after step3!" << endl;
		cachefile << "error occurs for !p_order.empty() after step3!" << endl;
	}

	if (PRINT_SCREEN)
	{
		cout << "MSHR_STATE = " << MSHR_STATE << endl;
		if (MSHR_STATE == 1)
			cout << "finishing the post_table " << miss_index << endl;

		cout << "------------FIFO-------------" << endl;
		for (uint32_t i = 0; i < pre_fifo.size(); i++)
			cout << "fifo " << i << " transid " << pre_fifo[i]->transid << " valid =" << pre_fifo[i]->valid << " addr "
			<< "=" << pre_fifo[i]->ADDR_ << " rdwr " << "=" << pre_fifo[i]->rdwr << " pointer = " << pre_fifo[i]->TAG_ << endl;

		cout << endl << "------------TABLE-------------" << endl;
		for (uint32_t i = 0; i < post_table.size(); i++)
		{
			cout << "table " << i << " transid " << post_table[i]->transid << " valid =" << post_table[i]->valid << " addr "
				<< "=" << post_table[i]->ADDR_ << " rdwr " << "=" << post_table[i]->rdwr << " complete = " << post_table[i]->complete;
			cout << " offset valid = ";
			for (int j = 0; j < POST_OFFSET_DEPTH; j++)
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

	if (OVERLAP_PRINT)
	{
		bool reading = 0;
		bool writing = 0;
		if (pre_fifo.front()->valid && pre_fifo.front()->rdwr == 1)
			writing = 1;
		for (int i = 0; i < pre_fifo.size(); i++)
		{
			if (pre_fifo[i]->valid && pre_fifo[i]->rdwr == 0)
			{
				reading = 1;
				break;
			}
		}

		if (reading)
			overlapfile_1 << "-";
		else if (writing)
			overlapfile_1 << "+";
		else
			overlapfile_1 << "=";

		if (ClockCycle % 500 == 0)
			overlapfile_1 << endl;
	}
}


bool LSUnit::NotSameBlock(uint32_t addr_)
{
	for (vector<uint32_t>::iterator iter = poped_addr.begin(); iter != poped_addr.end(); ++iter)
	{
		if (int(addr_ / CACHE_LINE_SIZE) == int(*iter / CACHE_LINE_SIZE))
			return 0;
	}
	return 1;
}

void LSUnit::read_hit_complete(uint32_t addr)
{
	if (MSHR_STATE != 1)                    //hit为miss完成让步暂停，标记complete位，当MSHR处于！=1状态时继续
	{
		if (inflight_reg->ADDR_ == addr && inflight_reg->valid && !inflight_reg->rdwr)
		{
			if (!inflight_reg->pref)
			{
				for (short j = 0; j < CACHE_LINE_SIZE; j++)
				{
					if (inflight_reg->offset[j].valid)
					{
						//cbk_addr在步长为1时才有效，用于debug
						(arbitrator->ArbitratorLines[inflight_reg->offset[j].pointer]->le_)->LEcallback(inflight_reg->ADDR_ + j, ClockCycle, inflight_reg->pe_tag, inflight_reg->pe_round);
						inflight_reg->offset[j].valid = 0;
					}
				}
				inflight_reg->reset();
			}
			else if (inflight_reg->pref)
			{
				inflight_reg->reset();
			}
			//release_poped_addr(addr);
		}
		else
		{
			cout << "error occurs for that readhit trans and inflight reg dont match!" << endl;
			system("pause");
		}
	}
	else
	{
		//inflight_reg若为非预取请求应标记为完成状态，在下一不冲突的cycle进行完成
		if (inflight_reg->pref)
		{
			inflight_reg->reset();
		}
		else
			inflight_reg->complete = 1;
	}
}

void LSUnit::write_hit_complete(uint32_t addr)
{
	if (inflight_reg->ADDR_ == addr && inflight_reg->valid && inflight_reg->rdwr)
	{
		//release_poped_addr(addr);
		inflight_reg->reset();
	}
	else
	{
		cout << "error occurs for that writehit trans and inflight reg dont match!" << endl;
		system("pause");
	}
}

void LSUnit::read_miss_complete(uint32_t addr)    //被动式的miss，通知MSHR中的同块请求去访问cache!
{
	uint32_t temp1 = 0;

	for (uint32_t i = 0; i < TABLINE_NUM; i++)
	{
		if (addr == post_table[i]->ADDR_ && post_table[i]->rdwr == 0 && post_table[i]->valid == 1 && post_table[i]->complete == 0)
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
		cout << "prf finished or error occurs in LSUnit::mem_read_complete() for no tableline finished when mem rdcallback " << endl;
		cout << "returned addr is " << addr << endl;
		release_poped_addr(addr);   //若未找到，说明为pref请求
	}
	else if (temp1 > 1)
	{
		cout << "error occurs in LSUnit::mem_read_complete() for more than one tableline finished when mem rdcallback " << endl;
		cachefile << "error occurs in LSUnit::mem_read_complete() for more than one tableline finished when mem rdcallback " << endl;
	}
}

void LSUnit::write_miss_complete(uint32_t addr)
{
	release_poped_addr(addr);	
}

void LSUnit::bus_update()
{
	if (BUS_ENABLE)
	{
		//bool out = 0;
		while (bus.size())
		{
			if (bus.front().cycle == BUS_DELAY)
			{
				mem->addTransaction(bus.front().rdwr, bus.front().addr);
				bus.pop_front();
				//out = 1;
			}
			else if (bus.front().cycle > BUS_DELAY)
			{
				cout << "error occurs in bus!" << endl;
				system("pause");
			}
			else if (bus.front().cycle < BUS_DELAY)
			{
				break;
			}
		}

		for (list<Bus_Tran>::iterator it = bus.begin(); it != bus.end(); it++)
		{
			if ((*it).cycle >= BUS_DELAY)
			{
				cout << "error occurs in bus!" << endl;
				system("pause");
			}
			else
				(*it).cycle++;
		}

		if (PRINT_BUS)
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


void LSUnit::config_in(vector<int> config)
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
			cout << "error occurs in void LSUnit::config_in!" << endl;
			std::system("pause");
		}
	}
}

bool LSUnit::IsPrefTran(uint32_t pointer)
{
	for (vector<uint>::iterator it = pref_pointer.begin(); it != pref_pointer.end(); it++)
	{
		if ((*it) == pointer)
			return true;
	}
	return false;
}

int LSUnit::IsVecTran(uint32_t pointer)
{
	for (int i = 0; i < vec_pointer.size(); i++)
	{
		if (vec_pointer[i].pointer == pointer)
			return i;
	}
	return -1;
}

bool LSUnit::addTrans2post(TabLine* line)
{
	for (short i = 0; i < TABLINE_NUM; i++)
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
				post_table[i]->pe_round = line->pe_round;
				post_table[i]->pe_tag = line->pe_tag;
				post_table[i]->rdwr = line->rdwr;
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

bool LSUnit::PostNotFull()
{
	for (short i = 0; i < TABLINE_NUM; i++)
	{
		if (!post_table[i]->valid)
		{
			return 1;
		}
	}
	return 0;
}

bool LSUnit::addreserve2post(TabLine* line)
{
	for (short i = 0; i < TABLINE_NUM; i++)
	{
		if (post_table[i]->valid && (line->ADDR_)/CACHE_LINE_SIZE == (post_table[i]->ADDR_)/CACHE_LINE_SIZE)
		{
			for (short j = 0; j < POST_OFFSET_DEPTH; j++)
			{
				if (!post_offset[i][j].valid)
				{
					post_offset[i][j].valid = 1;
					post_offset[i][j].round = line->pe_round;
					post_offset[i][j].offset = line->offset;
					return 1;
				}

				//offset已满	
			}
		}
	}
	return 0;
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
void LSUnit::release_poped_addr(uint32_t addr)
{
	for (vector<uint32_t>::iterator it = poped_addr.begin(); it < poped_addr.end(); it++)
	{
		if (*it == addr)
		{
			poped_addr.erase(it);
			return;
		}
	}
	cout << "error occurs in LSUnit::release_poped_addr! addr = " << addr << endl;
	system("pause");
}
