#ifndef _CL2OPT_H
#define _CL2OPT_H

#include "reg.h"

typedef struct context_word{
	int inA_bit16_8;
	int inB_bit16_8;
	int inT_bit16_8;
	int out_bit16_8;
	int datapath_bit16_8;
	int in_bit4_special;
	int cons_bit16_8;
	int flag_extension_if_force0;

	int input_cnt;
	int output_cnt;
	int calc_cnt;
	int loop_cnt;
	int pipeline_gap;
	int output_begin;

	int in_config_9A[64];//现在统统变成10bit
	int in_config_9B[64];  //contain config info for constant regs
	int c_reg0_addr, c_reg1_addr; //contained in c101
	int in_config_9temp[64];
	int opcode[64];
	int out_config_8[64];
	int out_config_8temp[64];
	int out_or_not[64];
	int out_or_not_temp[64];

	int c100_j,c100_k;

} STRUCT_CONTEXT;

typedef struct context_node{
	int A_in_cnt,B_in_cnt,T_in_cnt; //if 0, disable; else cnt no.
	int A_con_src,B_con_src,T_con_src; //if 16, disable; else src no.
	int R_out_cnt,T_out_cnt;		//if 0, disable; else cnt no.
	int R_opcode;
	int A_used_flag,B_used_flag,T_used_flag; //if 0, not used;1 is used for input.
} CONTEXT_NODE;

reg32 optimize_c100(CONTEXT_NODE ctx_no[64],reg32 c[108]);

void parse_context(STRUCT_CONTEXT *cp, reg32 c[108]);

void parse_context_node(CONTEXT_NODE ctx_no[64],STRUCT_CONTEXT *cp);

int path_value_max(CONTEXT_NODE ctx_no[64],int index,int reg_or_not);

int path_value_min(CONTEXT_NODE ctx_no[64],int index,int reg_or_not);


#endif


