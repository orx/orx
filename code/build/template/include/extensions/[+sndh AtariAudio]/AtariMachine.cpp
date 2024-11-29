/*--------------------------------------------------------------------
	Atari Audio Library
	Small & accurate ATARI-ST audio emulation
	Arnaud Carré aka Leonard/Oxygene
	@leonard_coder
--------------------------------------------------------------------*/
#include <stdlib.h>		// malloc & free
#include <string.h>		// memset & memcpy
#include <assert.h>
#include "external/Musashi/m68k.h"
#include "AtariMachine.h"

#define D_DUMP_READ		0
#define D_DUMP_WRITE	0
static const uint32_t D_DUMP_READ_AD1 = 0xfffa00;
static const uint32_t D_DUMP_READ_AD2 = 0xfffaff;
static const uint32_t D_DUMP_WRITE_AD1 = 0xfffa00;
static const uint32_t D_DUMP_WRITE_AD2 = 0xfffaff;
#if (D_DUMP_READ|D_DUMP_WRITE)
#include <stdio.h>
#endif

static AtariMachine*	gCurrentMachine = NULL;
static const uint32_t ivector[5] = { 0x134,0x120,0x114,0x110,0x13c };

unsigned int  m68k_read_memory_8(unsigned int address)
{
	return gCurrentMachine->memRead8(address);
}

unsigned int  m68k_read_memory_16(unsigned int address)
{
	return gCurrentMachine->memRead16(address);
}

void m68k_write_memory_8(unsigned int address, unsigned int value)
{
	gCurrentMachine->memWrite8(address, value);
}

void m68k_write_memory_16(unsigned int address, unsigned int value)
{
	gCurrentMachine->memWrite16(address, value);
}

unsigned int  m68k_read_memory_32(unsigned int address)
{
	uint32_t r = m68k_read_memory_16(address) << 16;
	r |= m68k_read_memory_16(address + 2);
	return r;
}

void m68k_write_memory_32(unsigned int address, unsigned int value)
{
	m68k_write_memory_16(address, uint16_t(value >> 16));
	m68k_write_memory_16(address + 2, uint16_t(value));
}


unsigned int  AtariMachine::memRead8(unsigned int address)
{
	assert(0 == (address & 0xff000000));
	uint8_t r = ~0;
	if (address < RAM_SIZE)
		return m_RAM[address];
	if ((address >= 0xff8800) && (address < 0xff8900))
		r = m_Ym2149.ReadPort(address & 255);
	else if (0xff8260 == address)
		r = 0;		// simulate Atari ST low res
	else if (0xff820a == address)
		r = 2;		// simulate Atari ST PAL (50Hz)
	else if ((address >= 0xfffa00) && (address < 0xfffa26))
		r = m_Mfp.Read8(address - 0xfffa00);
	else if ((address >= 0xff8900) && (address < 0xff8926))
		r = m_SteDac.Read8(address - 0xff8900);
#if D_DUMP_READ
	if ((address >= D_DUMP_READ_AD1) && (address <= D_DUMP_READ_AD2))
	{
		uint32_t pc = m68k_get_reg(NULL, M68K_REG_PC);
		printf("$%06x: move.b $%06x,d0 ( =#$%02x )\n", pc, address, r);
	}
#endif
	return r;
}

unsigned int  AtariMachine::memRead16(unsigned int address)
{
	assert(0 == (address & 0xff000000));
	uint16_t r = ~0;
	if (address < RAM_SIZE - 1)
		return uint16_t((m_RAM[address] << 8) | (m_RAM[address + 1]));
	if ((address >= 0xff8800) && (address < 0xff8900))
		r = m_Ym2149.ReadPort(address & 0xfe) << 8;
	else if ((address >= 0xfffa00) && (address < 0xfffa26))
		r = m_Mfp.Read16(address - 0xfffa00);
	else if ((address >= 0xff8900) && (address < 0xff8926))
		r = m_SteDac.Read16(address - 0xff8900);
#if D_DUMP_READ
	if ((address >= D_DUMP_READ_AD1) && (address <= D_DUMP_READ_AD2))
	{
		uint32_t pc = m68k_get_reg(NULL, M68K_REG_PC);
		printf("$%06x: move.w $%06x,d0 ( =#$%04x )\n", pc, address, r);
	}
#endif
	return r;
}

