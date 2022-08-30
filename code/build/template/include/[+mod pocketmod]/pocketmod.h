/* See end of file for license */

#ifndef POCKETMOD_H_INCLUDED
#define POCKETMOD_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pocketmod_context pocketmod_context;
int pocketmod_init(pocketmod_context *c, const void *data, int size, int rate);
int pocketmod_render(pocketmod_context *c, void *buffer, int size);
int pocketmod_loop_count(pocketmod_context *c);

#ifndef POCKETMOD_MAX_CHANNELS
#define POCKETMOD_MAX_CHANNELS 32
#endif

#ifndef POCKETMOD_MAX_SAMPLES
#define POCKETMOD_MAX_SAMPLES 31
#endif

typedef struct {
    signed char *data;          /* Sample data buffer                      */
    unsigned int length;        /* Data length (in bytes)                  */
} _pocketmod_sample;

typedef struct {
    unsigned char dirty;        /* Pitch/volume dirty flags                */
    unsigned char sample;       /* Sample number (0..31)                   */
    unsigned char volume;       /* Base volume without tremolo (0..64)     */
    unsigned char balance;      /* Stereo balance (0..255)                 */
    unsigned short period;      /* Note period (113..856)                  */
    unsigned short delayed;     /* Delayed note period (113..856)          */
    unsigned short target;      /* Target period (for tone portamento)     */
    unsigned char finetune;     /* Note finetune (0..15)                   */
    unsigned char loop_count;   /* E6x loop counter                        */
    unsigned char loop_line;    /* E6x target line                         */
    unsigned char lfo_step;     /* Vibrato/tremolo LFO step counter        */
    unsigned char lfo_type[2];  /* LFO type for vibrato/tremolo            */
    unsigned char effect;       /* Current effect (0x0..0xf or 0xe0..0xef) */
    unsigned char param;        /* Raw effect parameter value              */
    unsigned char param3;       /* Parameter memory for 3xx                */
    unsigned char param4;       /* Parameter memory for 4xy                */
    unsigned char param7;       /* Parameter memory for 7xy                */
    unsigned char param9;       /* Parameter memory for 9xx                */
    unsigned char paramE1;      /* Parameter memory for E1x                */
    unsigned char paramE2;      /* Parameter memory for E2x                */
    unsigned char paramEA;      /* Parameter memory for EAx                */
    unsigned char paramEB;      /* Parameter memory for EBx                */
    unsigned char real_volume;  /* Volume (with tremolo adjustment)        */
    float position;             /* Position in sample data buffer          */
    float increment;            /* Position increment per output sample    */
} _pocketmod_chan;

struct pocketmod_context
{
    /* Read-only song data */
    _pocketmod_sample samples[POCKETMOD_MAX_SAMPLES];
    unsigned char *source;      /* Pointer to source MOD data              */
    unsigned char *order;       /* Pattern order table                     */
    unsigned char *patterns;    /* Start of pattern data                   */
    unsigned char length;       /* Patterns in the order (1..128)          */
    unsigned char reset;        /* Pattern to loop back to (0..127)        */
    unsigned char num_patterns; /* Patterns in the file (1..128)           */
    unsigned char num_samples;  /* Sample count (15 or 31)                 */
    unsigned char num_channels; /* Channel count (1..32)                   */

    /* Timing variables */
    int samples_per_second;     /* Sample rate (set by user)               */
    int ticks_per_line;         /* A.K.A. song speed (initially 6)         */
    float samples_per_tick;     /* Depends on sample rate and BPM          */

    /* Loop detection state */
    unsigned char visited[16];  /* Bit mask of previously visited patterns */
    int loop_count;             /* How many times the song has looped      */

    /* Render state */
    _pocketmod_chan channels[POCKETMOD_MAX_CHANNELS];
    unsigned char pattern_delay;/* EEx pattern delay counter               */
    unsigned int lfo_rng;       /* RNG used for the random LFO waveform    */

    /* Position in song (from least to most granular) */
    signed char pattern;        /* Current pattern in order                */
    signed char line;           /* Current line in pattern                 */
    short tick;                 /* Current tick in line                    */
    float sample;               /* Current sample in tick                  */
};

#ifdef POCKETMOD_IMPLEMENTATION

/* Memorize a parameter unless the new value is zero */
#define POCKETMOD_MEM(dst, src) do { \
        (dst) = (src) ? (src) : (dst); \
    } while (0)

