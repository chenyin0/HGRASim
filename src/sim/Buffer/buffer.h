#pragma once
#include "../../define/define.hpp"
#include "../module/module.h"
#include "../inout.h"
#include "../../preprocess/preprocess.h"
#include "../ClkDomain.h"

//data ����32λ�ڣ� 

namespace Simulator::Array
{
	struct Bufferd
	{
		bool valid;
		int valued;
		bool valueb;
		bool last;
		bool condition=true;
	};

	//decouple inbuffer and outbuffer
	class Buffer
	{
	protected:
		Preprocess::ArrayPara system_parameter;
		vector<vector<Bufferd>> entity;
		vector<uint> tail_ptr;
		vector<uint> head_ptr;
		//virtual bool canReceiveInput(Port_inout input, uint port) = 0;
		//virtual void bufferInput(Port_inout input, uint port) = 0;
		
	public:
		friend class Loadstore_element;
		virtual bool input(Port_inout input, uint port) = 0;
		virtual bool input_tag_lsu(Port_inout_lsu input, uint port, uint tag) = 0;
		virtual bool input_tag(Port_inout input, uint port, uint tag) = 0;
		virtual void output(Port_inout& input, uint port) = 0;
		virtual void back_head(uint port) = 0;
		virtual bool input_nolast(Port_inout input, uint port) = 0;
		virtual void outputTag(Port_inout& output, uint tag) = 0;
		virtual void resetTag(uint tag) = 0;
		virtual bool output_ack(Port_inout& output, uint port) = 0;
		virtual void reset() = 0;
		virtual void reset(uint port) = 0;
		virtual void reset_head() = 0;
		virtual bool empty() = 0;
		virtual bool isBufferNotFull(uint port) = 0;
		virtual void update() = 0;
		virtual bool isInputSuccess(uint port) = 0;           //����ackbp
		virtual bool isBufferNotEmpty(uint port) = 0;
		virtual void getTagMatchIndex(vector<uint>& vec, uint port,uint tag) = 0;
		virtual void print(std::ofstream& file);
		virtual void print_valid(std::ofstream& file);

		Buffer(const Preprocess::ArrayPara para) { system_parameter = para; }
		virtual ~Buffer() = default;
	};

	//pe��ib��ob��ʹ��fifo��Ϊ�����Ǹ��ļ̳й�ϵ  ib��ob�߶����ƣ�����buffer�´���һ��pe�м����
	//buffer�в�ͬ��portӦ�÷ֿ��������״̬�� inbuffer��ͷָ���βָ�뷽ʽ
	class Buffer_in: public Buffer
	{
	protected:
		//vector<vector<Bufferd>> entity;          //��bool���ͺ�data����ͳһ
		//vector<vector<Bufferb>> boolbuffer;
		uint depth;                                 //���
		uint boolbreadth;                           //���
		uint databreadth;
		//vector<uint> tail_ptr;
		//vector<uint> head_ptr;
		vector<Bool> port_full;
		vector<Bool> port_empty;
		vector<Bool> ack;

//		bool canReceiveInput(uint port);//credit����ʱ�ú���ֻ��Ҫassertion���ack����ʱ�ú�����Ҫ�������ܷ����
		void bufferInput(Port_inout input, uint port);
		void update_tail(uint port);
		void update_head(uint port);
		//void bpSim();

	public:
		bool canReceiveInput(uint port);//credit����ʱ�ú���ֻ��Ҫassertion���ack����ʱ�ú�����Ҫ�������ܷ����  //
		explicit Buffer_in(const Preprocess::ArrayPara para);
	//	void update_head(uint port);
		
		~Buffer_in() = default;

		virtual bool input(Port_inout input, const uint port) override;   //Port_inout��buffer��Ϊ��ͬ�����ݽṹΪ�˽�������ʱ����
		bool input_tag_lsu(Port_inout_lsu input, uint port, uint tag) override;
		virtual bool input_tag(Port_inout input, uint port, uint tag) override;
		virtual void back_head(uint port) override;
		virtual bool input_nolast(Port_inout input, uint port) override;
		virtual void output(Port_inout& output, uint port) override;
		virtual bool output_ack(Port_inout& input, uint port) override;
		virtual void outputTag(Port_inout& output, uint tag) override;
		virtual void resetTag(uint tag) override;
		virtual void reset() override;
		virtual void reset_head() override;
		virtual void reset(uint port) override;
		virtual bool empty() override;
		virtual bool isBufferNotFull(uint port) override;
		virtual bool isInputSuccess(uint port) override;
		virtual void update() override;
		virtual bool isBufferNotEmpty(uint port) override;
		void getTagMatchIndex(vector<uint>& vec, uint port,uint tag) override;
	};

