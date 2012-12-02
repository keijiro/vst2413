/***********************************************************************************

  emu2413.c -- YM2413 emulator written by Mitsutaka Okazaki 2001

  2001 01-08 : Version 0.10 -- 1st version.
  2001 01-15 : Version 0.20 -- semi-public version.
  2001 01-16 : Version 0.30 -- 1st public version.
  2001 01-17 : Version 0.31 -- Fixed bassdrum problem.
             : Version 0.32 -- LPF implemented.
  2001 01-18 : Version 0.33 -- Fixed the drum problem, refine the mix-down method.
                            -- Fixed the LFO bug.
  2001 01-24 : Version 0.35 -- Fixed the drum problem, 
                               support undocumented EG behavior.
  2001 02-02 : Version 0.38 -- Improved the performance.
                               Fixed the hi-hat and cymbal model.
                               Fixed the default percussive datas.
                               Noise reduction.
                               Fixed the feedback problem.
  2001 03-03 : Version 0.39 -- Fixed some drum bugs.
                               Improved the performance.
  2001 03-04 : Version 0.40 -- Improved the feedback.
                               Change the default table size.
                               Clock and Rate can be changed during play.
  2001 06-24 : Version 0.50 -- Improved the hi-hat and the cymbal tone.
                               Added VRC7 patch (OPLL_reset_patch is changed).
                               Fixed OPLL_reset() bug.
                               Added OPLL_setMask, OPLL_getMask and OPLL_toggleMask.
                               Added OPLL_writeIO.
  2001 09-28 : Version 0.51 -- Remove the noise table.

  References: 
    fmopl.c        -- 1999,2000 written by Tatsuyuki Satoh (MAME development).
    s_opl.c        -- 2001 written by Mamiya (NEZplug development).
    fmgen.cpp      -- 1999,2000 written by cisc.
    fmpac.ill      -- 2000 created by NARUTO.
    MSX-Datapack
    YMU757 data sheet
    YM2143 data sheet

**************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "emu2413.h"

#if defined(_MSC_VER)
#define INLINE __forceinline
#elif defined(__GNUC__)
#define INLINE __inline__
#else
#define INLINE
#endif

#define EMU2413_COMPACTION 1
#ifdef EMU2413_COMPACTION
#define OPLL_TONE_NUM 1
static unsigned char default_inst[OPLL_TONE_NUM][(16+3)*16]=
{
  {
      /* YM2413 VOICE */
//      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x03,0x01,0x9a,0x04,0xf3,0xf4,0x13,0x23,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x61,0x61,0x1e,0x17,0xf0,0x7f,0x07,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x13,0x41,0x0f,0x0d,0xce,0xf5,0x43,0x23,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x03,0x01,0x9a,0x04,0xf3,0xf4,0x13,0x23,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x21,0x61,0x1d,0x07,0xfa,0x64,0x30,0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x22,0x21,0x1e,0x06,0xf0,0x76,0x18,0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x31,0x02,0x16,0x05,0x90,0x71,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x21,0x61,0x1d,0x07,0x82,0x80,0x10,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x23,0x21,0x2d,0x16,0xc0,0x70,0x07,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x61,0x21,0x1b,0x06,0x64,0x65,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x61,0x61,0x0c,0x18,0x85,0xa0,0x79,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x23,0x21,0x87,0x11,0xf0,0xa4,0x00,0xf7,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x97,0xe1,0x28,0x07,0xff,0xf3,0x02,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x61,0x10,0x0c,0x05,0xf2,0xc4,0x40,0xc8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x01,0x01,0x56,0x03,0xb4,0xb2,0x23,0x58,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x61,0x41,0x89,0x03,0xf1,0xf4,0xf0,0x13,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x04,0x21,0x16,0x00,0xdf,0xf8,0xff,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x23,0x32,0x00,0x00,0xd8,0xf7,0xf8,0xf7,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
      0x25,0x18,0x00,0x00,0xf8,0xda,0xf8,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
  }
};
#else
#define OPLL_TONE_NUM 2
static unsigned char default_inst[OPLL_TONE_NUM][(16+3)*16]=
{
  { 
#include "2413tone.h" 
  },
  { 
#include "vrc7tone.h" 
  }
};
#endif

/* Size of Sintable ( 1 -- 18 can be used, but 7 -- 14 recommended.)*/
#define PG_BITS 9
#define PG_WIDTH (1<<PG_BITS)

/* Phase increment counter */
#define DP_BITS 18
#define DP_WIDTH (1<<DP_BITS)
#define DP_BASE_BITS (DP_BITS - PG_BITS)

/* Dynamic range (Accuracy of sin table) */
#define DB_STEP (48.0/(1<<DB_BITS))
#define DB_BITS 8
#define DB_MUTE (1<<DB_BITS)

/* Dynamic range of envelope */
#define EG_STEP 0.375
#define EG_BITS 7
#define EG_MUTE (1<<EB_BITS)

/* Dynamic range of total level */
#define TL_STEP 0.75
#define TL_BITS 6
#define TL_MUTE (1<<TL_BITS)

/* Dynamic range of sustine level */
#define SL_STEP 3.0
#define SL_BITS 4
#define SL_MUTE (1<<SL_BITS)

#define EG2DB(d) ((d)*(e_int32)(EG_STEP/DB_STEP))
#define TL2EG(d) ((d)*(e_int32)(TL_STEP/EG_STEP))
#define SL2EG(d) ((d)*(e_int32)(SL_STEP/EG_STEP))

#define DB_POS(x) (e_uint32)((x)/DB_STEP)
#define DB_NEG(x) (e_uint32)(DB_MUTE+DB_MUTE+(x)/DB_STEP)

/* Bits for liner value */
#define DB2LIN_AMP_BITS 11
#define SLOT_AMP_BITS (DB2LIN_AMP_BITS)

/* Bits for envelope phase incremental counter */  
#define EG_DP_BITS 22
#define EG_DP_WIDTH (1<<EG_DP_BITS)

/* Bits for Pitch and Amp modulator */
#define PM_PG_BITS 8
#define PM_PG_WIDTH (1<<PM_PG_BITS)
#define PM_DP_BITS 16
#define PM_DP_WIDTH (1<<PM_DP_BITS)
#define AM_PG_BITS 8
#define AM_PG_WIDTH (1<<AM_PG_BITS)
#define AM_DP_BITS 16
#define AM_DP_WIDTH (1<<AM_DP_BITS)

/* PM table is calcurated by PM_AMP * pow(2,PM_DEPTH*sin(x)/1200) */
#define PM_AMP_BITS 8
#define PM_AMP (1<<PM_AMP_BITS)

/* PM speed(Hz) and depth(cent) */
#define PM_SPEED 6.4
#define PM_DEPTH 13.75

/* AM speed(Hz) and depth(dB) */
#define AM_SPEED 3.7
#define AM_DEPTH 2.4

/* Cut the lower b bit(s) off. */
#define HIGHBITS(c,b) ((c)>>(b))

/* Leave the lower b bit(s). */
#define LOWBITS(c,b) ((c)&((1<<(b))-1))

/* Expand x which is s bits to d bits. */
#define EXPAND_BITS(x,s,d) ((x)<<((d)-(s)))

/* Expand x which is s bits to d bits and fill expanded bits '1' */
#define EXPAND_BITS_X(x,s,d) (((x)<<((d)-(s)))|((1<<((d)-(s)))-1))

/* Adjust envelope speed which depends on sampling rate. */
#define rate_adjust(x) (rate==49716?x:(e_uint32)((double)(x)*clk/72/rate + 0.5)) /* +0.5 to round */