/* Same thing, but memorize each nibble separately */
#define POCKETMOD_MEM2(dst, src) do { \
        (dst) = (((src) & 0x0f) ? ((src) & 0x0f) : ((dst) & 0x0f)) \
              | (((src) & 0xf0) ? ((src) & 0xf0) : ((dst) & 0xf0)); \
    } while (0)

/* Shortcut to sample metadata (sample must be nonzero) */
#define POCKETMOD_SAMPLE(c, sample) ((c)->source + 12 + 30 * (sample))

/* Channel dirty flags */
#define POCKETMOD_PITCH  0x01
#define POCKETMOD_VOLUME 0x02

/* The size of one sample in bytes */
#define POCKETMOD_SAMPLE_SIZE sizeof(float[2])

/* Finetune adjustment table. Three octaves for each finetune setting. */
static const signed char _pocketmod_finetune[16][36] = {
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    { -6, -6, -5, -5, -4, -3, -3, -3, -3, -3, -3, -3, -3, -3, -2, -3, -2, -2, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0},
    {-12,-12,-10,-11, -8, -8, -7, -7, -6, -6, -6, -6, -6, -6, -5, -5, -4, -4, -4, -3, -3, -3, -3, -2, -3, -3, -2, -3, -3, -2, -2, -2, -2, -2, -2, -1},
    {-18,-17,-16,-16,-13,-12,-12,-11,-10,-10,-10, -9, -9, -9, -8, -8, -7, -6, -6, -5, -5, -5, -5, -4, -5, -4, -3, -4, -4, -3, -3, -3, -3, -2, -2, -2},
    {-24,-23,-21,-21,-18,-17,-16,-15,-14,-13,-13,-12,-12,-12,-11,-10, -9, -8, -8, -7, -7, -7, -7, -6, -6, -6, -5, -5, -5, -4, -4, -4, -4, -3, -3, -3},
    {-30,-29,-26,-26,-23,-21,-20,-19,-18,-17,-17,-16,-15,-14,-13,-13,-11,-11,-10, -9, -9, -9, -8, -7, -8, -7, -6, -6, -6, -5, -5, -5, -5, -4, -4, -4},
    {-36,-34,-32,-31,-27,-26,-24,-23,-22,-21,-20,-19,-18,-17,-16,-15,-14,-13,-12,-11,-11,-10,-10, -9, -9, -9, -7, -8, -7, -6, -6, -6, -6, -5, -5, -4},
    {-42,-40,-37,-36,-32,-30,-29,-27,-25,-24,-23,-22,-21,-20,-18,-18,-16,-15,-14,-13,-13,-12,-12,-10,-10,-10, -9, -9, -9, -8, -7, -7, -7, -6, -6, -5},
    { 51, 48, 46, 42, 42, 38, 36, 34, 32, 30, 24, 27, 25, 24, 23, 21, 21, 19, 18, 17, 16, 15, 14, 14, 12, 12, 12, 10, 10, 10,  9,  8,  8,  8,  7,  7},
    { 44, 42, 40, 37, 37, 35, 32, 31, 29, 27, 25, 24, 22, 21, 20, 19, 18, 17, 16, 15, 15, 14, 13, 12, 11, 10, 10,  9,  9,  9,  8,  7,  7,  7,  6,  6},
    { 38, 36, 34, 32, 31, 30, 28, 27, 25, 24, 22, 21, 19, 18, 17, 16, 16, 15, 14, 13, 13, 12, 11, 11,  9,  9,  9,  8,  7,  7,  7,  6,  6,  6,  5,  5},
    { 31, 30, 29, 26, 26, 25, 24, 22, 21, 20, 18, 17, 16, 15, 14, 13, 13, 12, 12, 11, 11, 10,  9,  9,  8,  7,  8,  7,  6,  6,  6,  5,  5,  5,  5,  5},
    { 25, 24, 23, 21, 21, 20, 19, 18, 17, 16, 14, 14, 13, 12, 11, 10, 11, 10, 10,  9,  9,  8,  7,  7,  6,  6,  6,  5,  5,  5,  5,  4,  4,  4,  3,  4},
    { 19, 18, 17, 16, 16, 15, 15, 14, 13, 12, 11, 10,  9,  9,  9,  8,  8, 18,  7,  7,  7,  6,  5,  6,  5,  4,  5,  4,  4,  4,  4,  3,  3,  3,  3,  3},
    { 12, 12, 12, 10, 11, 11, 10, 10,  9,  8,  7,  7,  6,  6,  6,  5,  6,  5,  5,  5,  5,  4,  4,  4,  3,  3,  3,  3,  2,  3,  3,  2,  2,  2,  2,  2},
    {  6,  6,  6,  5,  6,  6,  6,  5,  5,  5,  4,  4,  3,  3,  3,  3,  3,  3,  3,  3,  3,  2,  2,  2,  2,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1}
};

