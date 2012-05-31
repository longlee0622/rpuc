#include "cl2opt.h"

void parse_context(STRUCT_CONTEXT *cp, reg32 c[108])
{
	int i,j,k;
	for (i=0;i<100;i++)
	{
		if (i<64)
		{
			cp->in_config_9A[i] = c[i] & 0x1FF;
			cp->in_config_9B[i] = (c[i]>>9) & 0x1FF;
			cp->in_config_9temp[i] = (c[i]>>18) & 0x1FF;
			cp->opcode[i] = (c[i]>>27) & 0x1F;
		}
		else if (i<96)
		{
			int j = i-64;
			cp->out_config_8[j*2] = c[i] & 0xFF;
			cp->out_config_8temp[j*2] = (c[i]>>8) & 0xFF;
			cp->out_config_8[j*2 + 1] = (c[i]>>16) & 0xFF;
			cp->out_config_8temp[j*2 + 1] = (c[i]>>24) & 0xFF;
		}
		else if (i<98)
		{
			j = i - 96;
			for (k=0;k<32;k++)
			{
				cp->out_or_not[j*32 + k] = (c[i]>>k) & 0x1;	
			}
		} else
		{
			j = i - 98;
			for (k=0;k<32;k++)
			{
				cp->out_or_not_temp[j*32 + k] = (c[i]>>k) & 0x1;
			}
		}
	}

	cp->inA_bit16_8 = ( ((c[100]>>28) & 0x1) == 1);
	cp->inB_bit16_8 = ( ((c[100]>>29) & 0x1) == 1);
	cp->inT_bit16_8 = ( ((c[100]>>30) & 0x1) == 1);
	cp->out_bit16_8 = ( ((c[100]>>17) & 0x1) == 1);
	cp->datapath_bit16_8 = ( ((c[100]>>18) & 0x1) == 1);
	cp->cons_bit16_8 = ( ((c[100]>>19) & 0x1) == 1);
	cp->in_bit4_special = ( ((c[100]>>31) & 0x1) == 1);
	cp->flag_extension_if_force0 = ( ((c[100]>>20) & 0x1) == 1);

	//if ((true==cp->in_bit16_8) && (false==cp->datapath_bit16_8) && (true==cp->out_bit16_8)) //3'b101
	//{
	//	cp->datapath_bit16_8 = true; //输入4比特，16比特运算，16比特输出
	//	cp->in_bit4_special = true;
	//}
	//else
	//{
	//	cp->in_bit4_special = false;
	//}

	cp->input_cnt = c[100] & 0x3 + (((c[100]>>27)&0x1)<<2);
	cp->output_cnt = (c[100]>>2) & 0x7;
	cp->output_begin = (c[100]>>21) & 0xF;

	//cp->calc_cnt = (c[100]>>5) & 0xF;
	cp->calc_cnt = cp->output_begin+cp->output_cnt+1;//计算次数=计算开始+输出次数+1

	cp->loop_cnt = (c[100]>>9) & 0xF;//only for debug.
	cp->pipeline_gap = (c[100]>>13) & 0xF;

	cp->c100_j = (c[100]>>25) & 0x1;
	cp->c100_k = (c[100]>>26) & 0x1;
	cp->c_reg0_addr = (c[107]>>0) & 0x3F;
	cp->c_reg1_addr = (c[107]>>6) & 0x3F;

	for (i=0;i<32;i++)//输入配置信息的第10bit
	{
		cp->in_config_9A[i] = (((c[101]>>i)&0x1)<<7)+((cp->in_config_9A[i])&0x7F)+(((cp->in_config_9A[i]>>7)&0x3)<<8);
		cp->in_config_9A[32+i] = (((c[102]>>i)&0x1)<<7)+((cp->in_config_9A[32+i])&0x7F)+(((cp->in_config_9A[32+i]>>7)&0x3)<<8);
		cp->in_config_9B[i] = (((c[103]>>i)&0x1)<<7)+((cp->in_config_9B[i])&0x7F)+(((cp->in_config_9B[i]>>7)&0x3)<<8);
		cp->in_config_9B[32+i] = (((c[104]>>i)&0x1)<<7)+((cp->in_config_9B[32+i])&0x7F)+(((cp->in_config_9B[32+i]>>7)&0x3)<<8);
		cp->in_config_9temp[i] = (((c[105]>>i)&0x1)<<7)+((cp->in_config_9temp[i])&0x7F)+(((cp->in_config_9temp[i]>>7)&0x3)<<8);
		cp->in_config_9temp[32+i] = (((c[106]>>i)&0x1)<<7)+((cp->in_config_9temp[32+i])&0x7F)+(((cp->in_config_9temp[32+i]>>7)&0x3)<<8);
	}
}


