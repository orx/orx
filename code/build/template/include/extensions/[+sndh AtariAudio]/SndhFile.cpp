/*--------------------------------------------------------------------
	Atari Audio Library
	Small & accurate ATARI-ST audio emulation
	by Arnaud Carré aka Leonard/Oxygene
	@leonard_coder
--------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "SndhFile.h"
#include "external/ice_24.h"

SndhFile::SndhFile()
{
	m_rawBuffer = NULL;
	m_Title = NULL;
	m_Author = NULL;
	m_sYear = NULL;
	m_rawSize = 0;
}

SndhFile::~SndhFile()
{
	Unload();
}

void	SndhFile::Unload()
{
	free((void*)m_rawBuffer);
	free(m_Title);
	free(m_Author);
	free(m_sYear);
	m_bLoaded = false;
	m_rawBuffer = NULL;
	m_Title = NULL;
	m_Author = NULL;
	m_sYear = NULL;
	m_rawSize = 0;
	m_playerRate = 0;
	m_subSongCount = -1;
}

uint16_t	SndhFile::Read16(const char* r)
{
	assert(m_rawBuffer);
	assert(r+2 <= (const char*)m_rawBuffer+m_rawSize);
	const uint8_t* r8 = (const uint8_t*)r;
	uint16_t v = (r8[0] << 8) | (r8[1]);
	return v;
}

const char* SndhFile::skipNTString(const char* r)
{
	r += strlen(r) + 1;
	return r;
}

bool	SndhFile::Load(const void* rawSndhFile, int sndhFileSize, uint32_t hostReplayRate)
{

	Unload();
	m_hostReplayRate = hostReplayRate;
	bool ret = false;
	if (ice_24_header((unsigned char*)rawSndhFile))
	{
		m_rawSize = (int)ice_24_origsize((unsigned char*)rawSndhFile);
		m_rawBuffer = malloc(m_rawSize);
		long csize = ice_24_depack((unsigned char*)rawSndhFile, (unsigned char*)m_rawBuffer);
		if (m_rawSize != csize)
		{
			Unload();
			return false;
		}
	}
	else
	{
		m_rawSize = sndhFileSize;
		m_rawBuffer = malloc(m_rawSize);
		memcpy((void*)m_rawBuffer, rawSndhFile, sndhFileSize);
	}

	for (int i = 0; i < kSubsongCountMax; i++)
		m_subSongLen[i] = 0;

	const char* read8 = (const char*)m_rawBuffer;
	if (m_rawSize > 16)
	{
		if ((0x60 == read8[0]) && (0 == strncmp(read8 + 12, "SNDH", 4)))
		{
			int headerSize = Read16(read8 + 2) + 2; // suppose it's bra.w
			if (read8[1])
				headerSize = read8[1] + 2;			// but maybe it's bra.s
			const char* readEnd = read8 + headerSize;

			m_playerRate = 50;
			m_defaultSubSong = 1;
			m_subSongCount = 1;

			read8 += 16;
			while (read8 + 4 <= readEnd)
			{
				if (0 == strncmp(read8, "!#SN", 4))
				{
					assert(m_subSongCount > 0);
					read8 += 4 + m_subSongCount * 2;			// skip 2bytes per offset
				}
				if (0 == strncmp(read8, "!#", 2))
				{
					m_defaultSubSong = atoi(read8 + 2);
					read8 = skipNTString(read8+2);
				}
				else if (0 == strncmp(read8, "TITL", 4))
				{
					m_Title = _strdup(read8 + 4);
					read8 = skipNTString(read8 + 4);
				}
				else if (0 == strncmp(read8, "COMM", 4))
				{
					m_Author = _strdup(read8 + 4);
					read8 = skipNTString(read8 + 4);
				}
				else if (	(0 == strncmp(read8, "RIPP", 4)) ||
							(0 == strncmp(read8, "CONV", 4)))
				{
					read8 = skipNTString(read8 + 4);
				}
				else if ((0 == strncmp(read8, "YEAR", 4)))
				{
					if ( read8[4] != 0)
						m_sYear = _strdup(read8 + 4);	// many sndh files have "" as year string
					read8 = skipNTString(read8 + 4);
				}
				else if (0 == strncmp(read8, "##", 2))
				{
					char stemp[3];
					memcpy(stemp, read8 + 2, 2);
					stemp[2] = 0;
					m_subSongCount = atoi(stemp);
					if (m_subSongCount <= 0)	// some SNDH files have broken ## tag
						m_subSongCount = 1;
					read8 += 4;
				}
				else if (0 == strncmp(read8, "TIME", 4))
				{
					assert(m_subSongCount > 0);
					read8 += 4;
					for (int i = 0; i < m_subSongCount; i++)
					{
						m_subSongLen[i] = Read16(read8);
						read8 += 2;
					}
				}
				else if (0 == strncmp(read8, "HDNS", 4))
				{
					break;
				}
				else if (	(0 == strncmp(read8, "TA", 2)) ||
							(0 == strncmp(read8, "TB", 2)) ||
							(0 == strncmp(read8, "TC", 2)) ||
							(0 == strncmp(read8, "TD", 2)) ||
							(0 == strncmp(read8, "!V", 2)))
				{
					m_playerRate = atoi(read8 + 2);
					read8 = skipNTString(read8 + 2);
				}
				else
				{
					read8++;
				}
			}

			if ((m_defaultSubSong > m_subSongCount) ||
				(m_defaultSubSong < 1))
				m_defaultSubSong = 1;

			ret = true;
		}
	}

	if ( !ret )
		Unload();

	m_bLoaded = ret;
	return ret;
}

int	SndhFile::GetSubsongCount() const
{
	if (!m_bLoaded)
		return 0;
	return m_subSongCount;
}

bool	SndhFile::GetSubsongInfo(int subSongId, SubSongInfo& out) const
{
	if (!m_bLoaded)
		return false;
	if ((subSongId <= 0) || (subSongId > m_subSongCount))
		return false;

	const int songLen = m_subSongLen[subSongId - 1];
	out.playerTickCount = songLen * m_playerRate;
	out.playerTickRate = m_playerRate;
	out.samplePerTick = m_hostReplayRate / m_playerRate;
	out.musicName = m_Title;
	out.musicAuthor = m_Author;
	out.year = m_sYear;

	out.subsongCount = m_subSongCount;
	return true;
}

bool	SndhFile::InitSubSong(int subSongId)
{
	bool ret = false;
	SubSongInfo info;
	if (!GetSubsongInfo(subSongId, info))
		return false;
	m_samplePerTick = m_hostReplayRate / m_playerRate;
	m_innerSamplePos = 0;
	m_frame = 0;
	m_frameCount = info.playerTickCount;
	m_loopCount = 0;
	m_atariMachine.Startup(m_hostReplayRate);
	if (m_atariMachine.Upload(m_rawBuffer, SNDH_UPLOAD_ADDR, m_rawSize))
	{
		ret = m_atariMachine.Jsr(SNDH_UPLOAD_ADDR, subSongId);
	}
	return ret;
}

int	SndhFile::AudioRender(int16_t* buffer, int count, uint32_t* pSampleViewInfo)
{
	for (int i = 0; i < count; i++)
	{
		m_innerSamplePos--;
		// check if we should call SNDH music driver tick (most of the time 50hz)
		if (m_innerSamplePos <= 0)
		{
			m_atariMachine.Jsr(SNDH_UPLOAD_ADDR + 8, 0);
			m_innerSamplePos = m_samplePerTick;
			m_frame++;
			if (m_frameCount != 0 && m_frame >= m_frameCount)
			{
				m_frame -= m_frameCount;
				m_loopCount++;
			}
		}

		// compute the Atari machine sample (YM2149 and STE DAC)
		*buffer++ = m_atariMachine.ComputeNextSample(pSampleViewInfo);
		if (pSampleViewInfo)
			pSampleViewInfo++;
	}
	return m_loopCount;
}