void AtariMachine::memWrite8(unsigned int address, unsigned int value)
{
	assert(0 == (address & 0xff000000));
	if (address < RAM_SIZE)
	{
		m_RAM[address] = value;
		return;
	}
#if D_DUMP_WRITE
	if ((address >= D_DUMP_WRITE_AD1) && (address <= D_DUMP_WRITE_AD2))
	{
		uint32_t pc = m68k_get_reg(NULL, M68K_REG_PC);
		printf("$%06x: move.b #$%02x,$%06x\n", pc, value, address);
	}
#endif
	if ((address >= 0xff8800) && (address < 0xff8900))
		m_Ym2149.WritePort(address & 0xfe, (uint8_t)value);	// atari ym 8800 is also shadowed in 8801 and 8802 in 8803
	else if ((address >= 0xfffa00) && (address < 0xfffa26))
		m_Mfp.Write8(address - 0xfffa00, uint8_t(value));
	else if ((address >= 0xff8900) && (address < 0xff8926))
		m_SteDac.Write8(address - 0xff8900, uint8_t(value));
}

void AtariMachine::memWrite16(unsigned int address, unsigned int value)
{
	assert(0 == (address & 0xff000000));
	if (address < RAM_SIZE - 1)
	{
		m_RAM[address] = uint8_t(value >> 8);
		m_RAM[address + 1] = uint8_t(value);
		return;
	}
#if D_DUMP_WRITE
	if ((address >= D_DUMP_WRITE_AD1) && (address <= D_DUMP_WRITE_AD2))
	{
		uint32_t pc = m68k_get_reg(NULL, M68K_REG_PC);
		printf("$%06x: move.w #$%04x,$%06x\n", pc, value, address);
	}
#endif
	if ((address >= 0xff8800) && (address < 0xff8900))
		m_Ym2149.WritePort(address & 0xfe, uint8_t(value >> 8));
	else if ((address >= 0xfffa00) && (address < 0xfffa26))
		m_Mfp.Write16(address - 0xfffa00, uint16_t(value));
	else if ((address >= 0xff8900) && (address < 0xff8926))
		m_SteDac.Write16(address - 0xff8900, uint16_t(value));
}

AtariMachine::AtariMachine()
{
	m_RAM = (uint8_t*)malloc(RAM_SIZE);
}

AtariMachine::~AtariMachine()
{
	if (m_RAM)
	{
		free(m_RAM);
		m_RAM = NULL;
	}
}

static int	fIllegalCb(int opcode)
{
	assert(false);
	m68k_end_timeslice();
	return 1;
}

/*
static void	fDebugCb(unsigned int pc)
{
	printf("$%06x .. \n", pc);
}
*/

static void	fResetCb(void)
{
	assert(gCurrentMachine);
	gCurrentMachine->ResetCb();
	m68k_end_timeslice();
}

void	AtariMachine::ResetCb(void)
{
	m_ExitCode |= AtariMachine::ExitCode::kReset;
}

extern "C"
{
	void	TrapInstructionCallback(int v)
	{
		assert(gCurrentMachine);
		gCurrentMachine->TrapInstructionCallback(v);
	}
}

void	AtariMachine::Gemdos(int func, uint32_t a7)
{
	switch (func)
	{
	case 0x48:			// MALLOC
	{
		// very basic incremental allocator (required by Maxymizer player)
		int size = m68k_read_memory_32(a7 + 2);
		m68k_set_reg(M68K_REG_D0, m_NextGemdosMallocAd);
		m_NextGemdosMallocAd = (m_NextGemdosMallocAd + size + 1)&(-2);
		assert(m_NextGemdosMallocAd <= RAM_SIZE);
	}
	break;
	case 0x30:			// system version
	{
		m68k_set_reg(M68K_REG_D0, 0x0000);	// 0.15 : TOS 1.04 & 1.06
	}
	break;

	default:
		assert(false);	// unsupported GEMDOS function
		break;
	}
}