void parse_context_node(CONTEXT_NODE ctx_no[64],STRUCT_CONTEXT *cp) //解析到ctx node
{
	int i;
	for (i=0;i<64;i++)
	{
		int a=cp->in_config_9A[i];
		int a1=(a>>8)&0x3;
		ctx_no[i].A_in_cnt=/*(a1==2)?1:*/((a1==0)?(((a>>5)&0x7)+1):(0));//0: in cnt + 1; else:don't care
		ctx_no[i].A_con_src=(a1==1)?(a&0xF):16;  //1:connect;else: don't care

		a=cp->in_config_9B[i];
		a1=(a>>8)&0x3;
		//ctx_no[i].B_in_cnt=(a1==2)?1:((a1==0)?(((a>>5)&0x7)+1):(0));
		ctx_no[i].B_in_cnt=(a1==0)?(((a>>5)&0x7)+1):(0);
		ctx_no[i].B_con_src=(a1==1)?(a&0xF):16;

		a=cp->in_config_9temp[i];
		a1=(a>>8)&0x3;
		ctx_no[i].T_in_cnt=/*(a1==2)?1:*/((a1==0)?(((a>>5)&0x7)+1):(0));
		ctx_no[i].T_con_src=(a1==1)?(a&0xF):16;

		a=cp->out_config_8[i];
		a1=cp->out_or_not[i];
		ctx_no[i].R_out_cnt=a1?(((a>>5)&0x7)+1):0; //out cnt + 1

		a=cp->out_config_8temp[i];
		a1=cp->out_or_not_temp[i];
		ctx_no[i].T_out_cnt=a1?(((a>>5)&0x7)+1):0;

		ctx_no[i].R_opcode=cp->opcode[i];
	}
	return;
}