/* Min/max helper functions */
static int _pocketmod_min(int x, int y) { return x < y ? x : y; }
static int _pocketmod_max(int x, int y) { return x > y ? x : y; }

/* Clamp a volume value to the 0..64 range */
static int _pocketmod_clamp_volume(int x)
{
    x = _pocketmod_max(x, 0x00);
    x = _pocketmod_min(x, 0x40);
    return x;
}

/* Zero out a block of memory */
static void _pocketmod_zero(void *data, int size)
{
    char *byte = (char *)data, *end = byte + size;
    while (byte != end) { *byte++ = 0; }
}

/* Convert a period (at finetune = 0) to a note index in 0..35 */
static int _pocketmod_period_to_note(int period)
{
    switch (period) {
        case 856: return  0; case 808: return  1; case 762: return  2;
        case 720: return  3; case 678: return  4; case 640: return  5;
        case 604: return  6; case 570: return  7; case 538: return  8;
        case 508: return  9; case 480: return 10; case 453: return 11;
        case 428: return 12; case 404: return 13; case 381: return 14;
        case 360: return 15; case 339: return 16; case 320: return 17;
        case 302: return 18; case 285: return 19; case 269: return 20;
        case 254: return 21; case 240: return 22; case 226: return 23;
        case 214: return 24; case 202: return 25; case 190: return 26;
        case 180: return 27; case 170: return 28; case 160: return 29;
        case 151: return 30; case 143: return 31; case 135: return 32;
        case 127: return 33; case 120: return 34; case 113: return 35;
        default: return 0;
    }
}

/* Table-based sine wave oscillator */
static int _pocketmod_sin(int step)
{
    /* round(sin(x * pi / 32) * 255) for x in 0..15 */
    static const unsigned char sin[16] = {
        0x00, 0x19, 0x32, 0x4a, 0x62, 0x78, 0x8e, 0xa2,
        0xb4, 0xc5, 0xd4, 0xe0, 0xec, 0xf4, 0xfa, 0xfe
    };
    int x = sin[step & 0x0f];
    x = (step & 0x1f) < 0x10 ? x : 0xff - x;
    return step < 0x20 ? x : -x;
}

/* Oscillators for vibrato/tremolo effects */
static int _pocketmod_lfo(pocketmod_context *c, _pocketmod_chan *ch, int step)
{
    switch (ch->lfo_type[ch->effect == 7] & 3) {
        case 0: return _pocketmod_sin(step & 0x3f);         /* Sine   */
        case 1: return 0xff - ((step & 0x3f) << 3);         /* Saw    */
        case 2: return (step & 0x3f) < 0x20 ? 0xff : -0xff; /* Square */
        case 3: return (c->lfo_rng & 0x1ff) - 0xff;         /* Random */
        default: return 0; /* Hush little compiler */
    }
}

static void _pocketmod_update_pitch(pocketmod_context *c, _pocketmod_chan *ch)
{
    /* Don't do anything if the period is zero */
    ch->increment = 0.0f;
    if (ch->period) {
        float period = ch->period;

        /* Apply vibrato (if active) */
        if (ch->effect == 0x4 || ch->effect == 0x6) {
            int step = (ch->param4 >> 4) * ch->lfo_step;
            int rate = ch->param4 & 0x0f;
            period += _pocketmod_lfo(c, ch, step) * rate / 128.0f;

        /* Apply arpeggio (if active) */
        } else if (ch->effect == 0x0 && ch->param) {
            static const float arpeggio[16] = { /* 2^(X/12) for X in 0..15 */
                1.000000f, 1.059463f, 1.122462f, 1.189207f,
                1.259921f, 1.334840f, 1.414214f, 1.498307f,
                1.587401f, 1.681793f, 1.781797f, 1.887749f,
                2.000000f, 2.118926f, 2.244924f, 2.378414f
            };
            int step = (ch->param >> ((2 - c->tick % 3) << 2)) & 0x0f;
            period /= arpeggio[step];
        }

        /* Calculate sample buffer position increment */
        ch->increment = 3546894.6f / (period * c->samples_per_second);
    }

    /* Clear the pitch dirty flag */
    ch->dirty &= ~POCKETMOD_PITCH;
}

