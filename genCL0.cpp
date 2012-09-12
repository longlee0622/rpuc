
#include "cl0reg.h"
#include "rpucfg.h"

extern bool IsInnerPort(DFGPort * port);
extern bool IsTempExternPort (DFGPort *port);

int RPUConfig::genCL0Context( CL1Config & cl1config, Vector<RCA *> &CL1RCATemp,Vector<RCA *> &recordPseudoRCA )
{

	Vector<reg32> CL0ContextTemp;//���ݴ������֣�REDL,REDS��������Ϣ������ٸ�ֵ��CL0Context����Ϊǰ��Ҫ���롰��ͷ��
	Vector<reg32> C_CL0ContextTemp;//Cmodel�汾
	// Set Head
	CL0HeadReg headReg;
	
    /************************  for CL0 choose the right CL1  (start) ********************/
	int TempGCGMAddr = GCGMAddrRecord;   //����CL1 context ��ѡַ
	
	headReg.reset();
	
	headReg.setGroupAdrress( TempGCGMAddr );
	
	//TempGCGMAddr += cl1config.totalLength(); //��������ѡ��CL1�׵�ַ�ĵ�ַ�任

	GCGMAddrRecord += cl1config.totalLength();
	
	headReg.setSynMode(SYN_MODE_0);      //����ͬ��ģʽ

	headReg.setConstSelect(0);		//2012.5.7 longlee ����const1��ƫ�Ƶ�ַ��Ч
	headReg.setConstOffset1(0);	//2012.5.7 longlee ����const1��ƫ�Ƶ�ַΪ0
	
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

		int REDLTempPortNum;//����temp port������������༴REDL�ĸ���
		REDLTempPortNum = 0;//init

	    //1�������ǰRCA��ֱ���ⲿ����

		bool directSSRAMInputFlag;
		directSSRAMInputFlag=false;

		Vector<RCAPort> & curRCAInport = currentRCA->inports();//��ǰRCA������

		Vector<RCAPort>::iterator curPortIter;

	    for( curPortIter = curRCAInport.begin(); curPortIter != curRCAInport.end(); ++ curPortIter)
		{
			DFGPort * port = curPortIter->dfgPort();
			assert( port!=0 );
				
			//2012.5.7 longlee �����������������REDL����
			if( !((IsTempExternPort( curPortIter->dfgPort()))||(IsInnerPort( curPortIter->dfgPort())) || curPortIter->CMValid()) ) 
			{
				REDLNum += 1;//˵���϶���һ��REDL�������ⲿֱ������
				directSSRAMInputFlag = true;
				break;
			}
		}

		if(directSSRAMInputFlag)//����ֱ���ⲿ�����REDL������Ϣ
		{
			int delta = (currentRCA->rcaSSRAMInTopAddr()-currentRCA->rcaSSRAMInBaseAddr())/2 + 1;
			
			REDL.word0().setSSRAMAddress( currentRCA->rcaSSRAMInBaseAddr() );	

			//FIXME:�˴�û�п��ǵ����deltaС�ڵ���FIFO_WIDTH_DATA�����������HeightΪ��������
			//�޸���ɣ���264ԭ�������޲��죬mpeg��avs����Ч��������
			if(delta < FIFO_WIDTH_DATA)
			{
				//FIXME:��ѭ��ʱ�ǲ���Ҫ�ĳ�looptime-1�أ�
				REDL.word1().setSSRAMHeight(0);//modified 20111229 by longlee
			}
			else
			{
				REDL.word1().setSSRAMHeight( (delta / FIFO_WIDTH_DATA)*LoopTime - 1 );//20101117added "-1"
			}
			REDL.word1().setSSRAMLength(FIFO_WIDTH-1);
			REDL.word1().setSSRAMJump(0);
			REDL.word1().setMode(REDL_MODE_1L);        //����ƴ�ӣ�����ƴ��

			CL0ContextTemp.push_back(REDL.word1().reg());
			CL0ContextTemp.push_back(REDL.word0().reg());
			
			C_CL0ContextTemp.push_back(REDL.word0().reg());
			C_CL0ContextTemp.push_back(REDL.word1().reg());
			
		}

		
		//2����ǰRCA����Extern Temp port�ⲿ���룻ע�⣬�������Զ�飨��ΪRIM��temp���򵹳���ssram�󣬿��ܱ��������RCAʹ�ã�

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
			    break;    //ÿ��RCA���ֻ������REDL��Ƭ���������ݣ�һ��DFG���ⲿ���ݣ�һ���ڲ�tempExtern����
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

	REDLNum += REDLTempPortTotalNum;//������ܵ�REDL����




	// REDS setting

	Vector<RCA *>::iterator pseudoRCAIter;
	
	unsigned int REDSNum;

	//2011.5.30 liuxie
	REDSNum =recordPseudoRCA.size();   //�ж��ٸ�αRCA���ж��ٸ�REDS

	unsigned int directSSRAMOutPseudoRCANum;//ֱ�������αRCA�ĸ���������rim�����ڵ�0~1������Ϊ��С���Ϊ512bytes���������е�out region���ʹ��2��REDS
	directSSRAMOutPseudoRCANum =1;

	/*
	for(pseudoRCAIter = recordPseudoRCA.begin() ; pseudoRCAIter != recordPseudoRCA.end() ; ++ pseudoRCAIter)
	{
		RCA * currentPseudoRCA = *pseudoRCAIter;


		Vector<RCAPort> & curPseudoRCAInport = currentPseudoRCA->inports();//��ǰRCA������

		Vector<RCAPort>::iterator curPseudoPortIter;

		//bool checkDownRegion = false;  //�ж���Group��out region�Ƿ��²���region��������

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

	//REDSNum += (recordPseudoRCA.size() - directSSRAMOutPseudoRCANum);//������ܵ�REDS����

/*#################################################################
-----------����SPECv1.9 ��ͷ ��ʽ˵��

[1:0]	2bits	8x8 RCA ����
				00: ����8x8 RCA
				01: ����8x8 RCA
				10: ����8x8 RCA
				11: ����8x8 RCA

[5:2]   4bits	REDL������Ϣ����(0~8)

[10:7]	4bits	REDS������Ϣ����(0~8)

[12]	1bit	��ʾ����������
				0: ��������
				1: ��2��32bit���������ֱ�Ϊѡ���������ͱ���������

[16:13]	4bits	���ð�Դ������Ҳ���ж���ӦԴ����
				0x0: ����ARM
				0x1~0x8: ��P1~��P8
				0x9~0xF: ����

[6]				����
[11]			����
*/

	std::cout<<"REDL = "<<REDLNum<<std::endl;
	std::cout<<"REDS = "<<REDSNum<<std::endl;

	reg32 cfgPtkHead;
	cfgPtkHead= onRCANum;                            //���õ�ǰ�������ڵ�RCA���
	cfgPtkHead = cfgPtkHead|(REDLNum<<2);
	cfgPtkHead = cfgPtkHead|(REDSNum<<7);

	std::cout<<"cfgPtkHead = "<<cfgPtkHead<<std::endl;
	
	Vector<reg32> CL0Context_temp;
	Vector<reg32> C_CL0Context_temp;

	CL0Context_temp.push_back(cfgPtkHead);        //��cfgPtkHeadѹ��CL0 Contect��������Ϣ��
	C_CL0Context_temp.push_back(cfgPtkHead);        //��cfgPtkHeadѹ��CL0 Contect��������Ϣ��

	for(unsigned int assign2CL0Ctx =0; assign2CL0Ctx < CL0ContextTemp.size(); assign2CL0Ctx++)      //��ó���ͷ��� CL0��������Ϣ
	{
		CL0Context_temp.push_back(CL0ContextTemp[assign2CL0Ctx]);
		C_CL0Context_temp.push_back(C_CL0ContextTemp[assign2CL0Ctx]);
	}
	CL0Context.push_back(CL0Context_temp);
	C_CL0Context.push_back(C_CL0Context_temp);

	return 0;
}

