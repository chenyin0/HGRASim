#include "Cache.h"
#include "Lsu.h"

using namespace DRAMSim;

Cache::Cache(const Simulator::Preprocess::ArrayPara para):Node(para)
{
	//state = 0;
	//if (Simulator::Array::ClkDomain::getInstance()->getClk() >= Simulator::Array::Debug::getInstance()->print_file_begin && Simulator::Array::ClkDomain::getInstance()->getClk() < Simulator::Array::Debug::getInstance()->print_file_end)
	//	print_screen = false;
	//system_parameter.bus_enable = 1;
	cache_allhit = 0;
	misscounter = 0;
	non_pref_miss = 0;
	WB_table.resize(0);
	//data = 0;   
	//lsunit = lsu;
	for (int bank = 0; bank < CACHE_BANK; bank++)
	{
		for (uint32_t i = 0; i < CACHE_WAYS_NUM; i++)
		{
			for (uint32_t j = 0; j < CACHE_LINE_EACH_WAY; j++)
			{
				CacheTopology[bank][i][j] = 0;
				counter[bank][i][j] = 0;
			}
		}
	}

	cout << ADDR_INDEX << INDEX_BITS << ADDR_WORD << WORD_BITS << ADDR_BANK << BANK_BITS << endl;
}


Cache::~Cache()
{
}

/*
void Cache::attachlsu(Lsu *lsu)
{
lsunit = lsu;
}
*/

void Cache::attach(Lsu* lsu)
{
	lsunit = lsu;
}

bool Cache::addtranscation(uint32_t adr, uint32_t rw)
{
	short bank = (adr & BANK_BITS) >> ADDR_BANK;
	if (trans_v[bank] == 0)
	{
		addr_in[bank] = adr;
		rdwr_in[bank] = rw;
		trans_v[bank] = 1;
		return true;
	}
	else
	{
		return false;
	}
}




bool Cache::do_lookup(uint32_t addr)
{
	//uint32_t lineIndex = (addr & INDEX_BITS) >> ADDR_INDEX;           //提取组
	//cycle = cycle + LOOKUP_DELAY;
	uint32_t bank = (addr & BANK_BITS) >> ADDR_BANK;
	uint32_t lineIndex = (addr & INDEX_BITS) >> ADDR_INDEX;
	uint32_t lineTag = (addr & TAGS_BITS) >> ADDR_TAGS;   //提取TAG
	uint32_t lineWord = (addr & WORD_BITS) >> ADDR_WORD;
	uint32_t lineWay;
	// search the cache set
	int hit = 0;
	for (int i = 0; i < CACHE_WAYS_NUM; i++)
	{
		//find the cacheline in the set
		if ((CacheTopology[bank][i][lineIndex] & TAGS_BITS) >> ADDR_TAGS == lineTag)      //组定位后比较TAG
		{
			if ((CacheTopology[bank][i][lineIndex] & VALID_BIT))
			{
				hit = 1;
				lineWay = i;
			}
		}
	}
	//lsunit->hit = hit;

	switch (hit)
	{
	case 0:
	{ 	//do_replacement();
		/* cache miss替换后仍然要实现读写
		*/
		break;
	}
	case 1:
	{
		if (REPLACE_MODE == 1)                                              //更新使用优先级可以在此完成
		{
			short ori_counter = counter[bank][lineWay][lineIndex];
			if (ori_counter != (CACHE_WAYS_NUM - 1))                       //重新排序
			{
				for (int i = 0; i < CACHE_WAYS_NUM; i++)
				{
					if (counter[bank][i][lineIndex] > ori_counter)
					{
						counter[bank][i][lineIndex]--;
					}
				}
				counter[bank][lineWay][lineIndex] = CACHE_WAYS_NUM - 1;
			}
		}
		if (system_parameter.print_screen)
			cout << "cache hit!" << endl;

		break;
	}
	default:
		cout << "Error Condition in do_lookup" << endl;
		break;
	}

	return hit;
}