#define MOD(x) ch[x]->mod
#define CAR(x) ch[x]->car

/* Sampling rate */
static e_uint32 rate ;
/* Input clock */
static e_uint32 clk ;

/* WaveTable for each envelope amp */
static e_uint16 fullsintable[PG_WIDTH] ;
static e_uint16 halfsintable[PG_WIDTH] ;

static e_uint32 dphaseNoiseTable[512][8] ;

static e_uint16 *waveform[2] = {fullsintable,halfsintable} ;

/* LFO Table */
static e_int32 pmtable[PM_PG_WIDTH] ;
static e_int32 amtable[AM_PG_WIDTH] ;

/* Noise and LFO */
static e_uint32 pm_dphase ;
static e_uint32 am_dphase ;

/* dB to Liner table */
static e_int16 DB2LIN_TABLE[(DB_MUTE + DB_MUTE)*2] ;

/* Liner to Log curve conversion table (for Attack rate). */
static e_uint16 AR_ADJUST_TABLE[1<<EG_BITS] ;

/* Empty voice data */
static OPLL_PATCH null_patch = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } ;

/* Basic voice Data */
static OPLL_PATCH default_patch[OPLL_TONE_NUM][(16+3)*2] ; 


/* Definition of envelope mode */
enum { SETTLE,ATTACK,DECAY,SUSHOLD,SUSTINE,RELEASE,FINISH } ;

/* Phase incr table for Attack */
static e_uint32 dphaseARTable[16][16] ;
/* Phase incr table for Decay and Release */
static e_uint32 dphaseDRTable[16][16] ;

/* KSL + TL Table */
static e_uint32 tllTable[16][8][1<<TL_BITS][4] ;
static e_int32 rksTable[2][8][2] ;

/* Phase incr table for PG */
static e_uint32 dphaseTable[512][8][16] ; 

/***************************************************
 
                  Create tables
 
****************************************************/
INLINE static e_int32 Min(e_int32 i,e_int32 j)
{
  if(i<j) return i ; else return j ;
}

/* Table for AR to LogCurve. */
static void makeAdjustTable(void)
{
  e_int32 i ;

  AR_ADJUST_TABLE[0] = (1<<EG_BITS) ;
  for ( i=1 ; i < 128 ; i++)
    AR_ADJUST_TABLE[i] = (e_uint32)((double)(1<<EG_BITS) - 1 - (1<<EG_BITS) * log(i) / log(128)) ; 
}


/* Table for dB(0 -- (1<<DB_BITS)) to Liner(0 -- DB2LIN_AMP_WIDTH) */
static void makeDB2LinTable(void)
{
  e_int32 i ;

  for( i=0 ; i < DB_MUTE + DB_MUTE ; i++)
  {
    DB2LIN_TABLE[i] = (e_int32)((double)((1<<DB2LIN_AMP_BITS)-1) * pow(10,-(double)i*DB_STEP/20)) ;
    if(i>=DB_MUTE) DB2LIN_TABLE[i] = 0 ;
    DB2LIN_TABLE[i+ DB_MUTE + DB_MUTE] = -DB2LIN_TABLE[i] ;
  }
}

/* Liner(+0.0 - +1.0) to dB((1<<DB_BITS) - 1 -- 0) */
static e_int32 lin2db(double d)
{
  if(d == 0) return (DB_MUTE - 1) ;
  else return Min(-(e_int32)(20.0*log10(d)/DB_STEP), DB_MUTE - 1) ; /* 0 -- 127 */
}


/* Sin Table */
static void makeSinTable(void)
{
  e_int32 i ;

  for( i = 0 ; i < PG_WIDTH/4 ; i++ )
  {
    fullsintable[i] = lin2db(sin(2.0*PI*i/PG_WIDTH)) ;
  }

  for( i = 0 ; i < PG_WIDTH/4 ; i++ )
  {
    fullsintable[PG_WIDTH/2 - 1 - i] = fullsintable[i] ;
  }

  for( i = 0 ; i < PG_WIDTH/2 ; i++ )
  {
    fullsintable[PG_WIDTH/2+i] = DB_MUTE + DB_MUTE + fullsintable[i] ;
  }
  
  for( i = 0 ; i < PG_WIDTH/2 ; i++ ) halfsintable[i] = fullsintable[i] ;
  for( i = PG_WIDTH/2 ; i< PG_WIDTH ; i++ ) halfsintable[i] = fullsintable[0] ;
}

static void makeDphaseNoiseTable(void)
{
  int i, j ;

  for(i=0;i<512;i++)
    for(j=0;j<8;j++)
      dphaseNoiseTable[i][j] = rate_adjust(i<<j) ;
}

/* Table for Pitch Modulator */
static void makePmTable(void)
{
  e_int32 i ;

  for(i = 0 ; i < PM_PG_WIDTH ; i++ )
    pmtable[i] = (e_int32)((double)PM_AMP * pow(2,(double)PM_DEPTH*sin(2.0*PI*i/PM_PG_WIDTH)/1200)) ;
}

/* Table for Amp Modulator */
static void makeAmTable(void)
{
  e_int32 i ;

  for(i = 0 ; i < AM_PG_WIDTH ; i++ )
    amtable[i] = (e_int32)((double)AM_DEPTH/2/DB_STEP * (1.0 + sin(2.0*PI*i/PM_PG_WIDTH))) ;
}

/* Phase increment counter table */ 
static void makeDphaseTable(void)
{
  e_uint32 fnum, block , ML ;
  e_uint32 mltable[16]={ 1,1*2,2*2,3*2,4*2,5*2,6*2,7*2,8*2,9*2,10*2,10*2,12*2,12*2,15*2,15*2 } ;

  for(fnum=0; fnum<512; fnum++)
    for(block=0; block<8; block++)
      for(ML=0; ML<16; ML++)
        dphaseTable[fnum][block][ML] = rate_adjust(((fnum * mltable[ML])<<block)>>(20-DP_BITS)) ;
}

static void makeTllTable(void)
{
#define dB2(x) (e_uint32)((x)*2)

	static e_uint32 kltable[16] = {
    dB2( 0.000),dB2( 9.000),dB2(12.000),dB2(13.875),dB2(15.000),dB2(16.125),dB2(16.875),dB2(17.625),
    dB2(18.000),dB2(18.750),dB2(19.125),dB2(19.500),dB2(19.875),dB2(20.250),dB2(20.625),dB2(21.000)
	} ;
  
  e_int32 tmp ;
  e_int32 fnum, block ,TL , KL ;

  for(fnum=0; fnum<16; fnum++)
    for(block=0; block<8; block++)
      for(TL=0; TL<64; TL++)
        for(KL=0; KL<4; KL++)
        {
          if(KL==0)
          {
            tllTable[fnum][block][TL][KL] = TL2EG(TL) ;
          }
          else
          {
  	        tmp = kltable[fnum] - dB2(3.000) * (7 - block) ;
            if(tmp <= 0)
              tllTable[fnum][block][TL][KL] = TL2EG(TL) ;
            else 
              tllTable[fnum][block][TL][KL] = (e_uint32)((tmp>>(3-KL))/EG_STEP) + TL2EG(TL) ;
          }
       }
}

