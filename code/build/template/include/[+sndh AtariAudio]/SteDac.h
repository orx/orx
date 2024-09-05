/*--------------------------------------------------------------------
	Atari Audio Library
	Small & accurate ATARI-ST audio emulation
	Arnaud Carré aka Leonard/Oxygene
	@leonard_coder
--------------------------------------------------------------------*/
#pragma once
#include <stdint.h>

class Mk68901;
class SteDac
{
public:
	void		Reset(uint32_t hostReplayRate);

	void		Write8(int ad, uint8_t data);
	void		Write16(int ad, uint16_t data);
	uint8_t		Read8(int ad);
	uint16_t	Read16(int ad);

	int16_t		ComputeNextSample(const int8_t* atariRam, uint32_t ramSize, Mk68901& mfp);

private:
	void		FetchSamplePtr();
	int8_t		FetchSample(const int8_t* atariRam, uint32_t ramSize, uint32_t atariAd);
	uint16_t	MicrowireTick();
	void		MicrowireProceed();
	uint32_t	m_hostReplayRate;
	uint32_t	m_samplePtr;
	uint32_t	m_sampleEndPtr;
	uint32_t	m_innerClock;
	uint16_t	m_microwireMask;
	uint16_t	m_microwireData;
	int			m_microwireShift;
	uint8_t		m_regs[256];
	int			m_masterVolume;
	bool m_50to25;
	int m_50Acc;
	int16_t m_currentDacLevel;
};