static void _pocketmod_update_volume(pocketmod_context *c, _pocketmod_chan *ch)
{
    int volume = ch->volume;
    if (ch->effect == 0x7) {
        int step = ch->lfo_step * (ch->param7 >> 4);
        volume += _pocketmod_lfo(c, ch, step) * (ch->param7 & 0x0f) >> 6;
    }
    ch->real_volume = _pocketmod_clamp_volume(volume);
    ch->dirty &= ~POCKETMOD_VOLUME;
}

static void _pocketmod_pitch_slide(_pocketmod_chan *ch, int amount)
{
    int max = 856 + _pocketmod_finetune[ch->finetune][ 0];
    int min = 113 + _pocketmod_finetune[ch->finetune][35];
    ch->period += amount;
    ch->period = _pocketmod_max(ch->period, min);
    ch->period = _pocketmod_min(ch->period, max);
    ch->dirty |= POCKETMOD_PITCH;
}

static void _pocketmod_volume_slide(_pocketmod_chan *ch, int param)
{
    /* Undocumented quirk: If both x and y are nonzero, then the value of x */
    /* takes precedence. (Yes, there are songs that rely on this behavior.) */
    int change = (param & 0xf0) ? (param >> 4) : -(param & 0x0f);
    ch->volume = _pocketmod_clamp_volume(ch->volume + change);
    ch->dirty |= POCKETMOD_VOLUME;
}