void Cache::do_replacement(uint32_t addr)   //还需要检查UPDATE位，看是否需要写回。     不同替换算法的实现   writeback需不需要delay
{
	uint32_t bank = (addr & BANK_BITS) >> ADDR_BANK;
	uint32_t lineIndex = (addr & INDEX_BITS) >> ADDR_INDEX;
	uint32_t lineTag = (addr & TAGS_BITS) >> ADDR_TAGS;   //提取TAG
	uint32_t lineWord = (addr & WORD_BITS) >> ADDR_WORD;
	uint32_t ReplaceWay;

	uint32_t setup = (lineTag << ADDR_TAGS) + (lineIndex << ADDR_INDEX);
	uint32_t counterbits = CACHE_WAYS_NUM;
	//uint32_t ReplaceWay;

	if (REPLACE_MODE == 0)
		ReplaceWay = rand() % CACHE_WAYS_NUM;
	else if (REPLACE_MODE == 1)
	{
		for (uint32_t i = 0; i < CACHE_WAYS_NUM; i++)
		{
			if (counterbits > counter[bank][i][lineIndex])
			{
				counterbits = counter[bank][i][lineIndex];
				ReplaceWay = i;
			}
		}
	}
	else if (REPLACE_MODE == 2)
	{
		for (uint32_t i = 0; i < CACHE_WAYS_NUM; i++)
		{
			if (counter[bank][i][lineIndex] == 0)
			{
				ReplaceWay = i;
			}
			else
				printf("error occurs for fifo replace mode that no line replaced!\n");
		}
	}



	if (WRITE_MODE == 1)
	{
		if (CacheTopology[bank][ReplaceWay][lineIndex] & UPDATE_BIT)   //被替换块若UPDATE=1则写回内存否则只需替换
		{
			do_writeback(ReplaceWay, lineIndex, bank);
		}
	}

	//if(WRITE_MODE == 0 && rdwr == 1)   //当为直写策略且为写miss时已经回写过了
	//do_writethrough();


	//for(j = 0; j < CACHE_LINE_SIZE; j++)                                                      //替换
	//CacheTopoFindWord[ReplaceWay][lineIndex][j] = DramTopology[setup + j]; //填充cache_line 
	if (REPLACE_MODE == 0)
		CacheTopology[bank][ReplaceWay][lineIndex] = (lineTag << ADDR_TAGS) + VALID_BIT;  //更改TAG标记，UPDATE标记为0 
	else if (REPLACE_MODE == 1)
	{
		CacheTopology[bank][ReplaceWay][lineIndex] = (lineTag << ADDR_TAGS) + VALID_BIT;
		counter[bank][ReplaceWay][lineIndex] = CACHE_WAYS_NUM;
		for (int j = 0; j < CACHE_WAYS_NUM; j++)
		{
			if (counter[bank][j][lineIndex] != 0)
				counter[bank][j][lineIndex]--;
			else if (counter[bank][j][lineIndex] < 0)
				printf("error occurs that cachecounter < 0!\n");
		}
	}
	else if (REPLACE_MODE == 2)
	{
		CacheTopology[bank][ReplaceWay][lineIndex] = (lineTag << ADDR_TAGS) + VALID_BIT;
		counter[bank][ReplaceWay][lineIndex] = CACHE_WAYS_NUM;
		for (int j = 0; j < CACHE_WAYS_NUM; j++)
		{
			if (counter[bank][j][lineIndex] > 0)
				counter[bank][j][lineIndex]--;
			else if (counter[bank][j][lineIndex] < 0)
				printf("error occurs that cachecounter < 0!\n");
		}
	}
	else
		printf("error occurs for replace mode!\n");

	/*miss后的读写在访问mem时完成
	switch (rdwr)
	{
	case 0: do_read(ReplaceWay);
	break;
	case 1: do_write(ReplaceWay);
	break;
	default:
	printf("Unknown operation in do_lookup.\n");
	break;
	}
	*/
	lsunit->cachefile << "cache第 " << lineIndex << " 组打印" << endl;
	for (int i = 0; i < CACHE_WAYS_NUM; i++)
	{
		lsunit->cachefile << i << " " << CacheTopology[bank][i][lineIndex] << " " << counter[bank][i][lineIndex] << endl;
	}

}


