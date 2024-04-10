/*--------------------------------------------------------------------
	Atari Audio Library
	Small & accurate ATARI-ST audio emulation
	Arnaud Carré aka Leonard/Oxygene
	@leonard_coder
--------------------------------------------------------------------*/
#include <assert.h>
#include "Mk68901.h"
#include "external/Musashi/m68k.h"

static const uint32_t	kAtariMfpClock = 2457600;

void	Mk68901::Reset(uint32_t hostReplayRate)
{
	for (int i = 0; i < 256; i++)
		m_regs[i] = 0;

	for (int t = 0; t < 5; t++)
		m_timers[t].Reset();

	// by default on Atari OS timer C is enable (and even running, but we just enable)
	m_timers[eTimerC].enable = true;
	m_timers[eTimerC].mask = true;

	// gpi7 is not really a timer, "simulate" an event type timer with count=1 to make the code simpler
	m_timers[eGpi7].controlRegister = 1<<3;	// simulate event mode
	m_timers[eGpi7].dataRegisterInit = 1;	// event count always 1
	m_timers[eGpi7].dataRegister = 1;

	m_hostReplayRate = hostReplayRate;
}

void	Mk68901::Timer::Reset()
{
	controlRegister = 0;
	dataRegister = 0;
	enable = false;
	mask = false;
	innerClock = 0;
	externalEvent = false;
}

void	Mk68901::Timer::Restart()
{
	innerClock = 0;
	dataRegister = dataRegisterInit;
}

void	Mk68901::Timer::SetER(bool _enable)
{
	if ((enable ^ _enable) && (_enable && IsCounterMode()))
		Restart();
	enable = _enable;
}

void	Mk68901::Timer::SetDR(uint8_t data)
{
	dataRegisterInit = data;
	if (0 == controlRegister)
		Restart();
}

void	Mk68901::Write8(int port,uint8_t data)
{
	port &= 255;
	assert(1 == (port&1));
	if (port & 1)
	{
		switch (port)
		{
		case 0x19:
			m_timers[eTimerA].SetCR(data&0xf);
			break;
		case 0x1b:
			m_timers[eTimerB].SetCR(data&0xf);
			break;
		case 0x1d:
			m_timers[eTimerC].SetCR((data >> 4) & 7);
			m_timers[eTimerD].SetCR(data & 7);
			break;

		case 0x1f:
		case 0x21:
		case 0x23:
		case 0x25:
		{
			int timerId = (port - 0x1f) >> 1;
			m_timers[timerId].SetDR(data);
		}
		break;

		case 0x07:
			m_timers[eTimerA].SetER((data&(1 << 5)) != 0);
			m_timers[eTimerB].SetER((data&(1 << 0)) != 0);
			m_timers[eGpi7].SetER((data&(1 << 7)) != 0);
			break;
		case 0x09:
			m_timers[eTimerC].SetER((data&(1 << 5)) != 0);
			m_timers[eTimerD].SetER((data&(1 << 4)) != 0);
			break;
		case 0x13:
			m_timers[eTimerA].SetMR((data&(1 << 5)) != 0);
			m_timers[eTimerB].SetMR((data&(1 << 0)) != 0);
			m_timers[eGpi7].SetMR((data&(1 << 7)) != 0);
			break;
		case 0x15:
			m_timers[eTimerC].SetMR((data&(1 << 5)) != 0);
			m_timers[eTimerD].SetMR((data&(1 << 4)) != 0);
			break;
		default:
			break;
		}
		m_regs[port] = data;
	}

// 	uint32_t pc = m68k_get_reg(NULL, M68K_REG_PC);
// 	printf("$%06x: Write $%02x in $fffffa%02x\n", pc, data, ad);

}

uint8_t	Mk68901::Read8(int port)
{
	port &= 255;
	uint8_t data = ~0;
	if (port & 1)
	{
		data = m_regs[port];
		switch (port)
		{
		case 0x01:
			data = (m_regs[0x01] & 0x7f) | 0x80;
			break;

		case 0x1f:
		case 0x21:
		case 0x23:
		case 0x25:
		{
			const int timerId = (port - 0x1f) >> 1;
			data = m_timers[timerId].dataRegister;
		}
		break;

		default:
			break;
		}
	}

// 	uint32_t pc = m68k_get_reg(NULL, M68K_REG_PC);
// 	printf("$%06x: Read $fffffa%02x, returns $%02x\n", pc, ad, data);

	return data;
}

uint16_t	Mk68901::Read16(int ad)
{
	return uint16_t(0xff00) | Read8(ad + 1);
}

void	Mk68901::Write16(int ad,uint16_t data)
{
	Write8(ad + 1, uint8_t(data));
}

bool	Mk68901::Tick(int timerId)
{
	return m_timers[timerId].Tick(m_hostReplayRate);
}

bool	Mk68901::Timer::Tick(uint32_t hostReplayRate)
{
	bool ret = false;
	if (enable)
	{
		if (controlRegister&(1 << 3))	// event mode
		{
			if (externalEvent)
			{
				dataRegister--;
				if (0 == dataRegister)
				{
					dataRegister = dataRegisterInit;
					ret = true;
				}
				externalEvent = false;
			}
		}
		else if (controlRegister & 7)
		{
			// timer counter mode
			static	const	int	s_Prescale[8] = { 0,kAtariMfpClock / 4,kAtariMfpClock / 10,kAtariMfpClock / 16,kAtariMfpClock / 50,kAtariMfpClock / 64,kAtariMfpClock / 100,kAtariMfpClock / 200 };
			innerClock += s_Prescale[controlRegister & 7];
			// most of the time this while will never loop
			while (innerClock >= hostReplayRate)
			{
				dataRegister--;
				if (0 == dataRegister)
				{
					dataRegister = dataRegisterInit;
					ret = true;
				}
				innerClock -= hostReplayRate;
			}
		}
	}

	return ret && mask;
}