static void _pocketmod_next_line(pocketmod_context *c)
{
    unsigned char (*data)[4];
    int i, pos, pattern_break = -1;

    /* When entering a new pattern order index, mark it as "visited" */
    if (c->line == 0) {
        c->visited[c->pattern >> 3] |= 1 << (c->pattern & 7);
    }

    /* Move to the next pattern if this was the last line */
    if (++c->line == 64) {
        if (++c->pattern == c->length) {
            c->pattern = c->reset;
        }
        c->line = 0;
    }

    /* Find the pattern data for the current line */
    pos = (c->order[c->pattern] * 64 + c->line) * c->num_channels * 4;
    data = (unsigned char(*)[4]) (c->patterns + pos);
    for (i = 0; i < c->num_channels; i++) {

        /* Decode columns */
        int sample = (data[i][0] & 0xf0) | (data[i][2] >> 4);
        int period = ((data[i][0] & 0x0f) << 8) | data[i][1];
        int effect = ((data[i][2] & 0x0f) << 8) | data[i][3];

        /* Memorize effect parameter values */
        _pocketmod_chan *ch = &c->channels[i];
        ch->effect = (effect >> 8) != 0xe ? (effect >> 8) : (effect >> 4);
        ch->param = (effect >> 8) != 0xe ? (effect & 0xff) : (effect & 0x0f);

        /* Set sample */
        if (sample) {
            if (sample <= POCKETMOD_MAX_SAMPLES) {
                unsigned char *sample_data = POCKETMOD_SAMPLE(c, sample);
                ch->sample = sample;
                ch->finetune = sample_data[2] & 0x0f;
                ch->volume = _pocketmod_min(sample_data[3], 0x40);
                if (ch->effect != 0xED) {
                    ch->dirty |= POCKETMOD_VOLUME;
                }
            } else {
                ch->sample = 0;
            }
        }

        /* Set note */
        if (period) {
            int note = _pocketmod_period_to_note(period);
            period += _pocketmod_finetune[ch->finetune][note];
            if (ch->effect != 0x3) {
                if (ch->effect != 0xED) {
                    ch->period = period;
                    ch->dirty |= POCKETMOD_PITCH;
                    ch->position = 0.0f;
                    ch->lfo_step = 0;
                } else {
                    ch->delayed = period;
                }
            }
        }

        /* Handle pattern effects */
        switch (ch->effect) {

            /* Memorize parameters */
            case 0x3: POCKETMOD_MEM(ch->param3, ch->param); /* Fall through */
            case 0x5: POCKETMOD_MEM(ch->target, period); break;
            case 0x4: POCKETMOD_MEM2(ch->param4, ch->param); break;
            case 0x7: POCKETMOD_MEM2(ch->param7, ch->param); break;
            case 0xE1: POCKETMOD_MEM(ch->paramE1, ch->param); break;
            case 0xE2: POCKETMOD_MEM(ch->paramE2, ch->param); break;
            case 0xEA: POCKETMOD_MEM(ch->paramEA, ch->param); break;
            case 0xEB: POCKETMOD_MEM(ch->paramEB, ch->param); break;

            /* 8xx: Set stereo balance (nonstandard) */
            case 0x8: {
                ch->balance = ch->param;
            } break;

            /* 9xx: Set sample offset */
            case 0x9: {
                if (period != 0 || sample != 0) {
                    ch->param9 = ch->param ? ch->param : ch->param9;
                    ch->position = (float)(ch->param9 << 8);
                }
            } break;

            /* Bxx: Jump to pattern */
            case 0xB: {
                c->pattern = ch->param < c->length ? ch->param : 0;
                c->line = -1;
            } break;

            /* Cxx: Set volume */
            case 0xC: {
                ch->volume = _pocketmod_clamp_volume(ch->param);
                ch->dirty |= POCKETMOD_VOLUME;
            } break;

            /* Dxy: Pattern break */
            case 0xD: {
                pattern_break = (ch->param >> 4) * 10 + (ch->param & 15);
            } break;

            /* E4x: Set vibrato waveform */
            case 0xE4: {
                ch->lfo_type[0] = ch->param;
            } break;

            /* E5x: Set sample finetune */
            case 0xE5: {
                ch->finetune = ch->param;
                ch->dirty |= POCKETMOD_PITCH;
            } break;

            /* E6x: Pattern loop */
            case 0xE6: {
                if (ch->param) {
                    if (!ch->loop_count) {
                        ch->loop_count = ch->param;
                        c->line = ch->loop_line;
                    } else if (--ch->loop_count) {
                        c->line = ch->loop_line;
                    }
                } else {
                    ch->loop_line = c->line - 1;
                }
            } break;

            /* E7x: Set tremolo waveform */
            case 0xE7: {
                ch->lfo_type[1] = ch->param;
            } break;

            /* E8x: Set stereo balance (nonstandard) */
            case 0xE8: {
                ch->balance = ch->param << 4;
            } break;

            /* EEx: Pattern delay */
            case 0xEE: {
                c->pattern_delay = ch->param;
            } break;

            /* Fxx: Set speed */
            case 0xF: {
                if (ch->param != 0) {
                    if (ch->param < 0x20) {
                        c->ticks_per_line = ch->param;
                    } else {
                        float rate = (float)c->samples_per_second;
                        c->samples_per_tick = rate / (0.4f * ch->param);
                    }
                }
            } break;

            default: break;
        }
    }

    /* Pattern breaks are handled here, so that only one jump happens even  */
    /* when multiple Dxy commands appear on the same line. (You guessed it: */
    /* There are songs that rely on this behavior!)                         */
    if (pattern_break != -1) {
        c->line = (pattern_break < 64 ? pattern_break : 0) - 1;
        if (++c->pattern == c->length) {
            c->pattern = c->reset;
        }
    }
}