/* Rate Table for Attack */
static void makeDphaseARTable(void)
{
  e_int32 AR,Rks,RM,RL ;

  for(AR=0; AR<16; AR++)
    for(Rks=0; Rks<16; Rks++)
    {
      RM = AR + (Rks>>2) ;
      if(RM>15) RM = 15 ;
      RL = Rks&3 ;
      switch(AR)
      { 
        case 0:
          dphaseARTable[AR][Rks] = 0 ;
          break ;
        case 15:
          dphaseARTable[AR][Rks] = EG_DP_WIDTH ;
          break ;
        default:
          dphaseARTable[AR][Rks] = rate_adjust(( 3 * (RL + 4) << (RM + 1))) ;
          break ;
      }
    }
}

/* Rate Table for Decay */
static void makeDphaseDRTable(void)
{
  e_int32 DR,Rks,RM,RL ;

  for(DR=0; DR<16; DR++)
    for(Rks=0; Rks<16; Rks++)
    {
      RM = DR + (Rks>>2) ;
      RL = Rks&3 ;
      if(RM>15) RM = 15 ;
      switch(DR)
      { 
        case 0:
          dphaseDRTable[DR][Rks] = 0 ;
          break ;
        default:
          dphaseDRTable[DR][Rks] = rate_adjust((RL + 4) << (RM - 1));
          break ;
      }
    }
}

static void makeRksTable(void)
{

  e_int32 fnum8, block, KR ;

  for(fnum8 = 0 ; fnum8 < 2 ; fnum8++)
    for(block = 0 ; block < 8 ; block++)
      for(KR = 0; KR < 2 ; KR++)
      {
        if(KR!=0)
          rksTable[fnum8][block][KR] = ( block << 1 ) + fnum8 ;
        else
          rksTable[fnum8][block][KR] = block >> 1 ;
      }
}

void OPLL_dump2patch(const e_uint8 *dump, OPLL_PATCH *patch)
{
  patch[0].AM = (dump[0]>>7)&1 ;
  patch[1].AM = (dump[1]>>7)&1 ;
  patch[0].PM = (dump[0]>>6)&1 ;
  patch[1].PM = (dump[1]>>6)&1 ;
  patch[0].EG = (dump[0]>>5)&1 ;
  patch[1].EG = (dump[1]>>5)&1 ;
  patch[0].KR = (dump[0]>>4)&1 ;
  patch[1].KR = (dump[1]>>4)&1 ;
  patch[0].ML = (dump[0])&15 ;
  patch[1].ML = (dump[1])&15 ;
  patch[0].KL = (dump[2]>>6)&3 ;
  patch[1].KL = (dump[3]>>6)&3 ;
  patch[0].TL = (dump[2])&63 ;
  patch[0].FB = (dump[3])&7 ;
  patch[0].WF = (dump[3]>>3)&1 ;
  patch[1].WF = (dump[3]>>4)&1 ;
  patch[0].AR = (dump[4]>>4)&15 ;
  patch[1].AR = (dump[5]>>4)&15 ;
  patch[0].DR = (dump[4])&15 ;
  patch[1].DR = (dump[5])&15 ;
  patch[0].SL = (dump[6]>>4)&15 ;
  patch[1].SL = (dump[7]>>4)&15 ;
  patch[0].RR = (dump[6])&15 ;
  patch[1].RR = (dump[7])&15 ;
}

void OPLL_getDefaultPatch(e_int32 type, e_int32 num, OPLL_PATCH *patch)
{
  OPLL_dump2patch(default_inst[type]+num*16,patch) ;
}

static void makeDefaultPatch()
{
  e_int32 i, j ;

  for(i=0;i<OPLL_TONE_NUM;i++)
    for(j=0;j<19;j++)
      OPLL_getDefaultPatch(i,j,&default_patch[i][j*2]) ;
    
}

#ifndef EMU2413_COMPACTION
void OPLL_setPatch(OPLL *opll, const e_uint8 *dump)
{
  OPLL_PATCH patch[2] ;
  int i ;

  for(i=0;i<19;i++)
  {
    OPLL_dump2patch(dump+i*16, patch) ;
    memcpy(opll->patch[i*2], &patch[0], sizeof(OPLL_PATCH)) ;
    memcpy(opll->patch[i*2+1], &patch[1], sizeof(OPLL_PATCH)) ;
  }
}

void OPLL_patch2dump(const OPLL_PATCH *patch, e_uint8 *dump)
{
  dump[0] = (patch[0].AM<<7) + (patch[0].PM<<6) + (patch[0].EG<<5) + (patch[0].KR<<4) + patch[0].ML ;
  dump[1] = (patch[1].AM<<7) + (patch[1].PM<<6) + (patch[1].EG<<5) + (patch[1].KR<<4) + patch[1].ML ;
  dump[2] = (patch[0].KL<<6) + patch[0].TL ;
  dump[3] = (patch[1].KL<<6) + (patch[1].WF<<4) + (patch[0].WF<<3) + patch[0].FB ;
  dump[4] = (patch[0].AR<<4) + patch[0].DR ;
  dump[5] = (patch[1].AR<<4) + patch[1].DR ;
  dump[6] = (patch[0].SL<<4) + patch[0].RR ;
  dump[7] = (patch[1].SL<<4) + patch[1].RR ;
  dump[8] = 0 ;
  dump[9] = 0 ;
  dump[10] = 0 ;
  dump[11] = 0 ;
  dump[12] = 0 ;
  dump[13] = 0 ;
  dump[14] = 0 ;
  dump[15] = 0 ;
}
#endif

/************************************************************

                      Calc Parameters

************************************************************/

INLINE static e_uint32 calc_eg_dphase(OPLL_SLOT *slot)
{

  switch(slot->eg_mode)
  {
    case ATTACK:
      return dphaseARTable[slot->patch->AR][slot->rks] ;
  
    case DECAY:
      return dphaseDRTable[slot->patch->DR][slot->rks] ;
  
    case SUSHOLD:
      return 0 ;

    case SUSTINE:
      return dphaseDRTable[slot->patch->RR][slot->rks] ;
  
    case RELEASE:
      if(slot->sustine)
        return dphaseDRTable[5][slot->rks] ;
      else if(slot->patch->EG)
        return dphaseDRTable[slot->patch->RR][slot->rks] ;
      else 
        return dphaseDRTable[7][slot->rks] ;

    case FINISH:
      return 0 ;

    default:
      return 0 ;
  }
}

/*************************************************************

                    OPLL internal interfaces

*************************************************************/
#define SLOT_BD1 12
#define SLOT_BD2 13
#define SLOT_HH 14
#define SLOT_SD 15
#define SLOT_TOM 16
#define SLOT_CYM 17

#define UPDATE_PG(S)  (S)->dphase = dphaseTable[(S)->fnum][(S)->block][(S)->patch->ML]
#define UPDATE_TLL(S)\
(((S)->type==0)?\
((S)->tll = tllTable[((S)->fnum)>>5][(S)->block][(S)->patch->TL][(S)->patch->KL]):\
((S)->tll = tllTable[((S)->fnum)>>5][(S)->block][(S)->volume][(S)->patch->KL]))
#define UPDATE_RKS(S) (S)->rks = rksTable[((S)->fnum)>>8][(S)->block][(S)->patch->KR] 
#define UPDATE_WF(S)  (S)->sintbl = waveform[(S)->patch->WF] 
#define UPDATE_EG(S)  (S)->eg_dphase = calc_eg_dphase(S) 
#define UPDATE_ALL(S)\
  UPDATE_PG(S);\
  UPDATE_TLL(S);\
  UPDATE_RKS(S);\
  UPDATE_WF(S); \
  UPDATE_EG(S) /* EG should be updated last. */