void Cache::do_readhit(uint32_t addr)
{
	if (system_parameter.print_screen)
	{
		uint32_t bank = (addr & BANK_BITS) >> ADDR_BANK;
		uint32_t lineIndex = (addr & INDEX_BITS) >> ADDR_INDEX;
		uint32_t lineTag = (addr & TAGS_BITS) >> ADDR_TAGS;   //提取TAG
		uint32_t lineWord = (addr & WORD_BITS) >> ADDR_WORD;
		uint32_t lineWay;

		cout << "打印第  " << lineIndex << " 组cache中的tag和update " << endl;
		for (int i = 0; i < CACHE_WAYS_NUM; i++)
		{
			cout << CacheTopology[bank][i][lineIndex] << " " << i << "way" << endl;
		}
	}
}


void Cache::do_writehit(uint32_t addr)
{
	uint32_t bank = (addr & BANK_BITS) >> ADDR_BANK;
	uint32_t lineIndex = (addr & INDEX_BITS) >> ADDR_INDEX;
	uint32_t lineTag = (addr & TAGS_BITS) >> ADDR_TAGS;   //提取TAG
	uint32_t lineWord = (addr & WORD_BITS) >> ADDR_WORD;
	uint32_t lineWay;

	bool find;
	for (int i = 0; i < CACHE_WAYS_NUM; i++)
	{
		//find the cacheline in the set
		if ((CacheTopology[bank][i][lineIndex] & TAGS_BITS) >> ADDR_TAGS == lineTag)      //组定位后比较TAG
		{
			if ((CacheTopology[bank][i][lineIndex] & VALID_BIT))
			{
				lineWay = i;
				find = 1;
			}
		}
	}
	if (!find && !cache_allhit)
	{
		cout << "在hit延时期间命中的块被替换了！" << endl;
		system("pause");
	}

	if (WRITE_MODE == 0)
	{
		//cycle = cycle + WRITEHIT_DELAY;
		do_writethrough(addr);
	}


	//CacheTopoFindWord[lineWay][lineIndex][lineWord] = data;
	CacheTopology[bank][lineWay][lineIndex] |= UPDATE_BIT;           //写过以后update置位 
															   //printf("hit and write data %x \n",CacheTopoFindWord[lineWay][lineIndex][lineWord]);

															   //std::cout << "write data " << hex << CacheTopoFindWord[i][lineIndex][lineWord] << std::endl;
	if (system_parameter.print_screen)
	{
		cout << "打印第  " << lineIndex << " 组cache中的tag和update " << endl;
		for (int i = 0; i < CACHE_WAYS_NUM; i++)
		{
			cout << CacheTopology[bank][i][lineIndex] << " i way" << endl;
		}
	}
}



void Cache::do_writeback(uint32_t ReplaceWay, uint32_t lineIndex, uint32_t bank)
{
	uint32_t RepTag;
	uint32_t RepIndex;
	uint32_t RepAdr;
	uint32_t RepBank;
	uint32_t k;

	RepTag = (CacheTopology[bank][ReplaceWay][lineIndex] & TAGS_BITS) >> ADDR_TAGS;
	RepIndex = lineIndex;
	RepBank = bank;
	RepAdr = (RepTag << ADDR_TAGS) + (RepIndex << ADDR_INDEX) + (RepBank << ADDR_BANK);

	//整个块的回写
	//printf("write back.\n");
	lsunit->cachefile << "write back" << endl;
	if (!system_parameter.bus_enable)
	{
		lsunit->mem->addTransaction(true, (uint64_t(RepAdr) << 2));
	}
	else if (system_parameter.bus_enable)
	{
		Bus_Tran tran;
		tran.addr = (uint64_t(RepAdr) << 2);
		tran.rdwr = true;
		tran.cycle = 0;
		lsunit->bus.push_back(tran);
	}
	WB_table.push_back(RepAdr);

}