static void _pocketmod_next_tick(pocketmod_context *c)
{
    int i;

    /* Move to the next line if this was the last tick */
    if (++c->tick == c->ticks_per_line) {
        if (c->pattern_delay > 0) {
            c->pattern_delay--;
        } else {
            _pocketmod_next_line(c);
        }
        c->tick = 0;
    }

    /* Make per-tick adjustments for all channels */
    for (i = 0; i < c->num_channels; i++) {
        _pocketmod_chan *ch = &c->channels[i];
        int param = ch->param;

        /* Advance the LFO random number generator */
        c->lfo_rng = 0x0019660d * c->lfo_rng + 0x3c6ef35f;

        /* Handle effects that may happen on any tick of a line */
        switch (ch->effect) {

            /* 0xy: Arpeggio */
            case 0x0: {
                ch->dirty |= POCKETMOD_PITCH;
            } break;

            /* E9x: Retrigger note every x ticks */
            case 0xE9: {
                if (!(param && c->tick % param)) {
                    ch->position = 0.0f;
                    ch->lfo_step = 0;
                }
            } break;

            /* ECx: Cut note after x ticks */
            case 0xEC: {
                if (c->tick == param) {
                    ch->volume = 0;
                    ch->dirty |= POCKETMOD_VOLUME;
                }
            } break;

            /* EDx: Delay note for x ticks */
            case 0xED: {
                if (c->tick == param && ch->sample) {
                    ch->dirty |= POCKETMOD_VOLUME | POCKETMOD_PITCH;
                    ch->period = ch->delayed;
                    ch->position = 0.0f;
                    ch->lfo_step = 0;
                }
            } break;

            default: break;
        }

        /* Handle effects that only happen on the first tick of a line */
        if (c->tick == 0) {
            switch (ch->effect) {
                case 0xE1: _pocketmod_pitch_slide(ch, -ch->paramE1); break;
                case 0xE2: _pocketmod_pitch_slide(ch, +ch->paramE2); break;
                case 0xEA: _pocketmod_volume_slide(ch, ch->paramEA << 4); break;
                case 0xEB: _pocketmod_volume_slide(ch, ch->paramEB & 15); break;
                default: break;
            }

        /* Handle effects that are not applied on the first tick of a line */
        } else {
            switch (ch->effect) {

                /* 1xx: Portamento up */
                case 0x1: {
                    _pocketmod_pitch_slide(ch, -param);
                } break;

                /* 2xx: Portamento down */
                case 0x2: {
                    _pocketmod_pitch_slide(ch, +param);
                } break;

                /* 5xy: Volume slide + tone portamento */
                case 0x5: {
                    _pocketmod_volume_slide(ch, param);
                } /* Fall through */

                /* 3xx: Tone portamento */
                case 0x3: {
                    int rate = ch->param3;
                    int order = ch->period < ch->target;
                    int closer = ch->period + (order ? rate : -rate);
                    int new_order = closer < ch->target;
                    ch->period = new_order == order ? closer : ch->target;
                    ch->dirty |= POCKETMOD_PITCH;
                } break;

                /* 6xy: Volume slide + vibrato */
                case 0x6: {
                    _pocketmod_volume_slide(ch, param);
                } /* Fall through */

                /* 4xy: Vibrato */
                case 0x4: {
                    ch->lfo_step++;
                    ch->dirty |= POCKETMOD_PITCH;
                } break;

                /* 7xy: Tremolo */
                case 0x7: {
                    ch->lfo_step++;
                    ch->dirty |= POCKETMOD_VOLUME;
                } break;

                /* Axy: Volume slide */
                case 0xA: {
                    _pocketmod_volume_slide(ch, param);
                } break;

                default: break;
            }
        }

        /* Update channel volume/pitch if either is out of date */
        if (ch->dirty & POCKETMOD_VOLUME) { _pocketmod_update_volume(c, ch); }
        if (ch->dirty & POCKETMOD_PITCH) { _pocketmod_update_pitch(c, ch); }
    }
}

static void _pocketmod_render_channel(pocketmod_context *c,
                                      _pocketmod_chan *chan,
                                      float *output,
                                      int samples_to_write)
{
    /* Gather some loop data */
    _pocketmod_sample *sample = &c->samples[chan->sample - 1];
    unsigned char *data = POCKETMOD_SAMPLE(c, chan->sample);
    const int loop_start = ((data[4] << 8) | data[5]) << 1;
    const int loop_length = ((data[6] << 8) | data[7]) << 1;
    const int loop_end = loop_length > 2 ? loop_start + loop_length : 0xffffff;
    const float sample_end = (float)(1 + _pocketmod_min(loop_end, sample->length));

    /* Calculate left/right levels */
    const float volume = chan->real_volume / (float) (128 * 64 * 4);
    const float level_l = volume * (1.0f - chan->balance / 255.0f);
    const float level_r = volume * (0.0f + chan->balance / 255.0f);

    /* Write samples */
    int i, num;
    do {

        /* Calculate how many samples we can write in one go */
        num = (int)((sample_end - chan->position) / chan->increment);
        num = _pocketmod_min(num, samples_to_write);

        /* Resample and write 'num' samples */
        for (i = 0; i < num; i++) {
            int x0 = (int)chan->position;
#ifdef POCKETMOD_NO_INTERPOLATION
            float s = sample->data[x0];
#else
            int x1 = x0 + 1 - loop_length * (x0 + 1 >= loop_end);
            float t = chan->position - x0;
            float s = (1.0f - t) * sample->data[x0] + t * sample->data[x1];
#endif
            chan->position += chan->increment;
            *output++ += level_l * s;
            *output++ += level_r * s;
        }

        /* Rewind the sample when reaching the loop point */
        if (chan->position >= loop_end) {
            chan->position -= loop_length;

        /* Cut the sample if the end is reached */
        } else if (chan->position >= sample->length) {
            chan->position = -1.0f;
            break;
        }

        samples_to_write -= num;
    } while (num > 0);
}