/* Force Refresh (When external program changes some parameters). */
void OPLL_forceRefresh(OPLL *opll)
{
  e_int32 i ;

  if(opll==NULL) return ;

  for(i=0; i<18 ;i++)
  {
    UPDATE_PG(opll->slot[i]) ;
    UPDATE_RKS(opll->slot[i]) ;
    UPDATE_TLL(opll->slot[i]) ;
    UPDATE_WF(opll->slot[i]) ;
    UPDATE_EG(opll->slot[i]) ;
  }
}

/* Slot key on  */
INLINE static void slotOn(OPLL_SLOT *slot)
{
  slot->eg_mode = ATTACK ;
  slot->phase = 0 ;
  slot->eg_phase = 0 ;
}

/* Slot key off */
INLINE static void slotOff(OPLL_SLOT *slot)
{
  if(slot->eg_mode == ATTACK)
    slot->eg_phase = EXPAND_BITS(AR_ADJUST_TABLE[HIGHBITS(slot->eg_phase,EG_DP_BITS-EG_BITS)],EG_BITS,EG_DP_BITS) ;
  slot->eg_mode = RELEASE ;
}

/* Channel key on */
INLINE static void keyOn(OPLL *opll, e_int32 i)
{
  if(!opll->slot_on_flag[i*2]) slotOn(opll->MOD(i)) ;
  if(!opll->slot_on_flag[i*2+1]) slotOn(opll->CAR(i)) ;
  opll->ch[i]->key_status = 1 ;
}

/* Channel key off */
INLINE static void keyOff(OPLL *opll, e_int32 i)
{
  if(opll->slot_on_flag[i*2+1]) slotOff(opll->CAR(i)) ;
  opll->ch[i]->key_status = 0 ;
}

INLINE static void keyOn_BD(OPLL *opll){ keyOn(opll,6) ; }
INLINE static void keyOn_SD(OPLL *opll){ if(!opll->slot_on_flag[SLOT_SD]) slotOn(opll->CAR(7)) ; }
INLINE static void keyOn_TOM(OPLL *opll){ if(!opll->slot_on_flag[SLOT_TOM]) slotOn(opll->MOD(8)) ; }
INLINE static void keyOn_HH(OPLL *opll){ if(!opll->slot_on_flag[SLOT_HH]) slotOn(opll->MOD(7)) ; }
INLINE static void keyOn_CYM(OPLL *opll){ if(!opll->slot_on_flag[SLOT_CYM]) slotOn(opll->CAR(8)) ; }

/* Drum key off */
INLINE static void keyOff_BD(OPLL *opll){ keyOff(opll,6) ; }
INLINE static void keyOff_SD(OPLL *opll){ if(opll->slot_on_flag[SLOT_SD]) slotOff(opll->CAR(7)) ; }
INLINE static void keyOff_TOM(OPLL *opll){ if(opll->slot_on_flag[SLOT_TOM]) slotOff(opll->MOD(8)) ; }
INLINE static void keyOff_HH(OPLL *opll){ if(opll->slot_on_flag[SLOT_HH]) slotOff(opll->MOD(7)) ; }
INLINE static void keyOff_CYM(OPLL *opll){ if(opll->slot_on_flag[SLOT_CYM]) slotOff(opll->CAR(8)) ; }

/* Change a voice */
INLINE static void setPatch(OPLL *opll, e_int32 i, e_int32 num)
{
  opll->ch[i]->patch_number = num ;
  opll->MOD(i)->patch = opll->patch[num*2+0] ;
  opll->CAR(i)->patch = opll->patch[num*2+1] ;
}

/* Change a rhythm voice */
INLINE static void setSlotPatch(OPLL_SLOT *slot, OPLL_PATCH *patch)
{
  slot->patch = patch ;
}

/* Set sustine parameter */
INLINE static void setSustine(OPLL *opll, e_int32 c, e_int32 sustine)
{
  opll->CAR(c)->sustine = sustine ;
  if(opll->MOD(c)->type) opll->MOD(c)->sustine = sustine ;
}

/* Volume : 6bit ( Volume register << 2 ) */
INLINE static void setVolume(OPLL *opll, e_int32 c, e_int32 volume)
{
  opll->CAR(c)->volume = volume ;
}

INLINE static void setSlotVolume(OPLL_SLOT *slot, e_int32 volume)
{
  slot->volume = volume ;
}

/* Set F-Number ( fnum : 9bit ) */
INLINE static void setFnumber(OPLL *opll, e_int32 c, e_int32 fnum)
{
  opll->CAR(c)->fnum = fnum ;
  opll->MOD(c)->fnum = fnum ;
}

/* Set Block data (block : 3bit ) */
INLINE static void setBlock(OPLL *opll, e_int32 c, e_int32 block)
{
  opll->CAR(c)->block = block ;
  opll->MOD(c)->block = block ;
}

/* Change Rhythm Mode */
INLINE static void setRythmMode(OPLL *opll, e_int32 data)
{
  if(opll->rythm_mode == (data&32)>>5 ) return ;

  opll->rythm_mode = (data&32)>>5 ;

  if(data&32)
  {
    /* OFF->ON */
    opll->ch[6]->patch_number = 16 ;
    opll->ch[7]->patch_number = 17 ;
    opll->ch[8]->patch_number = 18 ;
    setSlotPatch(opll->slot[SLOT_BD1], opll->patch[16*2+0]) ;
    setSlotPatch(opll->slot[SLOT_BD2], opll->patch[16*2+1]) ;
    setSlotPatch(opll->slot[SLOT_HH], opll->patch[17*2+0]) ;
    setSlotPatch(opll->slot[SLOT_SD], opll->patch[17*2+1]) ;
    opll->slot[SLOT_HH]->type = 1 ;
    setSlotPatch(opll->slot[SLOT_TOM], opll->patch[18*2+0]) ;
    setSlotPatch(opll->slot[SLOT_CYM], opll->patch[18*2+1]) ;
    opll->slot[SLOT_TOM]->type = 1 ;
  }
  else
  {
    /* ON->OFF */
    setPatch(opll, 6, opll->reg[0x36]>>4) ;
    setPatch(opll, 7, opll->reg[0x37]>>4) ;
    opll->slot[SLOT_HH]->type = 0 ;
    setPatch(opll, 8, opll->reg[0x38]>>4) ;
    opll->slot[SLOT_TOM]->type = 0 ;

    if(!(opll->reg[0x26]&0x10)&&!(data&0x10))
      opll->slot[SLOT_BD1]->eg_mode = FINISH ;
    if(!(opll->reg[0x26]&0x10)&&!(data&0x10))
      opll->slot[SLOT_BD2]->eg_mode = FINISH ;
    if(!(opll->reg[0x27]&0x10)&&!(data&0x08))
      opll->slot[SLOT_HH]->eg_mode = FINISH ;
    if(!(opll->reg[0x27]&0x10)&&!(data&0x04))
      opll->slot[SLOT_SD]->eg_mode = FINISH ;
    if(!(opll->reg[0x28]&0x10)&&!(data&0x02))
      opll->slot[SLOT_TOM]->eg_mode = FINISH ;
    if(!(opll->reg[0x28]&0x10)&&!(data&0x01))
      opll->slot[SLOT_CYM]->eg_mode = FINISH ;
  }
}

void OPLL_copyPatch(OPLL *opll, e_int32 num, OPLL_PATCH *patch)
{
  memcpy(opll->patch[num],patch,sizeof(OPLL_PATCH)) ;
}

/***********************************************************

                      Initializing

***********************************************************/

