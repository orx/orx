#include <math.h>
#include <stdarg.h>
#include <stdio.h>

extern void exit(int);

static void fatalerror(char *format, ...) {
      va_list ap;
      va_start(ap,format);
      fprintf(stderr,format,ap);
      va_end(ap);
      exit(1);
}

#define FPCC_N			0x08000000
#define FPCC_Z			0x04000000
#define FPCC_I			0x02000000
#define FPCC_NAN		0x01000000

#define DOUBLE_INFINITY					(unsigned long long)(0x7ff0000000000000)
#define DOUBLE_EXPONENT					(unsigned long long)(0x7ff0000000000000)
#define DOUBLE_MANTISSA					(unsigned long long)(0x000fffffffffffff)

INLINE void SET_CONDITION_CODES(fp_reg reg)
{
	REG_FPSR &= ~(FPCC_N|FPCC_Z|FPCC_I|FPCC_NAN);

	// sign flag
	if (reg.i & (unsigned long long)(0x8000000000000000))
	{
		REG_FPSR |= FPCC_N;
	}

	// zero flag
	if ((reg.i & (unsigned long long)(0x7fffffffffffffff)) == 0)
	{
		REG_FPSR |= FPCC_Z;
	}

	// infinity flag
	if ((reg.i & (unsigned long long)(0x7fffffffffffffff)) == DOUBLE_INFINITY)
	{
		REG_FPSR |= FPCC_I;
	}

	// NaN flag
	if (((reg.i & DOUBLE_EXPONENT) == DOUBLE_EXPONENT) && ((reg.i & DOUBLE_MANTISSA) != 0))
	{
		REG_FPSR |= FPCC_NAN;
	}
}

INLINE int TEST_CONDITION(int condition)
{
	int n = (REG_FPSR & FPCC_N) != 0;
	int z = (REG_FPSR & FPCC_Z) != 0;
	int nan = (REG_FPSR & FPCC_NAN) != 0;
	int r = 0;
	switch (condition)
	{
		case 0x00:		return 0;							// False
		case 0x01:		return (z);							// Equal
		case 0x0e:		return (!z);						// Not Equal
		case 0x0f:		return 1;							// True
		case 0x12:		return (!(nan || z || n));			// Greater Than
		case 0x13:		return (z || !(nan || n));			// Greater or Equal
		case 0x14:		return (n && !(nan || z));			// Less Than
		case 0x15:		return (z || (n && !nan));			// Less Than or Equal
		case 0x1a:		return (nan || !(n || z));			// Not Less Than or Equal
		case 0x1b:		return (nan || z || !n);			// Not Less Than
		case 0x1c:		return (nan || (n && !z));			// Not Greater or Equal Than
		case 0x1d:		return (nan || z || n);				// Not Greater Than

		default:		fatalerror("M68040: test_condition: unhandled condition %02X\n", condition);
	}

	return r;
}

