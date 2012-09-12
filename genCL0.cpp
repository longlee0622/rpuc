
#include "cl0reg.h"
#include "rpucfg.h"

extern bool IsInnerPort(DFGPort * port);
extern bool IsTempExternPort (DFGPort *port);

int RPUConfig::genCL0Context( CL1Config & cl1config, Vector<RCA *> &CL1RCATemp,Vector<RCA *> &recordPseudoRCA )
{

	Vector<reg32> CL0ContextTemp;//先暂存配置字，REDL,REDS的配置信息，最后再赋值给CL0Context；因为前面要加入“包头”
	Vector<reg32> C_CL0ContextTemp;//Cmodel版本
	// Set Head
	CL0HeadReg headReg;
	
    /************************  for CL0 choose the right CL1  (start) ********************/
	int TempGCGMAddr = GCGMAddrRecord;   //代表CL1 context 的选址
	
	headReg.reset();
	
	headReg.setGroupAdrress( TempGCGMAddr );
	
	//TempGCGMAddr += cl1config.totalLength(); //这里做了选择CL1首地址的地址变换

	GCGMAddrRecord += cl1config.totalLength();
	
	headReg.setSynMode(SYN_MODE_0);      //设置同步模式

	headReg.setConstSelect(0);		//2012.5.7 longlee 设置const1的偏移地址有效
	headReg.setConstOffset1(0);	//2012.5.7 longlee 设置const1的偏移地址为0
	
	CL0ContextTemp.push_back(headReg.reg());
	C_CL0ContextTemp.push_back(headReg.reg());
	 /************************  for CL0 choose the right CL1  (end) ********************/

	// Set REDL & REDS
	REDLReg REDL; 
	REDSReg REDS;
	
	static int SSRAMLoadBase = 0;
	static int SSRAMLoadOffset = 0;

	//change by liuxie 2010.11.25
	//static int SSRAMSToreBase = 0x800;
	static int SSRAMSToreBase = 0x800;
	static int SSRAMStoreOffset = 0;

	static int SSRAMTempStoreBase = 0x300;
	static int SSRAMTempStoreOffset = 0;

	
	// REDL setting

	Vector<RCA *>::iterator realRCAIter = CL1RCATemp.begin( );

	int REDLNum;
	REDLNum =0;

	int REDLTempPortTotalNum;
	REDLTempPortTotalNum=0;

	for( ; realRCAIter != CL1RCATemp.end() ; ++ realRCAIter)
	{
		RCA * currentRCA = *realRCAIter;

		int REDLTempPortNum;//定义temp port的输入块数，亦即REDL的个数
		REDLTempPortNum = 0;//init

	    //1：如果当前RCA有直接外部输入

		bool directSSRAMInputFlag;
		directSSRAMInputFlag=false;

		Vector<RCAPort> & curRCAInport = currentRCA->inports();//当前RCA的输入

		Vector<RCAPort>::iterator curPortIter;

	    for( curPortIter = curRCAInport.begin(); curPortIter != curRCAInport.end(); ++ curPortIter)
		{
			DFGPort * port = curPortIter->dfgPort();
			assert( port!=0 );
				
			//2012.5.7 longlee 输入的立即数不触发REDL设置
			if( !((IsTempExternPort( curPortIter->dfgPort()))||(IsInnerPort( curPortIter->dfgPort())) || curPortIter->CMValid()) ) 
			{
				REDLNum += 1;//说明肯定有一套REDL来加载外部直接输入
				directSSRAMInputFlag = true;
				break;
			}
		}

		if(directSSRAMInputFlag)//生成直接外部输入的REDL配置信息
		{
			int delta = (currentRCA->rcaSSRAMInTopAddr()-currentRCA->rcaSSRAMInBaseAddr())/2 + 1;
			
			REDL.word0().setSSRAMAddress( currentRCA->rcaSSRAMInBaseAddr() );	

			//FIXME:此处没有考虑到如果delta小于等于FIFO_WIDTH_DATA的情况，导致Height为负数出错
			//修改完成，与264原版配置无差异，mpeg和avs代码效果待检验
			if(delta < FIFO_WIDTH_DATA)
			{
				//FIXME:多循环时是不是要改成looptime-1呢？
				REDL.word1().setSSRAMHeight(0);//modified 20111229 by longlee
			}
			else
			{
				REDL.word1().setSSRAMHeight( (delta / FIFO_WIDTH_DATA)*LoopTime - 1 );//20101117added "-1"
			}
			REDL.word1().setSSRAMLength(FIFO_WIDTH-1);
			REDL.word1().setSSRAMJump(0);
			REDL.word1().setMode(REDL_MODE_1L);        //单行拼接，即不拼接

			CL0ContextTemp.push_back(REDL.word1().reg());
			CL0ContextTemp.push_back(REDL.word0().reg());
			
			C_CL0ContextTemp.push_back(REDL.word0().reg());
			C_CL0ContextTemp.push_back(REDL.word1().reg());
			
		}

		
		//2：当前RCA还有Extern Temp port外部输入；注意，可能来自多块（因为RIM的temp区域倒出到ssram后，可能被多个后续RCA使用）

		List<List<int> > curRCATempDataBlockList = currentRCA->gettempDataBlockList();
		List<List<int> >::iterator curRCATempDataBlockListIter = curRCATempDataBlockList.begin();
		bool ExternSrcRCAExist = false;

		//if( (*curRCATempDataBlockListIter).size() )
		for(; curRCATempDataBlockListIter !=  curRCATempDataBlockList.end(); ++curRCATempDataBlockListIter)
		{
			if(curRCATempDataBlockListIter->size())
			{
				REDLTempPortNum += 1;
			    //20110529 liuxie
			    ExternSrcRCAExist=true;
			    break;    //每个RCA最多只用两个REDL从片外输入数据，一个DFG的外部数据，一个内部tempExtern数据
			}
		}

		std::cout<<"REDLTempPortNum = "<<REDLTempPortNum<<std::endl;

		REDLTempPortTotalNum +=REDLTempPortNum;
		
		//2011.5.29 liuxie
		
		if(ExternSrcRCAExist)
		{
            int delta = currentRCA->getSSRAMTempInTopAddr() - currentRCA->getSSRAMTempInBaseAddr();
			delta = delta/2 +1;
			
			REDL.word0().setSSRAMAddress( currentRCA->getSSRAMTempInBaseAddr());	

			REDL.word1().setSSRAMHeight( (delta / FIFO_WIDTH_DATA) - 1);//20101117added "-1"
	
			REDL.word1().setSSRAMLength(FIFO_WIDTH-1);

			REDL.word1().setSSRAMJump(0);
			REDL.word1().setMode(REDL_MODE_1L);

			CL0ContextTemp.push_back(REDL.word1().reg());
			CL0ContextTemp.push_back(REDL.word0().reg());

			C_CL0ContextTemp.push_back(REDL.word0().reg());
			C_CL0ContextTemp.push_back(REDL.word1().reg());
		}
		
				

				
	}

	REDLNum += REDLTempPortTotalNum;//计算出总的REDL套数




	// REDS setting

	Vector<RCA *>::iterator pseudoRCAIter;
	
	unsigned int REDSNum;

	//2011.5.30 liuxie
	REDSNum =recordPseudoRCA.size();   //有多少个伪RCA就有多少个REDS

	unsigned int directSSRAMOutPseudoRCANum;//直接输出的伪RCA的个数，即是rim区域内的0~1区，因为大小最多为512bytes，所以所有的out region最多使用2个REDS
	directSSRAMOutPseudoRCANum =1;

	/*
	for(pseudoRCAIter = recordPseudoRCA.begin() ; pseudoRCAIter != recordPseudoRCA.end() ; ++ pseudoRCAIter)
	{
		RCA * currentPseudoRCA = *pseudoRCAIter;


		Vector<RCAPort> & curPseudoRCAInport = currentPseudoRCA->inports();//当前RCA的输入

		Vector<RCAPort>::iterator curPseudoPortIter;

		//bool checkDownRegion = false;  //判定该Group中out region是否下部分region中有数据

	    for( curPseudoPortIter = curPseudoRCAInport.begin(); curPseudoPortIter != curPseudoRCAInport.end(); ++ curPseudoPortIter)
		{
			DFGPort * port = curPseudoPortIter->dfgPort();
			assert( port!=0 );
				
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if( (!((IsTempExternPort( curPseudoPortIter->dfgPort()))||(IsInnerPort( curPseudoPortIter->dfgPort())))) || 
				((IsTempExternPort( curPseudoPortIter->dfgPort())) && curPseudoPortIter->IsDFGExternPort())) 
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			{
				directSSRAMOutPseudoRCANum++;
				break;
			}

		}

	}


	std::cout<<"directSSRAMOutPseudoRCANum = "<<directSSRAMOutPseudoRCANum<<std::endl;
	*/

	/*unsigned int pseudoRCAcounter;
	pseudoRCAcounter=0;

	unsigned int pseudoRCASSRAMOutBaseAddr;
	pseudoRCASSRAMOutBaseAddr=0xffffffff;

	unsigned int pseudoRCASSRAMOutTopAddr;
	pseudoRCASSRAMOutTopAddr=0;

	unsigned int pseudoRCASSRAMOutHeight;
	pseudoRCASSRAMOutHeight=0;
	*/
	unsigned int pseudoRCAcounter;
	pseudoRCAcounter=0;

	for( pseudoRCAIter = recordPseudoRCA.begin(); pseudoRCAIter != recordPseudoRCA.end() ; ++ pseudoRCAIter)
	{
		
		


		unsigned int pseudoRCASSRAMOutBaseAddr;
		pseudoRCASSRAMOutBaseAddr=0xffffffff;

		unsigned int pseudoRCASSRAMOutTopAddr;
		pseudoRCASSRAMOutTopAddr=0;
	
		unsigned int pseudoRCASSRAMOutHeight;
		pseudoRCASSRAMOutHeight=0;
		
		
		pseudoRCAcounter ++;

		RCA * curPseudoRCA = *pseudoRCAIter;

		if(pseudoRCAcounter <= directSSRAMOutPseudoRCANum)
		{
			if(static_cast<unsigned int>((*pseudoRCAIter)->rcaSSRAMOutBaseAddr()) < pseudoRCASSRAMOutBaseAddr)
				pseudoRCASSRAMOutBaseAddr = (*pseudoRCAIter)->rcaSSRAMOutBaseAddr();


			if(static_cast<unsigned int>((*pseudoRCAIter)->rcaSSRAMOutTopAddr()) > pseudoRCASSRAMOutTopAddr)
				pseudoRCASSRAMOutTopAddr = (*pseudoRCAIter)->rcaSSRAMOutTopAddr();


			pseudoRCASSRAMOutHeight += ( (((*pseudoRCAIter)->rcaSSRAMOutTopAddr())/FIFO_WIDTH) - (((*pseudoRCAIter)->rcaSSRAMOutBaseAddr())/FIFO_WIDTH) );


				//2011.5.27 liuxie
				//REDS.word0().setSSRAMAddress(SSRAMSToreBase + pseudoRCASSRAMOutBaseAddr);
				REDS.word0().setSSRAMAddress(pseudoRCASSRAMOutBaseAddr);
	
				REDS.word1().setSSRAMHeight(pseudoRCASSRAMOutHeight*LoopTime -1);//20101117added "-1"
				REDS.word1().setSSRAMLength(FIFO_WIDTH-1);	
				REDS.word1().setSSRAMJump(0);
				REDS.word1().setMode(REDS_MODE_ML);

				CL0ContextTemp.push_back(REDS.word1().reg());
				CL0ContextTemp.push_back(REDS.word0().reg());
				
				C_CL0ContextTemp.push_back(REDS.word0().reg());
				C_CL0ContextTemp.push_back(REDS.word1().reg());
	
		}
		else//temp area output to ssram
		{
			REDS.word0().setSSRAMAddress((*pseudoRCAIter)->rcaSSRAMTempOutBaseAddr());
	
			REDS.word1().setSSRAMHeight((((*pseudoRCAIter)->rcaSSRAMTempOutTopAddr())/FIFO_WIDTH) - (((*pseudoRCAIter)->rcaSSRAMTempOutBaseAddr())/FIFO_WIDTH) -1);//20101117added "-1"
			REDS.word1().setSSRAMLength(FIFO_WIDTH-1);	
			REDS.word1().setSSRAMJump(0);
			REDS.word1().setMode(REDS_MODE_ML);

			CL0ContextTemp.push_back(REDS.word1().reg());
			CL0ContextTemp.push_back(REDS.word0().reg());

			C_CL0ContextTemp.push_back(REDS.word0().reg());
			C_CL0ContextTemp.push_back(REDS.word1().reg());
		
		}
	}

	//REDSNum += (recordPseudoRCA.size() - directSSRAMOutPseudoRCANum);//计算出总的REDS套数

/*#################################################################
-----------引自SPECv1.9 包头 格式说明

[1:0]	2bits	8x8 RCA 索引
				00: 左上8x8 RCA
				01: 右上8x8 RCA
				10: 左下8x8 RCA
				11: 右下8x8 RCA

[5:2]   4bits	REDL配置信息个数(0~8)

[10:7]	4bits	REDS配置信息个数(0~8)

[12]	1bit	表示有无立即数
				0: 无立即数
				1: 有2个32bit立即数，分别为选用立即数和必用立即数

[16:13]	4bits	配置包源索引，也是中断响应源索引
				0x0: 主控ARM
				0x1~0x8: μP1~μP8
				0x9~0xF: 保留

[6]				保留
[11]			保留
*/

	std::cout<<"REDL = "<<REDLNum<<std::endl;
	std::cout<<"REDS = "<<REDSNum<<std::endl;

	reg32 cfgPtkHead;
	cfgPtkHead= onRCANum;                            //设置当前函数所在的RCA编号
	cfgPtkHead = cfgPtkHead|(REDLNum<<2);
	cfgPtkHead = cfgPtkHead|(REDSNum<<7);

	std::cout<<"cfgPtkHead = "<<cfgPtkHead<<std::endl;
	
	Vector<reg32> CL0Context_temp;
	Vector<reg32> C_CL0Context_temp;

	CL0Context_temp.push_back(cfgPtkHead);        //将cfgPtkHead压入CL0 Contect的配置信息中
	C_CL0Context_temp.push_back(cfgPtkHead);        //将cfgPtkHead压入CL0 Contect的配置信息中

	for(unsigned int assign2CL0Ctx =0; assign2CL0Ctx < CL0ContextTemp.size(); assign2CL0Ctx++)      //获得除包头外的 CL0的配置信息
	{
		CL0Context_temp.push_back(CL0ContextTemp[assign2CL0Ctx]);
		C_CL0Context_temp.push_back(C_CL0ContextTemp[assign2CL0Ctx]);
	}
	CL0Context.push_back(CL0Context_temp);
	C_CL0Context.push_back(C_CL0Context_temp);

	return 0;
}