static void OPLL_SLOT_reset(OPLL_SLOT *slot)
{
  slot->sintbl = waveform[0] ;
  slot->phase = 0 ;
  slot->dphase = 0 ;
  slot->output[0] = 0 ;
  slot->output[1] = 0 ;
  slot->feedback = 0 ;
  slot->eg_mode = SETTLE ;
  slot->eg_phase = EG_DP_WIDTH ;
  slot->eg_dphase = 0 ;
  slot->rks = 0 ;
  slot->tll = 0 ;
  slot->sustine = 0 ;
  slot->fnum = 0 ;
  slot->block = 0 ;
  slot->volume = 0 ;
  slot->pgout = 0 ;
  slot->egout = 0 ;
  slot->patch = &null_patch ;
}

static OPLL_SLOT *OPLL_SLOT_new(void)
{
  OPLL_SLOT *slot ;

  slot = malloc(sizeof(OPLL_SLOT)) ;
  if(slot == NULL) return NULL ;
  
  return slot ;
}

static void OPLL_SLOT_delete(OPLL_SLOT *slot)
{
  free(slot) ;
}

static void OPLL_CH_reset(OPLL_CH *ch)
{
  if(ch->mod!=NULL) OPLL_SLOT_reset(ch->mod) ;
  if(ch->car!=NULL) OPLL_SLOT_reset(ch->car) ;
  ch->key_status = 0 ;
}

static OPLL_CH *OPLL_CH_new(void)
{
  OPLL_CH *ch ;
  OPLL_SLOT *mod, *car ;

  mod = OPLL_SLOT_new() ;
  if(mod == NULL) return NULL ;

  car = OPLL_SLOT_new() ;
  if(car == NULL)
  {
    OPLL_SLOT_delete(mod) ;
    return NULL ;
  }

  ch = malloc(sizeof(OPLL_CH)) ;
  if(ch == NULL)
  {
    OPLL_SLOT_delete(mod) ;
    OPLL_SLOT_delete(car) ;
    return NULL ;
  }
  
  mod->type = 0 ;
  car->type = 1 ;
  ch->mod = mod ;
  ch->car = car ;

  return ch ;
}


static void OPLL_CH_delete(OPLL_CH *ch)
{
  OPLL_SLOT_delete(ch->mod) ;
  OPLL_SLOT_delete(ch->car) ;
  free(ch) ;
}


OPLL *OPLL_new(void)
{
  OPLL *opll ;
  OPLL_CH *ch[9] ;
  OPLL_PATCH *patch[19*2] ;
  e_int32 i, j ;

  opll = calloc(sizeof(OPLL),1) ;
  if(opll == NULL) return NULL ;

  for( i = 0 ; i < 19*2 ; i++ )
  {
    patch[i] = calloc(sizeof(OPLL_PATCH),1) ;
    if(patch[i] == NULL)
    {
      for ( j = i ; i > 0 ; i++ ) free(patch[j-1]) ;
      free(opll) ;
      return NULL ;
    }
  }

  for( i = 0 ; i < 9 ; i++ )
  {
    ch[i] = OPLL_CH_new() ;
    if(ch[i]==NULL)
    {
      for ( j = i ; i > 0 ; i++ ) OPLL_CH_delete(ch[j-1]) ;
      for ( j = 0 ; j < 19*2 ; j++ ) free(patch[j]) ;
      free(opll) ;
      return NULL ;
    }
  }
  
  for ( i = 0 ; i < 19*2 ; i++ ) opll->patch[i] = patch[i] ;


  for ( i = 0 ; i <9 ; i++)
  {
    opll->ch[i] = ch[i] ;
    opll->slot[i*2+0] = opll->ch[i]->mod ;
    opll->slot[i*2+1] = opll->ch[i]->car ;
  }

  for ( i = 0 ; i < 18 ; i++)
  {
    opll->slot[i]->plfo_am = &opll->lfo_am ;
    opll->slot[i]->plfo_pm = &opll->lfo_pm ;
  }

  opll->mask = 0 ;

  OPLL_reset(opll) ;
  OPLL_reset_patch(opll,0) ;

  return opll ;
  
}


void OPLL_delete(OPLL *opll)
{
  e_int32 i ;

  for ( i = 0 ; i < 9 ; i++ )
    OPLL_CH_delete(opll->ch[i]) ;

  for ( i = 0 ; i < 19*2 ; i++ )
    free(opll->patch[i]) ;

  free(opll) ;
}


/* Reset patch datas by system default. */
void OPLL_reset_patch(OPLL *opll, e_int32 type)
{
  e_int32 i ;
  
  for ( i = 0 ; i < 19*2 ; i++ )
    OPLL_copyPatch(opll, i, &default_patch[type%OPLL_TONE_NUM][i]) ;
}

/* Reset whole of OPLL except patch datas. */
void OPLL_reset(OPLL *opll)
{
  e_int32 i ;

  if(!opll) return ;

  opll->adr = 0 ;

  opll->output[0] = 0 ;
  opll->output[1] = 0 ;

  opll->pm_phase = 0 ;
  opll->am_phase = 0 ;

  opll->noise_seed =0xffff ;
  opll->noiseA = 0 ;
  opll->noiseB = 0 ;
  opll->noiseA_phase = 0 ;
  opll->noiseB_phase = 0 ;
  opll->noiseA_dphase = 0 ;
  opll->noiseB_dphase = 0 ;

  opll->mask = 0 ;

  for(i = 0; i < 9 ; i++)
  {
    OPLL_CH_reset(opll->ch[i]) ;
    setPatch(opll,i,0) ;
  }

  for ( i = 0 ; i < 0x40 ; i++ ) OPLL_writeReg(opll, i, 0) ;

}

void OPLL_setClock(e_uint32 c, e_uint32 r)
{
  clk = c ;
  rate = r ;
  makeDphaseTable() ;
  makeDphaseARTable() ;
  makeDphaseDRTable() ;
  makeDphaseNoiseTable() ;
  pm_dphase = (e_uint32)rate_adjust(PM_SPEED * PM_DP_WIDTH / (clk/72) ) ;
  am_dphase = (e_uint32)rate_adjust(AM_SPEED * AM_DP_WIDTH / (clk/72) ) ;
}

void OPLL_init(e_uint32 c, e_uint32 r)
{
  makePmTable() ;
  makeAmTable() ;
  makeDB2LinTable() ; 
  makeAdjustTable() ;
  makeTllTable() ;
  makeRksTable() ;
  makeSinTable() ;
  makeDefaultPatch() ;
  OPLL_setClock(c,r) ;
}

void OPLL_close(void)
{
}

/*********************************************************

                 Generate wave data

*********************************************************/
/* Convert Amp(0 to EG_HEIGHT) to Phase(0 to 2PI). */
#if ( SLOT_AMP_BITS - PG_BITS ) > 0
#define wave2_2pi(e)  ( (e) >> ( SLOT_AMP_BITS - PG_BITS ))
#else
#define wave2_2pi(e)  ( (e) << ( PG_BITS - SLOT_AMP_BITS ))
#endif

/* Convert Amp(0 to EG_HEIGHT) to Phase(0 to 4PI). */
#if ( SLOT_AMP_BITS - PG_BITS - 1 ) == 0
#define wave2_4pi(e)  (e)
#elif ( SLOT_AMP_BITS - PG_BITS - 1 ) > 0
#define wave2_4pi(e)  ( (e) >> ( SLOT_AMP_BITS - PG_BITS - 1 ))
#else
#define wave2_4pi(e)  ( (e) << ( 1 + PG_BITS - SLOT_AMP_BITS ))
#endif

