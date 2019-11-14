#pragma once
#include "../module/module.h"
#include "../Buffer/buffer.h"
#include "../protocol/protocol.h"

namespace Simulator::Array
{
	//BUFFER_FACTORY
	class Buffer_factory
	{
	public:
		static Buffer* createInBuffer(Preprocess::ArrayPara para);
		static Buffer* createOutBuffer(Preprocess::ArrayPara para);
		static Buffer* createLcBuffer(Preprocess::ArrayPara para);
		static Buffer* createLseInBuffer(Preprocess::ArrayPara para);
		static Buffer* createLseBuffer(Simulator::Preprocess::ArrayPara para, Simulator::BufferSize size,Simulator::LSMode lsmode);
		~Buffer_factory() = default;
	};
	/*
	class Bufferpein_factory final : public Buffer_factory
	{
	public:
		Buffer * createBuffer(const Preprocess::ArrayPara para) override { return new Bufferpe_in(para); }
	};

	class Bufferpeout_factory final : public Buffer_factory
	{
	public:
		Buffer * createBuffer(const Preprocess::ArrayPara para) override { return new Bufferpe_out(para); }
	};
	*/
	//ALU_FACTORY
	class Alu_factory
	{
	public:
		virtual Alu* createAlu() = 0;
		virtual ~Alu_factory() = default;
	};


	class Bp_factory
	{
	public:
		static Bufferbp* createBufferBp(Preprocess::ArrayPara para, Buffer* buffer);
		static Protocol* createRegBp(Preprocess::ArrayPara para, Bufferd *reg);
		static Protocol* createLrBp(Preprocess::ArrayPara para, Localreg* lr);
		static Pebp* createPeBp(Preprocess::ArrayPara para, Processing_element* pe);
		static Lsebp* createLseBp(Simulator::Preprocess::ArrayPara para, Loadstore_element* pe);
		~Bp_factory() = default;
	};


	//IB_FACTORY 
	//reserved for future
	class Instructionbuffer_factory
	{
	public:
		virtual Instructionbuffer* createIb() = 0;
		virtual ~Instructionbuffer_factory() = default;
	};


	//LR_FACTORY
	class Localreg_factory
	{
	public:
		virtual Localreg* createLr() = 0;
		virtual ~Localreg_factory() = default;
	};


}