static int _pocketmod_ident(pocketmod_context *c, unsigned char *data, int size)
{
    int i, j;

    /* 31-instrument files are at least 1084 bytes long */
    if (size >= 1084) {

        /* The format tag is located at offset 1080 */
        unsigned char *tag = data + 1080;

        /* List of recognized format tags (possibly incomplete) */
        static const struct {
            char name[5];
            char channels;
        } tags[] = {
            /* TODO: FLT8 intentionally omitted because I haven't been able */
            /* to find a specimen to test its funky pattern pairing format  */
            {"M.K.",  4}, {"M!K!",  4}, {"FLT4",  4}, {"4CHN",  4},
            {"OKTA",  8}, {"OCTA",  8}, {"CD81",  8}, {"FA08",  8},
            {"1CHN",  1}, {"2CHN",  2}, {"3CHN",  3}, {"4CHN",  4},
            {"5CHN",  5}, {"6CHN",  6}, {"7CHN",  7}, {"8CHN",  8},
            {"9CHN",  9}, {"10CH", 10}, {"11CH", 11}, {"12CH", 12},
            {"13CH", 13}, {"14CH", 14}, {"15CH", 15}, {"16CH", 16},
            {"17CH", 17}, {"18CH", 18}, {"19CH", 19}, {"20CH", 20},
            {"21CH", 21}, {"22CH", 22}, {"23CH", 23}, {"24CH", 24},
            {"25CH", 25}, {"26CH", 26}, {"27CH", 27}, {"28CH", 28},
            {"29CH", 29}, {"30CH", 30}, {"31CH", 31}, {"32CH", 32}
        };

        /* Check the format tag to determine if this is a 31-sample MOD */
        for (i = 0; i < (int) (sizeof(tags) / sizeof(*tags)); i++) {
            if (tags[i].name[0] == tag[0] && tags[i].name[1] == tag[1]
             && tags[i].name[2] == tag[2] && tags[i].name[3] == tag[3]) {
                c->num_channels = tags[i].channels;
                c->length = data[950];
                c->reset = data[951];
                c->order = &data[952];
                c->patterns = &data[1084];
                c->num_samples = 31;
                return 1;
            }
        }
    }

    /* A 15-instrument MOD has to be at least 600 bytes long */
    if (size < 600) {
        return 0;
    }

    /* Check that the song title only contains ASCII bytes (or null) */
    for (i = 0; i < 20; i++) {
        if (data[i] != '\0' && (data[i] < ' ' || data[i] > '~')) {
            return 0;
        }
    }

    /* Check that sample names only contain ASCII bytes (or null) */
    for (i = 0; i < 15; i++) {
        for (j = 0; j < 22; j++) {
            char chr = data[20 + i * 30 + j];
            if (chr != '\0' && (chr < ' ' || chr > '~')) {
                return 0;
            }
        }
    }

    /* It looks like we have an older 15-instrument MOD */
    c->length = data[470];
    c->reset = data[471];
    c->order = &data[472];
    c->patterns = &data[600];
    c->num_samples = 15;
    c->num_channels = 4;
    return 1;
}