/* Convert Amp(0 to EG_HEIGHT) to Phase(0 to 8PI). */
#if ( SLOT_AMP_BITS - PG_BITS - 2 ) == 0
#define wave2_8pi(e)  (e)
#elif ( SLOT_AMP_BITS - PG_BITS - 2 ) > 0
#define wave2_8pi(e)  ( (e) >> ( SLOT_AMP_BITS - PG_BITS - 2 ))
#else
#define wave2_8pi(e)  ( (e) << ( 2 + PG_BITS - SLOT_AMP_BITS ))
#endif

/* Update Noise unit */
INLINE static void update_noise(OPLL *opll)
{
  if(opll->noise_seed&1) opll->noise_seed^=0x12000 ;
  opll->noise_seed >>= 1 ;
  opll->whitenoise = opll->noise_seed&1?DB_POS(6.0):DB_NEG(6.0);

  opll->noiseA_phase += opll->noiseA_dphase ;
  opll->noiseB_phase += opll->noiseB_dphase ;

#if 0
  if(opll->noiseA_phase & (1<<11))
  {
    static int noiseA_seed = 0x01;

    opll->noiseA_phase &= (1<<11) - 1;
    if(noiseA_seed&1) noiseA_seed^=0x14;
    noiseA_seed>>=1;
    opll->noiseA = noiseA_seed&1?DB_POS(6.0):DB_NEG(6.0);
  }
#else
  opll->noiseA_phase &= (0x40<<11) - 1 ;
  opll->noiseA = opll->noiseA_phase&(0x03<<11)?DB_POS(3.0):DB_NEG(3.0);
#endif

  opll->noiseB_phase &= (0x10<<11) - 1;
  opll->noiseB = opll->noiseB_phase&(0x0A<<11)?DB_POS(3.0):DB_NEG(3.0);

}

/* Update AM, PM unit */
INLINE static void update_ampm(OPLL *opll)
{
  opll->pm_phase = (opll->pm_phase + pm_dphase)&(PM_DP_WIDTH - 1) ;
  opll->am_phase = (opll->am_phase + am_dphase)&(AM_DP_WIDTH - 1) ;
  opll->lfo_am = amtable[HIGHBITS(opll->am_phase, AM_DP_BITS - AM_PG_BITS)] ;
  opll->lfo_pm = pmtable[HIGHBITS(opll->pm_phase, PM_DP_BITS - PM_PG_BITS)] ;
}

/* PG */
INLINE static e_uint32 calc_phase(OPLL_SLOT *slot)
{
  if(slot->patch->PM)
    slot->phase += (slot->dphase * (*(slot->plfo_pm))) >> PM_AMP_BITS ;
  else
    slot->phase += slot->dphase ;

  slot->phase &= (DP_WIDTH - 1) ;

  return HIGHBITS(slot->phase, DP_BASE_BITS) ;
}

/* EG */
INLINE static e_uint32 calc_envelope(OPLL_SLOT *slot)
{
  #define S2E(x) (SL2EG((e_int32)(x/SL_STEP))<<(EG_DP_BITS-EG_BITS)) 
  static e_uint32 SL[16] = 
  {
    S2E( 0), S2E( 3), S2E( 6), S2E( 9), S2E(12), S2E(15), S2E(18), S2E(21),
    S2E(24), S2E(27), S2E(30), S2E(33), S2E(36), S2E(39), S2E(42), S2E(48)
  };

  e_uint32 egout ;

  switch(slot->eg_mode)
  {

    case ATTACK:
      slot->eg_phase += slot->eg_dphase ;
      if(EG_DP_WIDTH & slot->eg_phase)
      {
        egout = 0 ;
        slot->eg_phase= 0 ;
        slot->eg_mode = DECAY ;
        UPDATE_EG(slot) ;
      }
      else
      {
        egout = AR_ADJUST_TABLE[HIGHBITS(slot->eg_phase, EG_DP_BITS - EG_BITS)] ;
      }
      break;

    case DECAY:
      slot->eg_phase += slot->eg_dphase ;
      egout = HIGHBITS(slot->eg_phase, EG_DP_BITS - EG_BITS) ;
      if(slot->eg_phase >= SL[slot->patch->SL])
      {
        if(slot->patch->EG)
        {
          slot->eg_phase = SL[slot->patch->SL] ;
	        slot->eg_mode = SUSHOLD ;
          UPDATE_EG(slot) ;
        }
        else
        {
          slot->eg_phase = SL[slot->patch->SL] ;
          slot->eg_mode = SUSTINE ;
          UPDATE_EG(slot) ;
        }
        egout = HIGHBITS(slot->eg_phase, EG_DP_BITS - EG_BITS) ;
      }
      break;

    case SUSHOLD:
      egout = HIGHBITS(slot->eg_phase, EG_DP_BITS - EG_BITS) ;
      if(slot->patch->EG == 0)
      {
        slot->eg_mode = SUSTINE ;
        UPDATE_EG(slot) ;
      }
      break;

    case SUSTINE:
    case RELEASE:
      slot->eg_phase += slot->eg_dphase ;
      egout = HIGHBITS(slot->eg_phase, EG_DP_BITS - EG_BITS) ; 
      if(egout >= (1<<EG_BITS))
      {
        slot->eg_mode = FINISH ;
        egout = (1<<EG_BITS) - 1 ;
      }
      break;

    case FINISH:
      egout = (1<<EG_BITS) - 1 ;
      break ;

    default:
      egout = (1<<EG_BITS) - 1 ;
      break;
  }

  if(slot->patch->AM) egout = EG2DB(egout+slot->tll) + *(slot->plfo_am) ;
  else egout = EG2DB(egout+slot->tll)  ;

  if(egout >= DB_MUTE) egout = DB_MUTE-1;
  return egout ;

}

/* CARRIOR */
INLINE static e_int32 calc_slot_car(OPLL_SLOT *slot, e_int32 fm)
{
  slot->egout = calc_envelope(slot) ; 
  slot->pgout = calc_phase(slot) ;
  slot->output[1] = slot->output[0] ;

  if(slot->egout>=(DB_MUTE-1))
  {
    slot->output[0] = 0 ;
  }
  else
  {
    slot->output[0] = DB2LIN_TABLE[slot->sintbl[(slot->pgout+wave2_8pi(fm))&(PG_WIDTH-1)] + slot->egout] ;
  }

  return (slot->output[1] + slot->output[0]) >> 1 ;
}

/* MODULATOR */
INLINE static e_int32 calc_slot_mod(OPLL_SLOT *slot)
{
  e_int32 fm ;

  slot->output[1] = slot->output[0] ;
  slot->egout = calc_envelope(slot) ; 
  slot->pgout = calc_phase(slot) ;

  if(slot->egout>=(DB_MUTE-1))
  {
    slot->output[0] = 0 ;
  }
  else if(slot->patch->FB!=0)
  {
    fm = wave2_4pi(slot->feedback) >> (7 - slot->patch->FB) ;
    slot->output[0] = DB2LIN_TABLE[slot->sintbl[(slot->pgout+fm)&(PG_WIDTH-1)] + slot->egout] ;
  }
  else
  {
    slot->output[0] = DB2LIN_TABLE[slot->sintbl[slot->pgout] + slot->egout] ;
  }

  slot->feedback = (slot->output[1] + slot->output[0]) >> 1 ;

  return slot->feedback ;

}

/* TOM */
INLINE static e_int32 calc_slot_tom(OPLL_SLOT *slot)
{

  slot->egout = calc_envelope(slot) ; 
  slot->pgout = calc_phase(slot) ;
  if(slot->egout>=(DB_MUTE-1)) return 0 ;
  
  return DB2LIN_TABLE[slot->sintbl[slot->pgout] + slot->egout] ;

}

