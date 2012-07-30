/************************************************
 *			File name: platf.h
 *-----------------------------------------------
 * Description: Define hardware platform parameters;
 *
 * Author: L. Xie @ MicroE. SJTU
 * Date : 2009/11/13
 *************************************************
 */

#ifndef HARDWARE_H
#define HARDWARE_H

//#define AHB_WIDTH               32
//#define AHB_LOAD_WIDTH          16
//#define AHB_STORE_WIDTH         16

// RCA parameters
#define RCA_ROW                 8
#define RCA_COL                 8

#define RC_REG_NUM              64
#define RC_TEMP_NUM             64

#define IN_OP_REG_SUM           64
#define OUT_REG_SUM             32
#define OUT_EN_SUM              4
#define IN_HBIT_SUM             6

#define CL2_CONTEXT_LEN         107

#define FIFO_WIDTH				32 // Byte
#define RIM_WIDTH				32 // Byte

#define FIFO_WIDTH_DATA         16
#define RIM_WIDTH_DATA          16

#define SSRAM_LOAD_BASE			0
#define SSRAM_STORE_BASE		0x300

//2011.5.18  liuxie
#define SSRAM_LOAD_TEMP_BASE    0x100



#define INST_SUM                0x20

// Opecode definition
enum {
    ADD,  /* A + B */ 
    SUB,  /* A - B */
	LSR,  /* A >> B */
	LSL,  /* A << B */
	LSRR, /* A >> B(with rounding) */ 
	BYP,  /* A  */
	AND,  /* A & B */
	ORR,  /* A | B */ 
	XOR,  /* A ^ B */
	NOR,  /* ~(A ^ B) */
	ABS,  /* |A-B| */
	GNT,  /* A > B */
	EQU,  /* A == B */
	GTE,  /* A >= B */
    LTR,  /* (A > B)? B : ( (A < 0)? 0 : A ) */
    ALT1, /* (A > B-1)? B : ( (A < -B)? 0 : A ) */
    SLT,  /* (Result of front RC)? A : B */
    LMT,  /* (A * B) & 0xFFFF */
    HMT,  /* ( (A * B) >> 8 ) & 0xFFFF */ 
    RSB,  /* B - A */
    RGNT, /* B > A */
    RGTE, /* B >= A */
    ALT2, /* (A > B)? B : ( (A < -B)? 0 : A ) */
    MLSR, /* (A x B) >> C */
	
	//change by liuxie 2010.11.23 
	NEQU, /* (A != B) */
    
    #if 0
	ADD, /* A + B */ SUB, /* A - B */
	AND, /* A & B */ ORR, /* A | B */
	XOR, /* A ^ B */ NOR, /* ~(A ^ B) */
	NOT, /* ~A */ LSL, /* A << (B & 0xF) */
	LSR, /* A >> (B & 0xF) */ ASR,/* A + B>>C */
	BYP, /* A */ ASL, /* A + B<<C  */ 
	SSL, /* A - B<<C */ ABS, /* |A-B| */
	SRS, /* (A>>C)-B */  
	GNT, /* A > B */ EQU, /* A == B */ 
	LES, /* A < B */ GTE, /* A >= B */
	LSE, /* A <= B */ NEQ, /* A != B */
	SSR, /* A- (B>>C) */ SLS, /* (A<<C) - B */ 
	ADR, /* (A + B) >> C */ ADL, /* (A + B) << C */ 
	SBR, /* (A - B) >> C */ SBL, /* (A - B) << C */
	
	LTR, /* (A > B)? B : ( (A < 0)? 0 : A ) */
	ALT, /* (A > B)? B : ( (A < -B)? 0 : A ) */
	SLT /* (Result of front RC)? A : B */
	#endif
};

#endif
