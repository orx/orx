/*--------------------------------------------------------------------
	Atari Audio Library
	Small & accurate ATARI-ST audio emulation
	Arnaud Carré aka Leonard/Oxygene
	@leonard_coder
--------------------------------------------------------------------*/
#pragma once
#include <stdint.h>
#include "ym2149c.h"
#include "Mk68901.h"
#include "SteDac.h"

static	const	uint32_t	RAM_SIZE = 4*1024*1024;
static	const	uint32_t	RTE_INSTRUCTION_ADDR = 0x500;
static	const	uint32_t	RESET_INSTRUCTION_ADDR = 0x502;
static	const	uint32_t	SNDH_UPLOAD_ADDR = 0x10002;		// some SNDH can't play below (ie SynthDream2) Also some driver crash if loaded at 64KiB bound ( metal planet by Floopy at 1:44 )
static	const	uint32_t	GEMDOS_MALLOC_EMUL_BUFFER = RAM_SIZE-0x100000;


class AtariMachine
{
public:
	AtariMachine();
	~AtariMachine();

	enum ExitCode
	{
		kCrash = (1 << 0),
		kReset = (1 << 1),
	};

	void		Startup(uint32_t hostReplayRate);
	bool		Upload(const void* src, uint32_t addr, uint32_t size);
	bool		Jsr(uint32_t addr, uint32_t d0);
	int16_t		ComputeNextSample(uint32_t* pSampleDebugInfo = NULL);

	unsigned int	memRead8(unsigned int address);
	unsigned int	memRead16(unsigned int address);
	void			memWrite8(unsigned int address, unsigned int value);
	void			memWrite16(unsigned int address, unsigned int value);
	void			TrapInstructionCallback(int v);
	void			ResetCb(void);


private:
	void		ConfigureReturnByRts();
	void		ConfigureReturnByRte();
	bool		JmpBinary(int pc, int timeOut50Hz);
	void		Gemdos(int func, uint32_t a7);
	void		XBios(int func, uint32_t a7);
	void		XbiosTimerSet(int ctrlPort, int dataPort, int enablePort, int bit, int mask, int ctrlValue, int dataValue);

	uint8_t*	m_RAM;
	int			m_ExitCode;
	uint32_t	m_NextGemdosMallocAd;
	Ym2149c		m_Ym2149;
	Mk68901		m_Mfp;
	SteDac		m_SteDac;

};
