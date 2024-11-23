/*--------------------------------------------------------------------
	Atari Audio Library
	Small & accurate ATARI-ST audio emulation
	Arnaud Carré aka Leonard/Oxygene
	@leonard_coder
--------------------------------------------------------------------*/
#pragma once
#include <stdint.h>

class Ym2149c
{
public:

	void	Reset(uint32_t hostReplayRate, uint32_t ymClock = 2000000);
	void	WritePort(uint8_t port, uint8_t value);
	uint8_t ReadPort(uint8_t port) const;
	int16_t	ComputeNextSample(uint32_t* pSampleDebugInfo = NULL);
	void	InsideTimerIrq(bool inside);

private:
	void	WriteReg(int reg, uint8_t value);
	uint16_t Tick();

	static const uint32_t kDcAdjustHistoryBit = 11;	// 2048 values (~20ms at 44Khz) 

	int16_t		dcAdjust(uint16_t v);

	int			m_selectedReg;
	const uint8_t* m_pCurrentEnv;
	uint32_t	m_ymClockOneEighth;
	uint32_t	m_resamplingDividor;
	uint32_t	m_hostReplayRate;
	uint32_t	m_toneCounter[3];
	uint32_t	m_tonePeriod[3];
	uint32_t	m_toneEdges;

	uint32_t	m_envCounter;
	int			m_envPos;
	uint32_t	m_envPeriod;
	uint32_t	m_noiseCounter;
	uint32_t	m_noisePeriod;
	uint32_t	m_toneMask;
	uint32_t	m_noiseMask;
	uint32_t	m_noiseRndRack;
	uint32_t	m_currentNoiseMask;
	uint16_t	m_dcAdjustBuffer[1<<kDcAdjustHistoryBit];
	unsigned int	m_dcAdjustPos;
	uint32_t	m_dcAdjustSum;
	uint8_t		m_regs[14];
	uint32_t	m_currentLevel;
	uint32_t	m_innerCycle;
	uint32_t 	m_noiseEnvHalf;
	uint32_t	m_currentDebugThreeVoices;
	bool		m_insideTimerIrq;
	bool		m_edgeNeedReset[3];
};