/* SNARE */
INLINE static e_int32 calc_slot_snare(OPLL_SLOT *slot, e_uint32 whitenoise)
{
  slot->egout = calc_envelope(slot) ; 
  slot->pgout = calc_phase(slot) ;
  if(slot->egout>=(DB_MUTE-1)) return 0 ;

  if(slot->pgout & (1<<(PG_BITS-1)))
    return (DB2LIN_TABLE[slot->egout] + DB2LIN_TABLE[slot->egout+whitenoise]) >> 1 ;
  else
    return (DB2LIN_TABLE[DB_MUTE + DB_MUTE + slot->egout] + DB2LIN_TABLE[slot->egout+whitenoise]) >> 1;
}

/* TOP-CYM */
INLINE static e_int32 calc_slot_cym(OPLL_SLOT *slot, e_int32 a, e_int32 b)
{
  slot->egout = calc_envelope(slot) ;
  if(slot->egout>=(DB_MUTE-1))
  {
    return 0 ;
  }
  else
  {
    return (DB2LIN_TABLE[slot->egout+a] + DB2LIN_TABLE[slot->egout+b]) >> 1;
  }
}

/* HI-HAT */
INLINE static e_int32 calc_slot_hat(OPLL_SLOT *slot, e_int32 a, e_int32 b, e_uint32 whitenoise)
{
  slot->egout = calc_envelope(slot) ;

  if(slot->egout>=(DB_MUTE-1))
  {
    return 0 ;
  }
  else
  {
    return (DB2LIN_TABLE[slot->egout+whitenoise] + DB2LIN_TABLE[slot->egout+a] + DB2LIN_TABLE[slot->egout+b]) >>2 ;
  }
}

e_int16 OPLL_calc(OPLL *opll)
{
  e_int32 inst = 0 , perc = 0 , out = 0 ;
  e_int32 i ;

  update_ampm(opll) ;
  update_noise(opll) ;

  for(i = 0 ; i < 6 ; i++)
    if(!(opll->mask&OPLL_MASK_CH(i))&&(opll->CAR(i)->eg_mode!=FINISH))
      inst += calc_slot_car(opll->CAR(i),calc_slot_mod(opll->MOD(i))) ;

  if(!opll->rythm_mode)
  {
    for(i = 6 ; i < 9 ; i++)
      if(!(opll->mask&OPLL_MASK_CH(i))&&(opll->CAR(i)->eg_mode!=FINISH))
        inst += calc_slot_car(opll->CAR(i),calc_slot_mod(opll->MOD(i))) ;
  }
  else
  {
    opll->MOD(7)->pgout = calc_phase(opll->MOD(7)) ;
    opll->CAR(8)->pgout = calc_phase(opll->CAR(8)) ;

    if(!(opll->mask&OPLL_MASK_BD)&&(opll->CAR(6)->eg_mode!=FINISH))
      perc += calc_slot_car(opll->CAR(6),calc_slot_mod(opll->MOD(6))) ;

    if(!(opll->mask&OPLL_MASK_HH)&&(opll->MOD(7)->eg_mode!=FINISH))
       perc += calc_slot_hat(opll->MOD(7), opll->noiseA, opll->noiseB, opll->whitenoise) ;

    if(!(opll->mask&OPLL_MASK_SD)&&(opll->CAR(7)->eg_mode!=FINISH))
        perc += calc_slot_snare(opll->CAR(7), opll->whitenoise) ;
    
    if(!(opll->mask&OPLL_MASK_TOM)&&(opll->MOD(8)->eg_mode!=FINISH))
       perc += calc_slot_tom(opll->MOD(8)) ;

    if(!(opll->mask&OPLL_MASK_CYM)&&(opll->CAR(8)->eg_mode!=FINISH))
       perc += calc_slot_cym(opll->CAR(8), opll->noiseA, opll->noiseB) ;
  }

  out = inst + ( perc << 1 ) ; 


  if(out>32767) return 32767 ;
  if(out<-32768) return -32768 ;

  return (e_int16)out ;

}

e_uint32 OPLL_setMask(OPLL *opll, e_uint32 mask)
{
  e_uint32 ret ;
  
  if(opll)
  {
    ret = opll->mask ;
    opll->mask = mask ;
    return ret ;
  }
  else return 0 ;
}

e_uint32 OPLL_toggleMask(OPLL *opll, e_uint32 mask)
{
  e_uint32 ret ;
  
  if(opll)
  {
    ret = opll->mask ;
    opll->mask ^= mask ;
    return ret ;
  }
  else return 0 ;
}

/****************************************************

                       I/O Ctrl

*****************************************************/