int pocketmod_init(pocketmod_context *c, const void *data, int size, int rate)
{
    int i, remaining, header_bytes, pattern_bytes;
    unsigned char *byte = (unsigned char*) c;
    signed char *sample_data;

    /* Check that arguments look more or less sane */
    if (!c || !data || rate <= 0 || size <= 0) {
        return 0;
    }

    /* Zero out the whole context and identify the MOD type */
    _pocketmod_zero(c, sizeof(pocketmod_context));
    c->source = (unsigned char*) data;
    if (!_pocketmod_ident(c, c->source, size)) {
        return 0;
    }

    /* Check that we are compiled with support for enough channels */
    if (c->num_channels > POCKETMOD_MAX_CHANNELS) {
        return 0;
    }

    /* Check that we have enough sample slots for this file */
    if (POCKETMOD_MAX_SAMPLES < 31) {
        byte = (unsigned char*) data + 20;
        for (i = 0; i < c->num_samples; i++) {
            unsigned int length = 2 * ((byte[22] << 8) | byte[23]);
            if (i >= POCKETMOD_MAX_SAMPLES && length > 2) {
                return 0; /* Can't fit this sample */
            }
            byte += 30;
        }
    }

    /* Check that the song length is in valid range (1..128) */
    if (c->length == 0 || c->length > 128) {
        return 0;
    }

    /* Make sure that the reset pattern doesn't take us out of bounds */
    if (c->reset >= c->length) {
        c->reset = 0;
    }

    /* Count how many patterns there are in the file */
    c->num_patterns = 0;
    for (i = 0; i < 128 && c->order[i] < 128; i++) {
        c->num_patterns = _pocketmod_max(c->num_patterns, c->order[i]);
    }
    pattern_bytes = 256 * c->num_channels * ++c->num_patterns;
    header_bytes = (int) ((char*) c->patterns - (char*) data);

    /* Check that each pattern in the order is within file bounds */
    for (i = 0; i < c->length; i++) {
        if (header_bytes + 256 * c->num_channels * c->order[i] > size) {
            return 0; /* Reading this pattern would be a buffer over-read! */
        }
    }

    /* Check that the pattern data doesn't extend past the end of the file */
    if (header_bytes + pattern_bytes > size) {
        return 0;
    }

    /* Load sample payload data, truncating ones that extend outside the file */
    remaining = size - header_bytes - pattern_bytes;
    sample_data = (signed char*) data + header_bytes + pattern_bytes;
    for (i = 0; i < c->num_samples; i++) {
        unsigned char *data = POCKETMOD_SAMPLE(c, i + 1);
        unsigned int length = ((data[0] << 8) | data[1]) << 1;
        _pocketmod_sample *sample = &c->samples[i];
        sample->data = sample_data;
        sample->length = _pocketmod_min(length > 2 ? length : 0, remaining);
        sample_data += sample->length;
        remaining -= sample->length;
    }

    /* Set up ProTracker default panning for all channels */
    for (i = 0; i < c->num_channels; i++) {
        c->channels[i].balance = 0x80 + ((((i + 1) >> 1) & 1) ? 0x20 : -0x20);
    }

    /* Prepare to render from the start */
    c->ticks_per_line = 6;
    c->samples_per_second = rate;
    c->samples_per_tick = rate / 50.0f;
    c->lfo_rng = 0xbadc0de;
    c->line = -1;
    c->tick = c->ticks_per_line - 1;
    _pocketmod_next_tick(c);
    return 1;
}

int pocketmod_render(pocketmod_context *c, void *buffer, int buffer_size)
{
    int i, samples_rendered = 0;
    int samples_remaining = buffer_size / POCKETMOD_SAMPLE_SIZE;
    if (c && buffer) {
        float (*output)[2] = (float(*)[2]) buffer;
        while (samples_remaining > 0) {

            /* Calculate the number of samples left in this tick */
            int num = (int) (c->samples_per_tick - c->sample);
            num = _pocketmod_min(num + !num, samples_remaining);

            /* Render and mix 'num' samples from each channel */
            _pocketmod_zero(output, num * POCKETMOD_SAMPLE_SIZE);
            for (i = 0; i < c->num_channels; i++) {
                _pocketmod_chan *chan = &c->channels[i];
                if (chan->sample != 0 && chan->position >= 0.0f) {
                    _pocketmod_render_channel(c, chan, *output, num);
                }
            }
            samples_remaining -= num;
            samples_rendered += num;
            output += num;

            /* Advance song position by 'num' samples */
            if ((c->sample += num) >= c->samples_per_tick) {
                c->sample -= c->samples_per_tick;
                _pocketmod_next_tick(c);

                /* Stop if a new pattern was reached */
                if (c->line == 0 && c->tick == 0) {

                    /* Increment loop counter as needed */
                    if (c->visited[c->pattern >> 3] & (1 << (c->pattern & 7))) {
                        _pocketmod_zero(c->visited, sizeof(c->visited));
                        c->loop_count++;
                    }
                    break;
                }
            }
        }
    }
    return samples_rendered * POCKETMOD_SAMPLE_SIZE;
}

int pocketmod_loop_count(pocketmod_context *c)
{
    return c->loop_count;
}

#endif /* #ifdef POCKETMOD_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef POCKETMOD_H_INCLUDED */

/*******************************************************************************

MIT License

Copyright (c) 2018 rombankzero

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*******************************************************************************/
