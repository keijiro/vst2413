#ifndef _EMU2413_H_
#define _EMU2413_H_

#ifdef EMU2413_DLL_EXPORTS
  #define EMU2413_API __declspec(dllexport)
#elif  EMU2413_DLL_IMPORTS
  #define EMU2413_API __declspec(dllimport)
#else
  #define EMU2413_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PI 3.14159265358979

typedef unsigned int e_uint;
typedef signed int e_int;

typedef unsigned char e_uint8 ;
typedef signed char e_int8 ;

typedef unsigned short e_uint16 ;
typedef signed short e_int16 ;

typedef unsigned int e_uint32 ;
typedef signed int e_int32 ;

enum {OPLL_2413_TONE=0, OPLL_VRC7_TONE=1} ;

/* voice data */
typedef struct {
  e_uint32 TL,FB,EG,ML,AR,DR,SL,RR,KR,KL,AM,PM,WF ;
} OPLL_PATCH ;

/* slot */
typedef struct {

  OPLL_PATCH *patch;  

  e_int32 type ;          /* 0 : modulator 1 : carrier */

  /* OUTPUT */
  e_int32 feedback ;
  e_int32 output[5] ;      /* Output value of slot */

  /* for Phase Generator (PG) */
  e_uint16 *sintbl ;    /* Wavetable */
  e_uint32 phase ;      /* Phase */
  e_uint32 dphase ;     /* Phase increment amount */
  e_uint32 pgout ;      /* output */

  /* for Envelope Generator (EG) */
  e_int32 fnum ;          /* F-Number */
  e_int32 block ;         /* Block */
  e_int32 volume ;        /* Current volume */
  e_int32 sustine ;       /* Sustine 1 = ON, 0 = OFF */
  e_uint32 tll ;	      /* Total Level + Key scale level*/
  e_uint32 rks ;        /* Key scale offset (Rks) */
  e_int32 eg_mode ;       /* Current state */
  e_uint32 eg_phase ;   /* Phase */
  e_uint32 eg_dphase ;  /* Phase increment amount */
  e_uint32 egout ;      /* output */


  /* refer to opll-> */
  e_int32 *plfo_pm ;
  e_int32 *plfo_am ;


} OPLL_SLOT ;

/* Channel */
typedef struct {

  e_int32 patch_number ;
  e_int32 key_status ;
  OPLL_SLOT *mod, *car ;

} OPLL_CH ;

/* Mask */
#define OPLL_MASK_CH(x) (1<<(x))
#define OPLL_MASK_HH (1<<(9))
#define OPLL_MASK_CYM (1<<(10))
#define OPLL_MASK_TOM (1<<(11))
#define OPLL_MASK_SD (1<<(12))
#define OPLL_MASK_BD (1<<(13))
#define OPLL_MASK_RYTHM ( OPLL_MASK_HH | OPLL_MASK_CYM | OPLL_MASK_TOM | OPLL_MASK_SD | OPLL_MASK_BD )

/* opll */
typedef struct {

  e_uint32 adr ;

  e_int32 output[2] ;

  /* Register */
  e_uint8 reg[0x40] ; 
  e_int32 slot_on_flag[18] ;

  /* Rythm Mode : 0 = OFF, 1 = ON */
  e_int32 rythm_mode ;

  /* Pitch Modulator */
  e_uint32 pm_phase ;
  e_int32 lfo_pm ;

  /* Amp Modulator */
  e_int32 am_phase ;
  e_int32 lfo_am ;


  /* Noise Generator */
  e_uint32 noise_seed ;
  e_uint32 whitenoise ;
  e_uint32 noiseA ;
  e_uint32 noiseB ;
  e_uint32 noiseA_phase ;
  e_uint32 noiseB_phase ;
  e_uint32 noiseA_dphase ;
  e_uint32 noiseB_dphase ;

  /* Channel & Slot */
  OPLL_CH *ch[9] ;
  OPLL_SLOT *slot[18] ;

  /* Voice Data */
  OPLL_PATCH *patch[19*2] ;
  e_int32 patch_update[2] ; /* flag for check patch update */

  e_uint32 mask ;

} OPLL ;

/* Initialize */
EMU2413_API void OPLL_init(e_uint32 clk, e_uint32 rate) ;
EMU2413_API void OPLL_close(void) ;

/* Create Object */
EMU2413_API OPLL *OPLL_new(void) ;
EMU2413_API void OPLL_delete(OPLL *) ;

/* Setup */
EMU2413_API void OPLL_reset(OPLL *) ;
EMU2413_API void OPLL_reset_patch(OPLL *, e_int32) ;
EMU2413_API void OPLL_setClock(e_uint32 c, e_uint32 r) ;

/* Port/Register access */
EMU2413_API void OPLL_writeIO(OPLL *, e_uint32 reg, e_uint32 val) ;
EMU2413_API void OPLL_writeReg(OPLL *, e_uint32 reg, e_uint32 val) ;

/* Synthsize */
EMU2413_API e_int16 OPLL_calc(OPLL *) ;

/* Misc */
EMU2413_API void OPLL_setPatch(OPLL *, const e_uint8 *dump) ;

EMU2413_API void OPLL_copyPatch(OPLL *, e_int32, OPLL_PATCH *) ;
EMU2413_API void OPLL_forceRefresh(OPLL *) ;

/* Utility */
EMU2413_API void OPLL_dump2patch(const e_uint8 *dump, OPLL_PATCH *patch) ;
EMU2413_API void OPLL_patch2dump(const OPLL_PATCH *patch, e_uint8 *dump) ;
EMU2413_API void OPLL_getDefaultPatch(e_int32 type, e_int32 num, OPLL_PATCH *) ;

/* Channel Mask */
EMU2413_API e_uint32 OPLL_setMask(OPLL *, e_uint32 mask) ;
EMU2413_API e_uint32 OPLL_toggleMask(OPLL *, e_uint32 mask) ;

#define dump2patch OPLL_dump2patch

#ifdef __cplusplus
}
#endif

#endif