void OPLL_writeReg(OPLL *opll, e_uint32 reg, e_uint32 data){

  e_int32 i,v,ch ;

  data = data&0xff ;
  reg = reg&0x3f ;

  switch(reg)
  {
    case 0x00:
      opll->patch[0]->AM = (data>>7)&1 ;
      opll->patch[0]->PM = (data>>6)&1 ;
      opll->patch[0]->EG = (data>>5)&1 ;
      opll->patch[0]->KR = (data>>4)&1 ;
      opll->patch[0]->ML = (data)&15 ;
      for(i=0;i<9;i++)
      {
        if(opll->ch[i]->patch_number==0)
        {
          UPDATE_PG(opll->MOD(i)) ;
          UPDATE_RKS(opll->MOD(i)) ;
          UPDATE_EG(opll->MOD(i)) ;
        }
      }
      break ;

    case 0x01:
      opll->patch[1]->AM = (data>>7)&1 ;
      opll->patch[1]->PM = (data>>6)&1 ;
      opll->patch[1]->EG = (data>>5)&1 ;
      opll->patch[1]->KR = (data>>4)&1 ;
      opll->patch[1]->ML = (data)&15 ;
      for(i=0;i<9;i++)
      {
        if(opll->ch[i]->patch_number==0)
        {
          UPDATE_PG(opll->CAR(i)) ;
          UPDATE_RKS(opll->CAR(i)) ;
          UPDATE_EG(opll->CAR(i)) ;
        }
      }
      break;

    case 0x02:
      opll->patch[0]->KL = (data>>6)&3 ;
      opll->patch[0]->TL = (data)&63 ;
      for(i=0;i<9;i++)
      {
        if(opll->ch[i]->patch_number==0)
        {
          UPDATE_TLL(opll->MOD(i)) ;
        }
      }
      break ;

    case 0x03:
      opll->patch[1]->KL = (data>>6)&3 ;
      opll->patch[1]->WF = (data>>4)&1 ;
      opll->patch[0]->WF = (data>>3)&1 ;
      opll->patch[0]->FB = (data)&7 ;
      for(i=0;i<9;i++)
      {
        if(opll->ch[i]->patch_number==0)
        {
          UPDATE_WF(opll->MOD(i)) ;
          UPDATE_WF(opll->CAR(i)) ;
        }
      }
      break ;

    case 0x04:
      opll->patch[0]->AR = (data>>4)&15 ;
      opll->patch[0]->DR = (data)&15 ;
      for(i=0;i<9;i++)
      {
        if(opll->ch[i]->patch_number==0)
        {
          UPDATE_EG(opll->MOD(i)) ;
        }
      }
      break ;

    case 0x05:
      opll->patch[1]->AR = (data>>4)&15 ;
      opll->patch[1]->DR = (data)&15 ;
      for(i=0;i<9;i++)
      {
        if(opll->ch[i]->patch_number==0)
        {
          UPDATE_EG(opll->CAR(i)) ;
        }
      }
      break ;

    case 0x06:
      opll->patch[0]->SL = (data>>4)&15 ;
      opll->patch[0]->RR = (data)&15 ;
      for(i=0;i<9;i++)
      {
        if(opll->ch[i]->patch_number==0)
        {
          UPDATE_EG(opll->MOD(i)) ;
        }
      }
      break ;

    case 0x07:
      opll->patch[1]->SL = (data>>4)&15 ;
      opll->patch[1]->RR = (data)&15 ;
      for(i=0;i<9;i++)
      {
        if(opll->ch[i]->patch_number==0)
        {
          UPDATE_EG(opll->CAR(i)) ;
        }
      }
      break ;

    case 0x0e:

      if(opll->rythm_mode)
      {
        opll->slot_on_flag[SLOT_BD1] = (opll->reg[0x0e]&0x10) | (opll->reg[0x26]&0x10) ;
        opll->slot_on_flag[SLOT_BD2] = (opll->reg[0x0e]&0x10) | (opll->reg[0x26]&0x10) ;
        opll->slot_on_flag[SLOT_SD]  = (opll->reg[0x0e]&0x08) | (opll->reg[0x27]&0x10) ;
        opll->slot_on_flag[SLOT_HH]  = (opll->reg[0x0e]&0x01) | (opll->reg[0x27]&0x10) ;
        opll->slot_on_flag[SLOT_TOM] = (opll->reg[0x0e]&0x04) | (opll->reg[0x28]&0x10) ;
        opll->slot_on_flag[SLOT_CYM] = (opll->reg[0x0e]&0x02) | (opll->reg[0x28]&0x10) ;
      }
      else
      {
        opll->slot_on_flag[SLOT_BD1] = (opll->reg[0x26]&0x10) ;
        opll->slot_on_flag[SLOT_BD2] = (opll->reg[0x26]&0x10) ;
        opll->slot_on_flag[SLOT_SD]  = (opll->reg[0x27]&0x10) ;
        opll->slot_on_flag[SLOT_HH]  = (opll->reg[0x27]&0x10) ;
        opll->slot_on_flag[SLOT_TOM] = (opll->reg[0x28]&0x10) ;
        opll->slot_on_flag[SLOT_CYM] = (opll->reg[0x28]&0x10) ;
      }

      if(((data>>5)&1)^(opll->rythm_mode))
      {
        setRythmMode(opll,data) ;
      }

      if(opll->rythm_mode)
      {
        if(data&0x10) keyOn_BD(opll) ; else keyOff_BD(opll) ;
        if(data&0x8) keyOn_SD(opll) ; else keyOff_SD(opll) ;
        if(data&0x4) keyOn_TOM(opll) ; else keyOff_TOM(opll) ;
        if(data&0x2) keyOn_CYM(opll) ; else keyOff_CYM(opll) ;
        if(data&0x1) keyOn_HH(opll) ; else keyOff_HH(opll) ;
      }
      
      UPDATE_ALL(opll->MOD(6)) ;
      UPDATE_ALL(opll->CAR(6)) ;
      UPDATE_ALL(opll->MOD(7)) ;  
      UPDATE_ALL(opll->CAR(7)) ;
      UPDATE_ALL(opll->MOD(8)) ;
      UPDATE_ALL(opll->CAR(8)) ;        
      break ;

    case 0x0f:
      break ;

    case 0x10:  case 0x11:  case 0x12:  case 0x13:
    case 0x14:  case 0x15:  case 0x16:  case 0x17:
    case 0x18:
      ch = reg-0x10 ;
      setFnumber(opll, ch, data + ((opll->reg[0x20+ch]&1)<<8)) ;
      UPDATE_ALL(opll->MOD(ch));
      UPDATE_ALL(opll->CAR(ch));
      switch(reg)
      {
      case 0x17:
        opll->noiseA_dphase = dphaseNoiseTable[data + ((opll->reg[0x27]&1)<<8)][(opll->reg[0x27]>>1)&7] ;
        break ;
      case 0x18:
        opll->noiseB_dphase = dphaseNoiseTable[data + ((opll->reg[0x28]&1)<<8)][(opll->reg[0x28]>>1)&7] ;
        break;
      default:
        break ;
      }
      break ;

    case 0x20:  case 0x21:  case 0x22:  case 0x23:
    case 0x24:  case 0x25:  case 0x26:  case 0x27:
    case 0x28:

      ch = reg - 0x20 ;
      setFnumber(opll, ch, ((data&1)<<8) + opll->reg[0x10+ch]) ;
      setBlock(opll, ch, (data>>1)&7 ) ;
      opll->slot_on_flag[ch*2] = opll->slot_on_flag[ch*2+1] = (opll->reg[reg])&0x10 ;
      
      switch(reg)
      {
      case 0x26:
        if(opll->rythm_mode)
        {
          opll->slot_on_flag[SLOT_BD1] |= (opll->reg[0x0e])&0x10 ;
          opll->slot_on_flag[SLOT_BD2] |= (opll->reg[0x0e])&0x10 ;
        }
        break ;
      
      case 0x27:
        opll->noiseA_dphase = dphaseNoiseTable[((data&1)<<8) + opll->reg[0x17]][(data>>1)&7] ;
        if(opll->rythm_mode)
        {
          opll->slot_on_flag[SLOT_SD]  |= (opll->reg[0x0e])&0x08 ;
          opll->slot_on_flag[SLOT_HH]  |= (opll->reg[0x0e])&0x01 ;
        }
        break;

      case 0x28:
        opll->noiseB_dphase = dphaseNoiseTable[((data&1)<<8) + opll->reg[0x18]][(data>>1)&7] ;
        if(opll->rythm_mode)
        {
          opll->slot_on_flag[SLOT_TOM] |= (opll->reg[0x0e])&0x04 ;
          opll->slot_on_flag[SLOT_CYM] |= (opll->reg[0x0e])&0x02 ;
        }
        break ;

      default:
        break ;
      }
      
      if((opll->reg[reg]^data)&0x20) setSustine(opll, ch, (data>>5)&1) ;
      if(data&0x10) keyOn(opll, ch) ; else keyOff(opll, ch) ;
      UPDATE_ALL(opll->MOD(ch)) ;
      UPDATE_ALL(opll->CAR(ch)) ;
      break ;

    case 0x30: case 0x31: case 0x32: case 0x33: case 0x34:
    case 0x35: case 0x36: case 0x37: case 0x38:
      i = (data>>4)&15 ;
      v = data&15 ;
      if((opll->rythm_mode)&&(reg>=0x36))
      {
        switch(reg)
        {
         case 0x37 :
            setSlotVolume(opll->MOD(7), i<<2) ;
            break ;
         case 0x38 :
           setSlotVolume(opll->MOD(8), i<<2) ;
           break ;
        }
      }
      else
      { 
        setPatch(opll, reg-0x30, i) ;
      }
    
      setVolume(opll, reg-0x30, v<<2) ;
      UPDATE_ALL(opll->MOD(reg-0x30)) ;
      UPDATE_ALL(opll->CAR(reg-0x30)) ;
      break ;

    default:
      break ;

  }

  opll->reg[reg] = (e_uint8)data ;

}

void OPLL_writeIO(OPLL *opll, e_uint32 adr, e_uint32 val)
{
  if(adr & 1) OPLL_writeReg(opll, opll->adr, val) ;
  else opll->adr = val ;
}