	class Bufferpe_in: public Buffer_in
	{
	public:
		Bufferpe_in(const Preprocess::ArrayPara para);
		~Bufferpe_in() = default;
	};

	class Bufferlse_in : public Buffer_in
	{
	public:
		Bufferlse_in(const Preprocess::ArrayPara para);
		~Bufferlse_in() = default;
	};

	//outbuffer���ö�ͷָ���βָ�뷽ʽ�������
	class Buffer_out : public Buffer
	{
	protected:
		//vector<vector<Bufferd>> entity;          //��bool���ͺ�data����ͳһ
												 //vector<vector<Bufferb>> boolbuffer;
		uint depth;                                 //���
		uint boolbreadth;                           //���
		uint databreadth;
		//vector<uint> tail_ptr;
		//vector<uint> head_ptr;
		vector<Bool> port_empty;
		vector<Bool> port_full;
		vector<Bool> ack;
		//getBp��Ҫʹ����Щ�������м�⣬����ϣ���ó���
		//bool canReceiveInput(uint port);//credit����ʱ�ú���ֻ��Ҫassertion���ack����ʱ�ú�����Ҫ�������ܷ����
		//bool canReceiveInput(uint port, uint tag);
		//bool canReceiveInput(uint port, uint tag);
		void bufferInput(Port_inout input, uint port);
		void bufferInput(Port_inout input, uint port, uint tag);
		void bufferInput(Port_inout_lsu input, uint port, uint tag);
		void update_tail(uint port);
		void update_head(uint port);
		//void bpSim();

	public:
		explicit Buffer_out(const Preprocess::ArrayPara para);
		~Buffer_out() = default;
	//	void update_head(uint port);
		bool canReceiveInput(uint port);//credit����ʱ�ú���ֻ��Ҫassertion���ack����ʱ�ú�����Ҫ�������ܷ����
		virtual void back_head(uint port) override;
		bool canReceiveInput(uint port, uint tag);
		virtual bool input_nolast(Port_inout input, uint port) override;
		bool input(Port_inout input, const uint port) override;   //Port_inout��buffer��Ϊ��ͬ�����ݽṹΪ�˽�������ʱ����
		bool input_tag_lsu(Port_inout_lsu input, uint port, uint tag) override;
		bool input_tag(Port_inout input, uint port, uint tag) override;
		virtual void output(Port_inout& output, uint port) override;
		virtual bool output_ack(Port_inout& output, uint port) override;
		virtual void outputTag(Port_inout& output, uint tag) override;
		virtual void resetTag(uint tag) override;
		virtual 
		void reset() override;
		virtual void reset(uint port) override;
		virtual void reset_head() override;
		bool empty() override;
		bool isBufferNotFull(uint port) override;
		bool isInputSuccess(uint port) override;
		void update() override;
		bool isBufferNotEmpty(uint port) override;
		void getTagMatchIndex(vector<uint>& vec, uint port,uint tag) override;
	};

	class Bufferlse_out:public Buffer_out
	{
	public:
		Bufferlse_out(const Simulator::Preprocess::ArrayPara para);
		Bufferlse_out(const Simulator::Preprocess::ArrayPara para, Simulator::BufferSize size,Simulator::LSMode lsmode);
		~Bufferlse_out() = default;
		bool output_ack(Port_inout& output, uint tag) override;      //��tag���
//		void output(Port_inout& output, uint tag) override;         //��tag���
		void outputTag(Port_inout& output, uint tag);
		void resetTag(uint tag);
//		void reset(uint tag) override;                              //��tag reset
	};

	class Buffer_lc : public Buffer_out
	{
	public:
		Buffer_lc(const Preprocess::ArrayPara para);
		~Buffer_lc() = default;
	};

	class Bufferpe_out: public Buffer_out
	{
	public:
		Bufferpe_out(const Preprocess::ArrayPara para);
		~Bufferpe_out() = default;
	};
	
}