void Cache::do_writethrough(uint32_t addr)
{
	//cycle = cycle + WRITETHROUGH_DELAY;
	if (!system_parameter.bus_enable)
	{
		lsunit->mem->addTransaction(true, (uint64_t(addr) << 2));
	}                                                                       //注意writethrough和writeback产生的callback！
	else if (system_parameter.bus_enable)
	{
		Bus_Tran tran;
		tran.addr = (uint64_t(addr) << 2);
		tran.rdwr = true;
		tran.cycle = 0;
		lsunit->bus.push_back(tran);
	}
	WB_table.push_back(addr);
}

/*
void Cache::do_read(uint32_t ReplaceWay)
{
cycle = cycle + READ_DEALY;
uint32_t i;
if(!hit)
{
data = CacheTopoFindWord[ReplaceWay][lineIndex][lineWord];
printf("not hit, read data %x \n",data);
}
else
printf("error occurs in do_read\n");

printf("now the cacheline is %x.\n",CacheTopology[ReplaceWay][lineIndex]);
for(i = 0; i < CACHE_WAYS_NUM; i++)
{
printf("TAG %x in this index.\n", CacheTopology[i][lineIndex]);
}


}


void Cache::do_write(uint32_t ReplaceWay)
{
cycle = cycle + WRITE_DELAY;
uint32_t i;
if(!hit)
{
CacheTopoFindWord[ReplaceWay][lineIndex][lineWord] = data;
CacheTopology[ReplaceWay][lineIndex] |= UPDATE_BIT;           //写过以后update置位
printf("not hit, write data %x \n",CacheTopoFindWord[ReplaceWay][lineIndex][lineWord]);
}
else
printf("error occurs in do_write\n");

printf("now the cacheline is %x.\n",CacheTopology[ReplaceWay][lineIndex]);
for(i = 0; i < CACHE_WAYS_NUM; i++)
{
printf("TAG %x in this index.\n", CacheTopology[i][lineIndex]);
}

}
*/