void	AtariMachine::XbiosTimerSet(int ctrlPort, int dataPort, int enablePort, int bit, int mask, int ctrlValue, int dataValue)
{
	const uint8_t back = m_Mfp.Read8(ctrlPort)&mask;
	m_Mfp.Write8(ctrlPort, back | 0x0);
	m_Mfp.Write8(dataPort, dataValue);
	m_Mfp.Write8(ctrlPort, back | ctrlValue);
	// seems Atari BIOS always enable the timer (even when switching it off)
	m_Mfp.Write8(enablePort, m_Mfp.Read8(enablePort) | (1 << bit));
	m_Mfp.Write8(enablePort+12, m_Mfp.Read8(enablePort+12) | (1 << bit));
}

void	AtariMachine::XBios(int func, uint32_t a7)
{
	switch (func)
	{
	case 31:
	{
		uint16_t timer = m68k_read_memory_16(a7 + 2);
		uint16_t ctrlWord = m68k_read_memory_16(a7 + 4);
		uint16_t dataWord = m68k_read_memory_16(a7 + 6);
		uint32_t vector = m68k_read_memory_32(a7 + 8);
		if (timer < 4)
		{
			m68k_write_memory_32(ivector[timer], vector);
			switch (timer)
			{
			case 0:		// A
				XbiosTimerSet(0x19, 0x1f, 0x07, 5, 0x00, ctrlWord, dataWord);
				break;
			case 1:		// B
				XbiosTimerSet(0x1b, 0x21, 0x07, 0, 0x00, ctrlWord, dataWord);
				break;
			case 2:		// C
				XbiosTimerSet(0x1d, 0x23, 0x09, 5, 0x0f, (ctrlWord&0xf)<<4, dataWord);
				break;
			case 3:		// D
				XbiosTimerSet(0x1d, 0x25, 0x09, 4, 0xf0, ctrlWord&0xf, dataWord);
				break;
			default:
				assert(false);			// unknown timer!
				break;
			}
		}
	}
	break;
		case 38:
		{
			// XBios(38) -> execute callback code in supervisor
			// we just simulate a "jsr callback"
			uint32_t callbackAddr = m68k_read_memory_32(a7 + 2);

			// push PC on stack (so future RTS will get back right after the TRAP)
			uint32_t pc = m68k_get_reg(NULL, M68K_REG_PC);
			a7 -= 4;
			m68k_write_memory_32(a7, pc);
			m68k_set_reg(M68K_REG_SP, a7);
			m68k_set_reg(M68K_REG_PC, callbackAddr);
		}
		break;
	default:
		assert(false);	// unsupported XBIOS function
		break;
	}
}

void	AtariMachine::TrapInstructionCallback(int v)
{

	int a7 = m68k_get_reg(NULL, M68K_REG_SP);
	int func = m68k_read_memory_16(a7);

	switch (v)
	{
	case 1:
		Gemdos(func, a7);
		break;
	case 14:
		XBios(func, a7);
		break;
	default:
		assert(false);		// unsupported TRAP #n
		break;
	}
}

void	AtariMachine::Startup(uint32_t hostReplayRate)
{
	gCurrentMachine = this;
	assert(m_RAM);
	memset(m_RAM, 0, RAM_SIZE);

	m_Ym2149.Reset(hostReplayRate);
	m_Mfp.Reset(hostReplayRate);
	m_SteDac.Reset(hostReplayRate);
	m_NextGemdosMallocAd = GEMDOS_MALLOC_EMUL_BUFFER;

	m68k_set_cpu_type(M68K_CPU_TYPE_68000);
	m68k_init();
	m68k_set_illg_instr_callback(fIllegalCb);
	m68k_set_reset_instr_callback(fResetCb);
//	m68k_set_instr_hook_callback(fDebugCb);

	// setup some cookie jar for MaxyMizer player!
	m68k_write_memory_32(0x900, '_SND');
	m68k_write_memory_32(0x904, 0x3);		// soundchip+STE DMA
	m68k_write_memory_32(0x908, '_MCH');
	m68k_write_memory_32(0x90c, 0x00010000);	// STE
	m68k_write_memory_32(0x910, 0);			// end
	m68k_write_memory_32(0x5a0, 0x900);		// cookie jar start

	memWrite16(RESET_INSTRUCTION_ADDR, 0x4e70);			// 4e70=reset instruction
	memWrite16(RTE_INSTRUCTION_ADDR, 0x4e73);			// 4e73=rte

	// some SNDH setup MFP registers with values from OS, with timer C running!
	// so by default, set the timer C handler to RTE, just in case
	m68k_write_memory_32(0x114, RTE_INSTRUCTION_ADDR);

	gCurrentMachine = NULL;
}

