/*--------------------------------------------------------------------
	Atari Audio Library
	Small & accurate ATARI-ST audio emulation
	Arnaud Carré aka Leonard/Oxygene
	@leonard_coder
--------------------------------------------------------------------*/
#include "SteDac.h"
#include "Mk68901.h"

static const uint32_t kSTE_DAC_Frq = 50066;

void	SteDac::Reset(uint32_t hostReplayRate)
{
	for (int i = 0; i < 256; i++)
		m_regs[i] = 0;
	m_hostReplayRate = hostReplayRate;
	m_samplePtr = 0;
	m_innerClock = 0;
	m_microwireMask = 0;
	m_microwireShift = 0;
	m_microwireData = 0;
	m_masterVolume = 64;
	m_currentDacLevel = 0;
	m_50Acc = 0;
	m_50to25 = false;
}

void	SteDac::FetchSamplePtr()
{
	m_samplePtr = (m_regs[3] << 16) | (m_regs[5] << 8) | (m_regs[7]&0xfe);
	m_sampleEndPtr = (m_regs[0xf] << 16) | (m_regs[0x11] << 8) | (m_regs[0x13]&0xfe);
}

void	SteDac::Write8(int ad, uint8_t data)
{
	ad &= 0xff;
	if (ad & 1)
	{
		switch (ad)
		{
		case 0x1:
		{
			if ((data & 1) && ((data^m_regs[1]) & 1))
			{
				// replay just started
				FetchSamplePtr();
			}
		}
		break;
		case 0x7:
		case 0xd:
			data &= 0xfe;
			case 0x21:
				if ((data & 3) != (m_regs[0x21]&3))
				{
					m_50Acc = 0;
					m_50to25 = false;
				}
			break;
		default:
			break;
		}

		m_regs[ad] = data;
	}
}

void	SteDac::Write16(int ad, uint16_t data)
{
	ad &= 0xff;
	if (0 == (ad & 1))
	{
		switch (ad)
		{
		case 0x22:
			m_microwireData = data;
			MicrowireProceed();
			m_microwireShift = 16;
			break;
		case 0x24:
			m_microwireMask = data;
			break;
		default:
			Write8(ad + 1, uint8_t(data));
			break;
		}
	}
}

uint8_t SteDac::Read8(int ad)
{
	ad &= 0xff;
	uint8_t data = ~0;
	if (ad & 1)
	{
		data = m_regs[ad];
		switch (ad)
		{
		case 0x09:	data = uint8_t(m_samplePtr >> 16);	break;
		case 0x0b:	data = uint8_t(m_samplePtr >> 8);	break;
		case 0x0d:	data = uint8_t(m_samplePtr >> 0);	break;
		}
	}
	return data;
}

uint16_t	SteDac::Read16(int ad)
{
	ad &= 0xff;
	uint16_t data = ~0;
	if (0 == (ad & 1))
	{
		switch (ad)
		{
		case 0x22:
			data = m_microwireData;
			break;
		case 0x24:
			data = MicrowireTick();
			break;
		default:
			data = 0xff00 | Read8(ad + 1);
			break;
		}
	}
	return data;
}

int8_t	SteDac::FetchSample(const int8_t* atariRam, uint32_t ramSize, uint32_t atariAd)
{
	if (atariAd < ramSize)
		return atariRam[atariAd];
	return 0;
}

int16_t	SteDac::ComputeNextSample(const int8_t* atariRam, uint32_t ramSize, Mk68901& mfp)
{
	// supports tricky Tao "MS3" driver. Seems to be a 3 or 4 voices synth, without need of mixing code!
	// the 4 voices are just output in 4 consecutive bytes. Everything is playing at 50Khz, stereo
	// On real hardware with analog filters & friends, it "sounds" like if you mixed 4 voices at 25Khz
	//
	// ComputeNextSample is called at host rate
	// but the while loop is running at DAC speed. In 50khz mode, 2 samples are accumulated before 
	// output. So you get a mixed stream at 25Khz. None of original atari samples are missed, and
	// Tao MS3 songs are playing ok!
	// Please note it also works perfectly with Quartet STE code, that is mixing into a 2 bytes 50Khz buffer!! :)
	static	const uint32_t	sDacFreq[4] = { kSTE_DAC_Frq / 8 , kSTE_DAC_Frq / 4 , kSTE_DAC_Frq / 2 , kSTE_DAC_Frq / 1 };
	if (m_regs[1] & 1)
	{
		m_innerClock += sDacFreq[m_regs[0x21] & 3];
		const bool stereo = (0 == (m_regs[0x21] & 0x80));
		const bool b50k = (3 == (m_regs[0x21]&3));
	
		while (m_innerClock >= m_hostReplayRate)
		{
			if (m_samplePtr == m_sampleEndPtr)
			{
				mfp.SetSteDacExternalEvent();
				FetchSamplePtr();
				if ((m_regs[0x1] & (1 << 1)) == 0)
				{
					// if no loop mode, switch off replay
					m_regs[0x1] &= 0xfe;
					m_currentDacLevel = 0;
					break;
				}
			}
			
			int16_t level = FetchSample(atariRam, ramSize, m_samplePtr);
			if (stereo)
				level += FetchSample(atariRam, ramSize, m_samplePtr + 1);
	
			if (b50k)
			{
				m_50Acc += level;
				m_50to25 ^= true;
				if (!m_50to25)
				{
					m_currentDacLevel = (m_50Acc * m_masterVolume)>>1;
					m_50Acc = 0;
				}
			}
			else
			{
				m_currentDacLevel = level * m_masterVolume;
			}
	
			m_samplePtr += stereo?2:1;
			m_innerClock -= m_hostReplayRate;
		}
	}
	else
		m_currentDacLevel = 0;
	return m_currentDacLevel;
}

// emulate internal rol to please any user 68k code reading & waiting the complete cycle
uint16_t	SteDac::MicrowireTick()
{
	if (m_microwireShift > 0)
	{
		// rol.w #1
		m_microwireMask = (m_microwireMask << 1) | (m_microwireMask >> 15);
		m_microwireShift--;
	}
	return m_microwireMask;
}

void	SteDac::MicrowireProceed()
{
	uint16_t value = 0;
	int count = 0;
	for (int i = 0; i < 16; i++)
	{
		if (m_microwireMask&(1 << i))
		{
			if (m_microwireData&(1 << i))
				value |= (1 << count);
			count++;
		}
	}

	if (11 == count)
	{
		if (2 == (value >> 9))
		{
			const int data = value & 0x3f;
			switch ((value >> 6) & 7)
			{
			case 3:	m_masterVolume = (data > 40) ? 64 : (data*64)/40;	break;
			default:
				break;
			}
		}
	}
}