void Cache::update()                                           //利用宏实现的延时适合顺序写
{
	/*
	if (missfifo.size())
	lsunit->cachefile << "miss_fifo size = " << missfifo.size() << "and the front is " << missfifo.front().addr << missfifo.front().cycle << endl;
	if (hitfifo.size())
	lsunit->cachefile << "hit_fifo size = " << hitfifo.size() << "and the front is " << hitfifo.front().addr << hitfifo.front().cycle << endl;
	*/

	for (int i = 0; i < CACHE_BANK; i++)
	{
		if (state[i] == 0)
		{
			if (lookup_trans[i].valid == 0 && trans_v[i] == 1)           //cache入数
			{
				lookup_trans[i].addr = addr_in[i];
				lookup_trans[i].rdwr = rdwr_in[i];
				lookup_trans[i].cycle = 0;
				lookup_trans[i].valid = trans_v[i];
				state[i] = 1;
				trans_v[i] = 0;
			}
		}
		if (lookup_trans[i].valid)
		{
			if (hitfifo[i].size() == 0)
			{
				lookup_trans[i].cycle = 0;
				hitfifo[i].push_back(lookup_trans[i]);
				lookup_trans[i].valid = 0;
				if (system_parameter.print_screen)
				{
					//cout << "addr " << lookup_trans.addr << " hit!" << endl;
				}
			}
			else
			{
				cout << "error occurs in Cache::update()!" << endl;//hitfifo.size!=0有问题吗////////////////
				system("pause");
			}
		}


		if (hitfifo[i].size())
		{
			if (hitfifo[i].size() == 1)
			{
				if (hitfifo[i].front().cycle < LOOKUP_DELAY)
					hitfifo[i].front().cycle++;
				else
				{
					cout << "error occurs in Cache::update()!" << endl;
					system("pause");
				}

				if (hitfifo[i].front().cycle == LOOKUP_DELAY)
				{
					if (cache_allhit)///////////////////////不会有这个的
					{
						state[i] = 0;
						if (hitfifo[i].front().rdwr)
						{
							lsunit->write_hit_complete(hitfifo[i].front().addr);
							//do_writehit(hitfifo[i].front().addr);
							lsunit->cachefile << "RD Cache hit in Addr " << hitfifo[i].front().addr << " at Mem Cycle " << lsunit->ClockCycle << endl;
						}
						else
						{
							lsunit->read_hit_complete(hitfifo[i].front().addr);
							do_readhit(hitfifo[i].front().addr);
							lsunit->cachefile << "WR Cache hit in Addr " << hitfifo[i].front().addr << " at Mem Cycle " << lsunit->ClockCycle << endl;
						}
						hitfifo[i].pop_back();
					}
					else
					{
						if (do_lookup(hitfifo[i].front().addr))    //若命中
						{
							state[i] = 0;
							if (hitfifo[i].front().rdwr)
							{
								lsunit->write_hit_complete(hitfifo[i].front().addr);
								do_writehit(hitfifo[i].front().addr);
								lsunit->cachefile << "RD Cache hit in Addr " << hitfifo[i].front().addr << " at Mem Cycle " << lsunit->ClockCycle << endl;
							}
							else
							{
								lsunit->read_hit_complete(hitfifo[i].front().addr);
								do_readhit(hitfifo[i].front().addr);
								lsunit->cachefile << "WR Cache hit in Addr " << hitfifo[i].front().addr << " at Mem Cycle " << lsunit->ClockCycle << endl;
							}
							hitfifo[i].pop_back();
						}
						else                                        //miss
						{
							if (!system_parameter.bus_enable)
							{
								if (!hitfifo[i].front().rdwr)         //只有读请求才能进入MSHR！！！
								{
									if (!lsunit->addTrans2post(lsunit->inflight_reg[i]))                                               //miss的请求从reg转移到MSHR
									{
										TabLine* transcation = new TabLine();
										transcation->ADDR_ = lsunit->inflight_reg[i]->ADDR_;
										transcation->rdwr = lsunit->inflight_reg[i]->rdwr;
										transcation->transid = lsunit->inflight_reg[i]->transid;
										transcation->complete = lsunit->inflight_reg[i]->complete;
										transcation->offset = lsunit->inflight_reg[i]->offset;
//										transcation->offset_4 = lsunit->inflight_reg[i]->offset_4;
//										transcation->pe_round = lsunit->inflight_reg[i]->pe_round;
										transcation->pe_tag = lsunit->inflight_reg[i]->pe_tag;
//										transcation->pref = lsunit->inflight_reg[i]->pref;
										transcation->TAG_ = lsunit->inflight_reg[i]->TAG_;
										transcation->valid = lsunit->inflight_reg[i]->valid;
//										transcation->vec = lsunit->inflight_reg[i]->vec;
										lsunit->pre_fifo[i].push_back(transcation);                                           //table满时返回fifo底部
									}
									else
									{
										lsunit->mem->addTransaction(hitfifo[i].front().rdwr, uint64_t(hitfifo[i].front().addr << 2));    //一个字4字节，所以左移2位
										lsunit->poped_addr.push_back(hitfifo[i].front().addr);
										// if (!lsunit->inflight_reg[i]->pref)
										// 	non_pref_miss++;
									}
								}
								lsunit->inflight_reg[i]->reset();
							}
							else if (system_parameter.bus_enable)
							{
								if (!hitfifo[i].front().rdwr)
									if (!lsunit->addTrans2post(lsunit->inflight_reg[i]))
									{
										TabLine* transcation = new TabLine();
										transcation->ADDR_ = lsunit->inflight_reg[i]->ADDR_;
										transcation->rdwr = lsunit->inflight_reg[i]->rdwr;
										transcation->transid = lsunit->inflight_reg[i]->transid;
										transcation->complete = lsunit->inflight_reg[i]->complete;
										transcation->offset = lsunit->inflight_reg[i]->offset;
//										transcation->offset_4 = lsunit->inflight_reg[i]->offset_4;
//										transcation->pe_round = lsunit->inflight_reg[i]->pe_round;
										transcation->pe_tag= lsunit->inflight_reg[i]->pe_tag;
										transcation->pref = lsunit->inflight_reg[i]->pref;
										transcation->TAG_ = lsunit->inflight_reg[i]->TAG_;
										transcation->valid = lsunit->inflight_reg[i]->valid;
										transcation->vec = lsunit->inflight_reg[i]->vec;
										lsunit->pre_fifo[i].push_back(transcation);
									}
									else
									{
										Bus_Tran tran;
										tran.addr = uint64_t(hitfifo[i].front().addr << 2);
										tran.rdwr = hitfifo[i].front().rdwr;
										tran.cycle = 0;
										lsunit->bus.push_back(tran);
										lsunit->poped_addr.push_back(hitfifo[i].front().addr);
										//统计非预取的miss
										// if (!lsunit->inflight_reg[i]->pref)
										// 	non_pref_miss++;
									}
								lsunit->inflight_reg[i]->reset();
							}

							state[i] = 0;
							misscounter++;
							lsunit->cachefile << hitfifo[i].front().rdwr << " Trans Cache Miss in Addr " << hitfifo[i].front().addr << " at Mem Cycle " << lsunit->ClockCycle;
							lsunit->cachefile << " -----Miss_counter =  " << misscounter << endl;
							hitfifo[i].pop_back();
						}
					}
				}
			}
			else
			{
				cout << "error occurs in Cache::update()!" << endl;
				system("pause");
			}
		}

		if (hitfifo[i].size() + lookup_trans[i].valid > 1)                  //独占的！
			cout << "error occurs in Cache::update()!" << endl;
	}
}


