/*--------------------------------------------------------------------
	Atari Audio Library
	Small & accurate ATARI-ST audio emulation
	Arnaud Carré aka Leonard/Oxygene
	@leonard_coder
--------------------------------------------------------------------*/
#pragma once
#include <stdint.h>

class Mk68901
{

public:
	void	Reset(uint32_t hostReplayRate);
	bool	Tick(int timerId);

	enum eTimerName
	{
		eTimerA = 0,
		eTimerB,
		eTimerC,
		eTimerD,
		eGpi7,
	};

	uint8_t		Read8(int port);
	uint16_t	Read16(int port);
	void		Write8(int port, uint8_t data);				// Write a 8bits value to a MFP register
	void		Write16(int port, uint16_t data);				// Write a 16bits value on MFP register
	void		SetSteDacExternalEvent() { m_timers[eTimerA].externalEvent = true; m_timers[eGpi7].externalEvent = true; }

private:
	struct Timer
	{
		bool	enable;
		bool	mask;
		int		controlRegister;
		uint8_t	dataRegister;
		uint8_t	dataRegisterInit;
		uint32_t innerClock;
		bool	externalEvent;

		void	Reset();
		bool	Tick(uint32_t hostReplayRate);
		void	SetER(bool _enable);
		void	SetDR(uint8_t data);
		void	SetCR(uint8_t data) { controlRegister = data; }
		void	SetMR(bool _mask) { mask = _mask; }
		void	Restart();
		bool	IsCounterMode() const { return ((controlRegister & 7) && (0 == (controlRegister & 8))); }

	};

	uint32_t m_hostReplayRate;
	uint8_t	m_regs[256];
	Timer	m_timers[5];
};