static uint8 READ_EA_8(int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:		// Dn
		{
			return REG_D[reg];
		}
		case 5:		// (d16, An)
		{
			uint32 ea = EA_AY_DI_8();
			return m68ki_read_8(ea);
		}
		case 6:		// (An) + (Xn) + d8
		{
			uint32 ea = EA_AY_IX_8();
			return m68ki_read_8(ea);
		}
		case 7:
		{
			switch (reg)
			{
				case 1:		// (xxx).L
				{
					uint32 d1 = OPER_I_16();
					uint32 d2 = OPER_I_16();
					uint32 ea = (d1 << 16) | d2;
					return m68ki_read_8(ea);
				}
				case 4:		// #<data>
				{
					return  OPER_I_8();
				}
				default:	fatalerror("MC68040: READ_EA_8: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: READ_EA_8: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}

	return 0;
}

static uint16 READ_EA_16(int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:		// Dn
		{
			return (uint16)(REG_D[reg]);
		}
		case 2:		// (An)
		{
			uint32 ea = REG_A[reg];
			return m68ki_read_16(ea);
		}
		case 5:		// (d16, An)
		{
			uint32 ea = EA_AY_DI_16();
			return m68ki_read_16(ea);
		}
		case 6:		// (An) + (Xn) + d8
		{
			uint32 ea = EA_AY_IX_16();
			return m68ki_read_16(ea);
		}
		case 7:
		{
			switch (reg)
			{
				case 1:		// (xxx).L
				{
					uint32 d1 = OPER_I_16();
					uint32 d2 = OPER_I_16();
					uint32 ea = (d1 << 16) | d2;
					return m68ki_read_16(ea);
				}
				case 4:		// #<data>
				{
					return OPER_I_16();
				}

				default:	fatalerror("MC68040: READ_EA_16: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: READ_EA_16: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}

	return 0;
}

static uint32 READ_EA_32(int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:		// Dn
		{
			return REG_D[reg];
		}
		case 2:		// (An)
		{
			uint32 ea = REG_A[reg];
			return m68ki_read_32(ea);
		}
		case 3:		// (An)+
		{
			uint32 ea = EA_AY_PI_32();
			return m68ki_read_32(ea);
		}
		case 5:		// (d16, An)
		{
			uint32 ea = EA_AY_DI_32();
			return m68ki_read_32(ea);
		}
		case 6:		// (An) + (Xn) + d8
		{
			uint32 ea = EA_AY_IX_32();
			return m68ki_read_32(ea);
		}
		case 7:
		{
			switch (reg)
			{
				case 1:		// (xxx).L
				{
					uint32 d1 = OPER_I_16();
					uint32 d2 = OPER_I_16();
					uint32 ea = (d1 << 16) | d2;
					return m68ki_read_32(ea);
				}
				case 2:		// (d16, PC)
				{
					uint32 ea = EA_PCDI_32();
					return m68ki_read_32(ea);
				}
				case 4:		// #<data>
				{
					return  OPER_I_32();
				}
				default:	fatalerror("MC68040: READ_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: READ_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}
	return 0;
}

static void WRITE_EA_32(int ea, uint32 data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 0:		// Dn
		{
			REG_D[reg] = data;
			break;
		}
		case 2:		// (An)
		{
			uint32 ea = REG_A[reg];
			m68ki_write_32(ea, data);
			break;
		}
		case 3:		// (An)+
		{
			uint32 ea = EA_AY_PI_32();
			m68ki_write_32(ea, data);
			break;
		}
		case 4:		// -(An)
		{
			uint32 ea = EA_AY_PD_32();
			m68ki_write_32(ea, data);
			break;
		}
		case 5:		// (d16, An)
		{
			uint32 ea = EA_AY_DI_32();
			m68ki_write_32(ea, data);
			break;
		}
		case 6:		// (An) + (Xn) + d8
		{
			uint32 ea = EA_AY_IX_32();
			m68ki_write_32(ea, data);
			break;
		}
		case 7:
		{
			switch (reg)
			{
				case 1:		// (xxx).L
				{
					uint32 d1 = OPER_I_16();
					uint32 d2 = OPER_I_16();
					uint32 ea = (d1 << 16) | d2;
					m68ki_write_32(ea, data);
					break;
				}
				case 2:		// (d16, PC)
				{
					uint32 ea = EA_PCDI_32();
					m68ki_write_32(ea, data);
					break;
				}
				default:	fatalerror("MC68040: WRITE_EA_32: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: WRITE_EA_32: unhandled mode %d, reg %d, data %08X at %08X\n", mode, reg, data, REG_PC);
	}
}

static uint64 READ_EA_64(int ea)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);
	uint32 h1, h2;

	switch (mode)
	{
		case 2:		// (An)
		{
			uint32 ea = REG_A[reg];
			h1 = m68ki_read_32(ea+0);
			h2 = m68ki_read_32(ea+4);
			return  (uint64)(h1) << 32 | (uint64)(h2);
		}
		case 3:		// (An)+
		{
			uint32 ea = REG_A[reg];
			REG_A[reg] += 8;
			h1 = m68ki_read_32(ea+0);
			h2 = m68ki_read_32(ea+4);
			return  (uint64)(h1) << 32 | (uint64)(h2);
		}
		case 5:		// (d16, An)
		{
			uint32 ea = EA_AY_DI_32();
			h1 = m68ki_read_32(ea+0);
			h2 = m68ki_read_32(ea+4);
			return  (uint64)(h1) << 32 | (uint64)(h2);
		}
		case 7:
		{
			switch (reg)
			{
				case 4:		// #<data>
				{
					h1 = OPER_I_32();
					h2 = OPER_I_32();
					return  (uint64)(h1) << 32 | (uint64)(h2);
				}
				case 2:		// (d16, PC)
				{
					uint32 ea = EA_PCDI_32();
					h1 = m68ki_read_32(ea+0);
					h2 = m68ki_read_32(ea+4);
					return  (uint64)(h1) << 32 | (uint64)(h2);
				}
				default:	fatalerror("MC68040: READ_EA_64: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
			}
			break;
		}
		default:	fatalerror("MC68040: READ_EA_64: unhandled mode %d, reg %d at %08X\n", mode, reg, REG_PC);
	}

	return 0;
}

static void WRITE_EA_64(int ea, uint64 data)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	switch (mode)
	{
		case 2:		// (An)
		{
			uint32 ea = REG_A[reg];
			m68ki_write_32(ea, (uint32)(data >> 32));
			m68ki_write_32(ea, (uint32)(data));
			break;
		}
		case 4:		// -(An)
		{
			uint32 ea;
			REG_A[reg] -= 8;
			ea = REG_A[reg];
			m68ki_write_32(ea+0, (uint32)(data >> 32));
			m68ki_write_32(ea+4, (uint32)(data));
			break;
		}
		case 5:		// (d16, An)
		{
			uint32 ea = EA_AY_DI_32();
			m68ki_write_32(ea+0, (uint32)(data >> 32));
			m68ki_write_32(ea+4, (uint32)(data));
			break;
		}
		default:	fatalerror("MC68040: WRITE_EA_64: unhandled mode %d, reg %d, data %08X%08X at %08X\n", mode, reg, (uint32)(data >> 32), (uint32)(data), REG_PC);
	}
}

static fp_reg READ_EA_FPE(int ea)
{
	fp_reg r;
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	// TODO: convert to extended floating-point!

	switch (mode)
	{
		case 3:		// (An)+
		{
			uint32 d1,d2;
			uint32 ea = REG_A[reg];
			REG_A[reg] += 12;
			d1 = m68ki_read_32(ea+0);
			d2 = m68ki_read_32(ea+4);
			// d3 = m68ki_read_32(ea+8);
			r.i = (uint64)(d1) << 32 | (uint64)(d2);
			break;
		}
		default:	fatalerror("MC68040: READ_EA_FPE: unhandled mode %d, reg %d, at %08X\n", mode, reg, REG_PC);
	}

	return r;
}

static void WRITE_EA_FPE(int ea, fp_reg fpr)
{
	int mode = (ea >> 3) & 0x7;
	int reg = (ea & 0x7);

	// TODO: convert to extended floating-point!

	switch (mode)
	{
		case 4:		// -(An)
		{
			uint32 ea;
			REG_A[reg] -= 12;
			ea = REG_A[reg];
			m68ki_write_32(ea+0, (uint32)(fpr.i >> 32));
			m68ki_write_32(ea+4, (uint32)(fpr.i));
			m68ki_write_32(ea+8, 0);
			break;
		}
		default:	fatalerror("MC68040: WRITE_EA_FPE: unhandled mode %d, reg %d, data %f at %08X\n", mode, reg, fpr.f, REG_PC);
	}
}


static void fpgen_rm_reg(uint16 w2)
{
	int ea = REG_IR & 0x3f;
	int rm = (w2 >> 14) & 0x1;
	int src = (w2 >> 10) & 0x7;
	int dst = (w2 >>  7) & 0x7;
	int opmode = w2 & 0x7f;
	double source;

	if (rm)
	{
		switch (src)
		{
			case 0:		// Long-Word Integer
			{
				sint32 d = READ_EA_32(ea);
				source = (double)(d);
				break;
			}
			case 1:		// Single-precision Real
			{
				uint32 d = READ_EA_32(ea);
				source = (double)(*(float*)&d);
				break;
			}
			case 2:		// Extended-precision Real
			{
				fatalerror("fpgen_rm_reg: extended-precision real load unimplemented at %08X\n", REG_PC-4);
				break;
			}
			case 3:		// Packed-decimal Real
			{
				fatalerror("fpgen_rm_reg: packed-decimal real load unimplemented at %08X\n", REG_PC-4);
				break;
			}
			case 4:		// Word Integer
			{
				sint16 d = READ_EA_16(ea);
				source = (double)(d);
				break;
			}
			case 5:		// Double-precision Real
			{
				uint64 d = READ_EA_64(ea);
				source = *(double*)&d;
				break;
			}
			case 6:		// Byte Integer
			{
				sint8 d = READ_EA_8(ea);
				source = (double)(d);
				break;
			}
			default:	fatalerror("fmove_rm_reg: invalid source specifier at %08X\n", REG_PC-4);
		}
	}
	else
	{
		source = REG_FP[src].f;
	}

	switch (opmode)
	{
		case 0x00:		// FMOVE
		{
			REG_FP[dst].f = source;
			USE_CYCLES(4);
			break;
		}
		case 0x04:		// FSQRT
		{
			REG_FP[dst].f = sqrt(source);
			SET_CONDITION_CODES(REG_FP[dst]);
			USE_CYCLES(109);
			break;
		}
		case 0x18:		// FABS
		{
			REG_FP[dst].f = fabs(source);
			SET_CONDITION_CODES(REG_FP[dst]);
			USE_CYCLES(3);
			break;
		}
		case 0x1a:		// FNEG
		{
			REG_FP[dst].f = -source;
			SET_CONDITION_CODES(REG_FP[dst]);
			USE_CYCLES(3);
			break;
		}
		case 0x20:		// FDIV
		{
			REG_FP[dst].f /= source;
			USE_CYCLES(43);
			break;
		}
		case 0x22:		// FADD
		{
			REG_FP[dst].f += source;
			SET_CONDITION_CODES(REG_FP[dst]);
			USE_CYCLES(9);
			break;
		}
		case 0x23:		// FMUL
		{
			REG_FP[dst].f *= source;
			SET_CONDITION_CODES(REG_FP[dst]);
			USE_CYCLES(11);
			break;
		}
		case 0x28:		// FSUB
		{
			REG_FP[dst].f -= source;
			SET_CONDITION_CODES(REG_FP[dst]);
			USE_CYCLES(9);
			break;
		}
		case 0x38:		// FCMP
		{
			fp_reg res;
			res.f = REG_FP[dst].f - source;
			SET_CONDITION_CODES(res);
			USE_CYCLES(7);
			break;
		}
		case 0x3a:		// FTST
		{
			fp_reg res;
			res.f = source;
			SET_CONDITION_CODES(res);
			USE_CYCLES(7);
			break;
		}

		default:	fatalerror("fpgen_rm_reg: unimplemented opmode %02X at %08X\n", opmode, REG_PC-4);
	}
}

static void fmove_reg_mem(uint16 w2)
{
	int ea = REG_IR & 0x3f;
	int src = (w2 >>  7) & 0x7;
	int dst = (w2 >> 10) & 0x7;
	//int kfactor = w2 & 0x7f;

	switch (dst)
	{
		case 0:		// Long-Word Integer
		{
			sint32 d = (sint32)(REG_FP[src].f);
			WRITE_EA_32(ea, d);
			break;
		}
		case 1:		// Single-precision Real
		{
			float f = (float)(REG_FP[src].f);
			uint32 d = *(uint32 *)&f;
			WRITE_EA_32(ea, d);
			break;
		}
		case 2:		// Extended-precision Real
		{
			fatalerror("fmove_reg_mem: extended-precision real store unimplemented at %08X\n", REG_PC-4);
			break;
		}
		case 3:		// Packed-decimal Real with Static K-factor
		{
			fatalerror("fmove_reg_mem: packed-decimal real store unimplemented at %08X\n", REG_PC-4);
			break;
		}
		case 4:		// Word Integer
		{
			fatalerror("fmove_reg_mem: word integer store unimplemented at %08X\n", REG_PC-4);
			break;
		}
		case 5:		// Double-precision Real
		{
			uint64 d = REG_FP[src].i;
			WRITE_EA_64(ea, d);
			break;
		}
		case 6:		// Byte Integer
		{
			fatalerror("fmove_reg_mem: byte integer store unimplemented at %08X\n", REG_PC-4);
			break;
		}
		case 7:		// Packed-decimal Real with Dynamic K-factor
		{
			fatalerror("fmove_reg_mem: packed-decimal real store unimplemented at %08X\n", REG_PC-4);
			break;
		}
	}

	USE_CYCLES(12);
}

static void fmove_fpcr(uint16 w2)
{
	int ea = REG_IR & 0x3f;
	int dir = (w2 >> 13) & 0x1;
	int reg = (w2 >> 10) & 0x7;

	if (dir)	// From system control reg to <ea>
	{
		switch (reg)
		{
			case 1:		WRITE_EA_32(ea, REG_FPIAR); break;
			case 2:		WRITE_EA_32(ea, REG_FPSR); break;
			case 4:		WRITE_EA_32(ea, REG_FPCR); break;
			default:	fatalerror("fmove_fpcr: unknown reg %d, dir %d\n", reg, dir);
		}
	}
	else		// From <ea> to system control reg
	{
		switch (reg)
		{
			case 1:		REG_FPIAR = READ_EA_32(ea); break;
			case 2:		REG_FPSR = READ_EA_32(ea); break;
			case 4:		REG_FPCR = READ_EA_32(ea); break;
			default:	fatalerror("fmove_fpcr: unknown reg %d, dir %d\n", reg, dir);
		}
	}

	USE_CYCLES(10);
}

static void fmovem(uint16 w2)
{
	int i;
	int ea = REG_IR & 0x3f;
	int dir = (w2 >> 13) & 0x1;
	int mode = (w2 >> 11) & 0x3;
	int reglist = w2 & 0xff;

	if (dir)	// From FP regs to mem
	{
		switch (mode)
		{
			case 0:		// Static register list, predecrement addressing mode
			{
				for (i=0; i < 8; i++)
				{
					if (reglist & (1 << i))
					{
						WRITE_EA_FPE(ea, REG_FP[i]);
						USE_CYCLES(2);
					}
				}
				break;
			}

			default:	fatalerror("040fpu0: FMOVEM: mode %d unimplemented at %08X\n", mode, REG_PC-4);
		}
	}
	else		// From mem to FP regs
	{
		switch (mode)
		{
			case 2:		// Static register list, postincrement addressing mode
			{
				for (i=0; i < 8; i++)
				{
					if (reglist & (1 << i))
					{
						REG_FP[7-i] = READ_EA_FPE(ea);
						USE_CYCLES(2);
					}
				}
				break;
			}

			default:	fatalerror("040fpu0: FMOVEM: mode %d unimplemented at %08X\n", mode, REG_PC-4);
		}
	}
}

static void fbcc16(void)
{
	sint32 offset;
	int condition = REG_IR & 0x3f;

	offset = (sint16)(OPER_I_16());

	// TODO: condition and jump!!!
	if (TEST_CONDITION(condition))
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset-2);
	}

	USE_CYCLES(7);
	}

static void fbcc32(void)
{
	sint32 offset;
	int condition = REG_IR & 0x3f;

	offset = OPER_I_32();

	// TODO: condition and jump!!!
	if (TEST_CONDITION(condition))
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_32(offset-4);
	}

	USE_CYCLES(7);
}


void m68040_fpu_op0(void)
{
	switch ((REG_IR >> 6) & 0x3)
	{
		case 0:
		{
			uint16 w2 = OPER_I_16();
			switch ((w2 >> 13) & 0x7)
			{
				case 0x0:	// FPU ALU FP, FP
				case 0x2:	// FPU ALU ea, FP
				{
					fpgen_rm_reg(w2);
					break;
				}

				case 0x3:	// FMOVE FP, ea
				{
					fmove_reg_mem(w2);
					break;
				}

				case 0x4:	// FMOVE ea, FPCR
				case 0x5:	// FMOVE FPCR, ea
				{
					fmove_fpcr(w2);
					break;
				}

				case 0x6:	// FMOVEM ea, list
				case 0x7:	// FMOVEM list, ea
				{
					fmovem(w2);
					break;
				}

				default:	fatalerror("m68040_fpu_op0: unimplemented subop %d at %08X\n", (w2 >> 13) & 0x7, REG_PC-4);
			}
			break;
		}

		case 2:		// FBcc disp16
		{
			fbcc16();
			break;
		}
		case 3:		// FBcc disp32
		{
			fbcc32();
			break;
		}

		default:	fatalerror("m68040_fpu_op0: unimplemented main op %d\n", (REG_IR >> 6)	& 0x3);
	}
}

void m68040_fpu_op1(void)
{
	int ea = REG_IR & 0x3f;

	switch ((REG_IR >> 6) & 0x3)
	{
		case 0:		// FSAVE <ea>
		{
			WRITE_EA_32(ea, 0x00000000);
			// TODO: correct state frame
			break;
		}

		case 1:		// FRESTORE <ea>
		{
			READ_EA_32(ea);
			// TODO: correct state frame
			break;
		}

		default:	fatalerror("m68040_fpu_op1: unimplemented op %d at %08X\n", (REG_IR >> 6) & 0x3, REG_PC-2);
	}
}