void Cache::mem_read_complete(unsigned id, uint64_t address, uint64_t clock_cycle)
{
	//uint32_t j;
	//printf("[Callback] read complete from mem: %d 0x%llx cycle=%llu\n", id, address, clock_cycle);
	uint32_t addr = (uint32_t)address >> 2;                 //字节->字
	if (system_parameter.print_screen)
		std::cout << "[Callback] read complete from mem: " << id << " " << addr << " " << clock_cycle << endl;
	uint32_t temp1 = 0;

	do_replacement(addr);                                //从mem读完成后才进行替换！

	lsunit->read_miss_complete(addr);                        //向lsu返回读完成
}


void Cache::mem_write_complete(unsigned id, uint64_t address, uint64_t clock_cycle)
{
	//uint32_t j;
	//printf("[Callback] read complete from mem: %d 0x%llx cycle=%llu\n", id, address, clock_cycle);
	uint32_t addr = (uint32_t)address >> 2;
	if (system_parameter.print_screen)
		std::cout << "[Callback] read complete from mem: " << id << " " << addr << " " << clock_cycle << endl;
	uint32_t temp1 = 0;

	bool misstrans = 1;
	for (vector<uint32_t>::iterator it = WB_table.begin(); it < WB_table.end(); it++)          //检测是不是写回触发的call back
	{
		if (*it == addr)
		{
			if (system_parameter.print_screen)
			{
				cout << "write_back or write_through completed" << endl;
			}
			misstrans = 0;
			WB_table.erase(it);
			break;
		}
	}

	if (misstrans)
	{
		do_replacement(addr);                                //从mem读完成后才进行替换！
		lsunit->write_miss_complete(addr);                        //向lsu返回读完成
	}

}