bool	AtariMachine::Upload(const void* src, uint32_t addr, uint32_t size)
{
	if (addr + size > RAM_SIZE)
		return false;

	if ((NULL == src) || (0 == size))
		return false;

	memcpy(m_RAM + addr, src, size);
	return true;
}

void	AtariMachine::ConfigureReturnByRts()
{
	m68k_write_memory_32(RAM_SIZE - 4, RESET_INSTRUCTION_ADDR);		// next RTS will go to RESET_INSTRUCTION_ADDR (reset)
	m68k_write_memory_32(0, RAM_SIZE-4);				// stack ptr at next reset on TOP of RAM
}

void	AtariMachine::ConfigureReturnByRte()
{
	m68k_write_memory_32(RAM_SIZE - 4, RESET_INSTRUCTION_ADDR);		// next RTE will go to RESET_INSTRUCTION_ADDR (reset)
	memWrite16(RAM_SIZE - 6, 0x2300);					// SR=2300
	m68k_write_memory_32(0, RAM_SIZE - 6);				// stack ptr at next reset on TOP of RAM
}

bool	AtariMachine::JmpBinary(int pc, int timeOut50Hz)
{
	m68k_write_memory_32(0x14, RTE_INSTRUCTION_ADDR);		// DIV by ZERO excep jump at $500

	m68k_write_memory_32(4, pc);			// pc at next RESET
	m68k_pulse_reset();						// reset CPU & start execution at PC

	m_ExitCode = 0;
	int cycles = 0;
	for (int t = 0; t < timeOut50Hz; t++)
	{
		cycles += m68k_execute(512 * 313);				// 50hz frame
		if (m_ExitCode)
			break;
	}
	return (kReset == m_ExitCode);
}

bool	AtariMachine::Jsr(uint32_t addr, uint32_t d0)
{
	gCurrentMachine = this;

	bool ret = false;
	// upload data in RAM
	ConfigureReturnByRts();
	m68k_set_reg(M68K_REG_D0, d0);
	ret = JmpBinary(addr, 50*10);		// timeout of 1sec for init
	gCurrentMachine = NULL;
	return ret;
}

int16_t	AtariMachine::ComputeNextSample(uint32_t* pSampleDebugInfo)
{
	gCurrentMachine = this;
	int32_t level = m_Ym2149.ComputeNextSample(pSampleDebugInfo);
	int32_t steLevel = m_SteDac.ComputeNextSample((const int8_t*)m_RAM, RAM_SIZE, m_Mfp);
	if (steLevel && (pSampleDebugInfo))
		*pSampleDebugInfo |= (steLevel >> 8) << 24;
	level += steLevel;

	if (level > 32767)
		level = 32767;
	else if (level < -32768)
		level = -32768;

	int16_t out = (int16_t)level;

	// tick 4 Atari timers, maybe one of them is running
	for (int t = 0; t < 4+1; t++)
	{
		if (m_Mfp.Tick(t))
		{
			uint32_t pc = m68k_read_memory_32(ivector[t]);
			ConfigureReturnByRte();
			m_Ym2149.InsideTimerIrq(true);
			JmpBinary(pc, 1);	// execute the timer code until RTE (probably SID or any other special fx code)
			m_Ym2149.InsideTimerIrq(false);
		}
	}
	gCurrentMachine = NULL;
	return out;
}