int path_value_max(CONTEXT_NODE ctx_no[64],int index,int reg_or_not) //计算max(i+1+c)
{
	if(reg_or_not) //1, reg; 0, temp;
	{
		int sum1=0;
		int sum2=0;
		int sum3=0;
		//ctx_no[i].A_in_cnt=(a1==0)?(((a>>5)&0x7)+1):(0);//0: in cnt + 1; else:don't care
		//ctx_no[i].A_con_src=(a1==1)?(a&0xF):16;  //1:connect;else: don't care
		if (ctx_no[index].R_opcode!=25)  //A is required; if 25, then not required!
		{
			if(ctx_no[index].A_in_cnt!=0) //a from inputfifo
			{
				sum1=ctx_no[index].A_in_cnt;
				ctx_no[index].A_used_flag=1;//this node is used for input
			} 
			else if(ctx_no[index].A_con_src!=16) //a from prevous line
			{
				sum1=path_value_max(ctx_no
					,( ((int)(index/8)==0)?(64-8):(((int)(index/8)-1)*8)  ) + (ctx_no[index].A_con_src&0x7)
					,((((ctx_no[index].A_con_src)>>3)&0x1)==0)?1:0);
			} 
			//else  //a from constant or self
		} //else //A is not required!

		if (ctx_no[index].R_opcode!=5)  //B is required; if 5, then not required!
		{
			if(ctx_no[index].B_in_cnt!=0) //b from inputfifo
			{
				sum2=ctx_no[index].B_in_cnt;
				ctx_no[index].B_used_flag=1;//this node is used for input
			}
			else if(ctx_no[index].B_con_src!=16) //b from prevous line
			{
				sum2=path_value_max(ctx_no
					,( ((int)(index/8)==0)?(64-8):(((int)(index/8)-1)*8)  ) + (ctx_no[index].B_con_src&0x7)
					,((((ctx_no[index].B_con_src)>>3)&0x1)==0)?1:0);

			} 
			//else //b from constant or self
		} //else //B is not required!

		if (ctx_no[index].R_opcode==16) //comp?A:B
		{
			sum3=path_value_max(ctx_no
				,( ((int)(index/8)==0)?(64+index-8):(index-8)  )
				,1); //previous reg
		}
		else if ((ctx_no[index].R_opcode==22)||(ctx_no[index].R_opcode==24)) //Temp
		{
			sum3=path_value_max(ctx_no
				,index
				,0); //current temp
		}
		if(sum1<sum2)
			sum1=sum2;
		if(sum1<sum3)
			sum1=sum3;
		return (sum1+1); //get the bigger one.

	}
	else
	{
		if (ctx_no[index].T_in_cnt!=0)// T from inputfifo
		{
			ctx_no[index].T_used_flag=1;//this node is used for input
			return (ctx_no[index].T_in_cnt); //zhumin 2010-11-20
		}
		else if ((ctx_no[index].T_con_src!=16))//T from prevous line
		{
			return ((((ctx_no[index].T_con_src)>>3)&0x1)==0)?(path_value_max(ctx_no
				,( ((int)(index/8)==0)?(64-8):(((int)(index/8)-1)*8)  ) + (ctx_no[index].T_con_src&0x7)
				,1 )):(1+path_value_max(ctx_no
				,( ((int)(index/8)==0)?(64-8):(((int)(index/8)-1)*8)  ) + (ctx_no[index].T_con_src&0x7)
				,0 ));
		}
		//else //T from constant or self
		return 0;		
	}	
}

int path_value_min(CONTEXT_NODE ctx_no[64],int index,int reg_or_not) //计算min(i+1+c)
{
	if(reg_or_not) //1, reg; 0, temp;
	{
		int sum1=100;
		int sum2=100;
		int sum3=100;
		if (ctx_no[index].R_opcode!=25)  //A is required; if 25, then not required!
		{
			if(ctx_no[index].A_in_cnt!=0) //a from inputfifo
			{
				sum1=ctx_no[index].A_in_cnt;
				ctx_no[index].A_used_flag=1;//this node is used for input
			} 
			else if(ctx_no[index].A_con_src!=16) //a from prevous line
			{
				sum1=path_value_min(ctx_no
					,( ((int)(index/8)==0)?(64-8):(((int)(index/8)-1)*8)  ) + (ctx_no[index].A_con_src&0x7)
					,((((ctx_no[index].A_con_src)>>3)&0x1)==0)?1:0);
			} 
			//else  //a from constant or self
		}//else A is not required!

		if (ctx_no[index].R_opcode!=5)  //B is required; if 5, then not required!
		{
			if(ctx_no[index].B_in_cnt!=0) //b from inputfifo
			{
				sum2=ctx_no[index].B_in_cnt;
				ctx_no[index].B_used_flag=1;//this node is used for input
			}
			else if(ctx_no[index].B_con_src!=16) //b from prevous line
			{
				sum2=path_value_min(ctx_no
					,( ((int)(index/8)==0)?(64-8):(((int)(index/8)-1)*8)  ) + (ctx_no[index].B_con_src&0x7)
					,((((ctx_no[index].B_con_src)>>3)&0x1)==0)?1:0);

			} 
			//else //b from constant or self
		} //else //B is not required!
		if (ctx_no[index].R_opcode==16) //comp?A:B
		{
			sum3=path_value_min(ctx_no
				,(((int)(index/8)==0)?(64+index-8):(index-8)  )
				,1); //pervious reg
		}
		else if ((ctx_no[index].R_opcode==22)||(ctx_no[index].R_opcode==24)) //Temp
		{
			sum3=path_value_min(ctx_no
				,index
				,0); //current temp
		}
		if(sum1>sum2)
			sum1=sum2;
		if(sum1>sum3)
			sum1=sum3;
		return (sum1+1); //get the smaller one.


	}  // temp
	else
	{
		if (ctx_no[index].T_in_cnt!=0)// T from inputfifo
		{
			ctx_no[index].T_used_flag=1;//this node is used for input
			return (ctx_no[index].T_in_cnt); //zhumin 2010-11-20
		}
		else if ((ctx_no[index].T_con_src!=16))//T from prevous line
		{
			return ((((ctx_no[index].T_con_src)>>3)&0x1)==0)?(path_value_min(ctx_no
				,( ((int)(index/8)==0)?(64-8):(((int)(index/8)-1)*8)  ) + (ctx_no[index].T_con_src&0x7)
				,1 )):(1+path_value_min(ctx_no
				,( ((int)(index/8)==0)?(64-8):(((int)(index/8)-1)*8)  ) + (ctx_no[index].T_con_src&0x7)
				,0 ));
		}
		//else //T from constant or self
		return 100;

	}	

}

reg32 optimize_c100(CONTEXT_NODE ctx_no[64],reg32 c[108])
{
	STRUCT_CONTEXT *p_cc, core_ctx;
	int I,O,W,G;
	int i;
	int maxv,minv;
	int tmp;
	p_cc=&core_ctx;
	parse_context(p_cc, c);
	parse_context_node(ctx_no,p_cc);	

	for (i=0;i<64;i++)
	{
		ctx_no[i].A_used_flag=0;
		ctx_no[i].B_used_flag=0;
		ctx_no[i].T_used_flag=0;
	}

	maxv=0;minv=100;

	for (i=0;i<64;i++)
	{
		if(ctx_no[i].R_out_cnt!=0) //for output
		{
			tmp = path_value_max(ctx_no,i,1)-ctx_no[i].R_out_cnt;
			if (maxv<tmp)
			{
				maxv=tmp;
			}
			tmp=path_value_min(ctx_no,i,1)-ctx_no[i].R_out_cnt;
			if (minv>tmp)
			{
				minv=tmp;
			}
		}
		if(ctx_no[i].T_out_cnt!=0) //for temp output
		{
			tmp=path_value_max(ctx_no,i,0)-ctx_no[i].T_out_cnt;
			if (maxv<tmp)
			{
				maxv=tmp;
			}
			tmp=path_value_min(ctx_no,i,0)-ctx_no[i].T_out_cnt;
			if (minv>tmp)
			{
				minv=tmp;
			}
		}
	}


	I=O=0;
	for (i=0;i<64;i++)
	{
		if ((ctx_no[i].A_in_cnt>I)&&(ctx_no[i].A_used_flag==1))
			I=ctx_no[i].A_in_cnt;
		if ((ctx_no[i].B_in_cnt>I)&&(ctx_no[i].B_used_flag==1))
			I=ctx_no[i].B_in_cnt;
		if ((ctx_no[i].T_in_cnt>I)&&(ctx_no[i].T_used_flag==1))
			I=ctx_no[i].T_in_cnt;

		if (ctx_no[i].R_out_cnt>O)
			O=ctx_no[i].R_out_cnt;
		if (ctx_no[i].T_out_cnt>O)
			O=ctx_no[i].T_out_cnt;		
	}

	I=I-1;
	O=O-1;


	W=maxv-1-I;//max(i+c-o)-1-I
	G=maxv-minv-I;//max(i+c-o)-min(i+c-o)-I

	//saturate of W,G
	if (W<0) {G=G-W;W=0;}
	if (G<0) G=0;
	//

	c[100]=(c[100]&0xF7FFFFFC)+ (I&0x3) + (((I>>2)&0x1)<<27); //input
	c[100]=(c[100]&0xFFFFFFE3)+ ((O&0x7)<<2);   //output
	c[100]=(c[100]&0xFE1FFFFF)+ ((W&0xF)<<21);  //wait
	c[100]=(c[100]&0xFFFE1FFF)+ ((G&0xF)<<13);  //gap

	return c[100];
}





