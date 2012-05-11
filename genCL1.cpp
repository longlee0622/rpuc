
#include "cl1reg.h"
#include "rpucfg.h"
#include "cl1cfg.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <string>

extern bool IsInnerPort(DFGPort * port);
extern bool IsTempExternPort (DFGPort *port);
extern void PortStyleSwitch( DFGPort *port );
	
int RPUConfig::genGroupContext()
{

	Vector<RCA *> CL1RCA;
	Vector<RCA *> CL1RCATemp;//CL1RCATemp���vector���ʵ��RCA�����ڴ��ε�CLOȥ����REDL
	Vector<RCA *> RCA_MappedFalse;
	Vector<CL1Block> CL1BlockMapped;
	

	CL1RCA.reserve( MAX_CL1_RCA_NUM );
	CL1RCATemp.reserve( MAX_CL1_RCA_NUM );
	RCA_MappedFalse.reserve(MAX_CL1_RCA_NUM);
	CL1BlockMapped.reserve( MAX_CL1BLOCK_NUM );
	
	List<Ptr<RCA> >::iterator rcaIter = rcaList.begin();
	Vector<RCA *>::iterator CL1RCAIter;

	//yin0915begin
	//����6������ssram�ĵ�ַ������
	int varSSRAMInBaseAddr;
	int varSSRAMInTopAddr;
	
	int varSSRAMOutBaseAddr;	     
	int varSSRAMOutTopAddr;

	int varSSRAMTempOutBaseAddr;
	int varSSRAMTempOutTopAddr;

	//��������Ե�ַ
	varSSRAMInBaseAddr = 0;
	varSSRAMInTopAddr = 192; 

	varSSRAMOutBaseAddr = 816;	    
	varSSRAMOutTopAddr =1024;

	varSSRAMTempOutBaseAddr = 192;
	varSSRAMTempOutTopAddr = 816;
	//yin0915end
		
	//2011.5.18 liuxie
	int CL0GroupNumber; //��¼��ǰ��CL0 Group���
    CL0GroupNumber=0;

	while((rcaIter != rcaList.end()) || (RCA_MappedFalse.size() != 0))
	{    
        //2011.5.18 liuxie
	    int REDLSum=0;
	    int REDSSum=0;
	    int REDLSumTemp=0;
	    int REDSSumTemp=0;
		
		//2011.5.18 liuxie
		CL0GroupNumber++;   //Group��ʼ��Ϊ1
		//yin0911begin
		//����һ��vector�����"��RIM�洢�����ݵ�αRCA"
		Vector<RCA *> recordPseudoRCA;		
		recordPseudoRCA.reserve(MAX_CL1BLOCK_NUM-MAX_CL1_RCA_NUM);
		recordPseudoRCA.clear();
		//yin0911end

		CL1Config cl1config;

		cl1config.setRPUBlockBefoe(RCANumberBefore);

		int actualSSRAMBaseAddr;

		//20110719 liuxie RPU�ϸ���RCA��SSRAM����һ��ר�ŵ����������м����ݵ��ݴ�
		if(onRPUNum == 0)
		{
			if(onRCANum == 0)	actualSSRAMBaseAddr = 0;
			else if(onRCANum == 1)	actualSSRAMBaseAddr = 1024;
			else if(onRCANum == 2)	actualSSRAMBaseAddr = 2048;
			else	actualSSRAMBaseAddr = 3072;
		}
		else
		{
			if(onRCANum == 0)	actualSSRAMBaseAddr = 4096;
			else if(onRCANum == 1)	actualSSRAMBaseAddr = 5120;
			else if(onRCANum == 2)	actualSSRAMBaseAddr = 6144;
			else	actualSSRAMBaseAddr = 7168;
		}


		//2011.5.27 liuxie 
		//update those var's value when changing the cl1 group
		//��SSRAM�й̶�һ�����򹩶������ݣ��̶�һ������д�����ݣ��̶�һ�������м�д�����ݵ��ݴ�
		cl1config.setSSRAMBaseAddrIn( actualSSRAMBaseAddr);
		//cl1config.setSSRAMBaseAddrIn(0);
		cl1config.setSSRAMTopAddrIn( actualSSRAMBaseAddr + varSSRAMInTopAddr );

		cl1config.setSSRAMBaseAddrOut(actualSSRAMBaseAddr + varSSRAMOutBaseAddr);
		//cl1config.setSSRAMBaseAddrOut(2048);    //��ʼ��ַ0x800
		cl1config.setSSRAMTopAddrOut(actualSSRAMBaseAddr + varSSRAMOutTopAddr );		

		cl1config.setSSRAMTempBaseAddrOut(actualSSRAMBaseAddr + varSSRAMTempOutBaseAddr);
		//cl1config.setSSRAMTempBaseAddrOut(768);  //��ʼ��ַ0x300
		cl1config.setSSRAMTempTopAddrOut(actualSSRAMBaseAddr + varSSRAMTempOutTopAddr);

		//yin0915end
			 
		int RIMFullFlag = 0,RIMUsedNum = 0;
		CL1RCA.clear();
		CL1RCATemp.clear();
		
		//���½�һ��Group֮ǰ�Ȱ�֮ǰӳ��ʧ�ܵ�RCAӳ��
		//���ո��ֲ��԰�RCA���뵽RCA_MappedFalse������
		//������������²������RCA_MappedFalse�����г���12��RCA�������RIM��
		CL1RCAIter = RCA_MappedFalse.begin();               
		for( ; CL1RCAIter != RCA_MappedFalse.end(); CL1RCAIter++) 
		{
			
			//2011.5.18 liuxie 
		    ////////////////////////////////////start////////////////////////////////////////////////
			RCA * RCAtemp = * CL1RCAIter;
		    //��¼RCA_MappedFalse���������е�RCA����Ҫ��REDL����
			REDLSumTemp=REDLSum;
		    // ��ǰRCA�Ƿ��д��ⲿ�����룬�еĻ�REDL++ 
		    Vector<RCAPort> & RCAInport = (*CL1RCAIter)->inports();		
			bool ExternPortRecord = false;
			Vector<RCAPort>::iterator portIter;
			bool ExternTempPortRecord1 = false;

			for(portIter = RCAInport.begin();portIter != RCAInport.end(); ++ portIter) 
			{
				if(!(IsInnerPort(portIter->dfgPort())))
				{
					ExternPortRecord = true;
					break;
				}
			}
			if(ExternPortRecord)    //��ǰRCA�д�Ƭ��ֱ�����������
			{
				REDLSumTemp++;
			}
		   //��ǰRCA��source RCA�Ƿ������ͬһ��Group��
		   //�����ǰRCA��source RCA��û�б�mapped����ǰRCA���ܱ�mapped
		   //�����ǰRCA��source RCA��GroupNum<��ǰRCA��GroupNum����ǰRCA����Ҫ��Ƭ����ȡһ�����ݣ�REDL++
		   //�����ǰRCA��source RCA������ͬһ��Group�У���������
           //�����ϵ�ǰRCA��source RCA���Զ��ٸ�Group������Ҫ���ٸ�temp Extern Port��REDL��Ŀ
			
			/*Vector<RCA *>::iterator srcRCAIter;
			int SourceSize = curRCA->sources().size();
			int recordGroupNum[1000];     //��ȡ��һ���㹻�������
			int z=0;
			bool GroupExist2 =false;*/
			RCA * curRCA = * CL1RCAIter;
			for(portIter = RCAInport.begin();portIter != RCAInport.end(); ++ portIter) 
			{
				if(IsTempExternPort(portIter->dfgPort()))  //��ExternTempPort
				{
					ExternTempPortRecord1 = true;
					break;
				}
			}
			if(ExternTempPortRecord1)    //��ǰRCA�д�Ƭ��ֱ�����������
			{
				REDLSumTemp++;
			}


			///////////////////////////////////////end/////////////////////////////////////////////////////
			
			(*CL1RCAIter)->setCL0GroupNumber(CL0GroupNumber);
			CL1RCA.push_back( *CL1RCAIter );

		}//end of for loop
		
		RCA_MappedFalse.clear();   //�������Ҫ��֤�����RCA_MappedFalse�����е�RCAȫ�������ã���Ȼ�����



		
		int MappedRCA = (int)CL1RCA.size();
		Vector<RCAPort>::iterator portIter;
		for(int i =0; i <(MAX_CL1_RCA_NUM - MappedRCA) ; ++ i)
		{

			if(rcaIter == rcaList.end()) break;  
			
			RCA * thisRCA = rcaIter->get();
			assert(thisRCA != 0);


			//2011.5.18 liuxie
			//���ӷ����Լ���������������£�
			/*
				1.REDL���������ܳ���8����
				2.REDS���������ܳ���8������������ⲻ���ڣ���Ϊ���Ҳ����4��αRCA��
				3.RCA���������ܳ���12������MAX_CL1_RCA_NUM�� ***
				4.RIM�е�out region�е���������***
				5.RIM��temp region�������ڲ��ܱ�Ĩȥ�����������������ֱ�����ó����˾ͷֵڶ��飩��
			*/
			//////////////////////////////////////////////////start/////////////////////////////////////////////////////////////////////////////
		
			//REDLSumTemp=REDLSum;                                                                    ��
			/* ��ǰRCA�Ƿ��д��ⲿ�����룬�еĻ�REDL++ */
			Vector<RCAPort> & RCAInport = (*rcaIter)->inports();		
			bool ExternPortRecord = false;
			bool ExternTempPortRecord = false;
			Vector<RCAPort>::iterator portIter;

			for(portIter = RCAInport.begin();portIter != RCAInport.end(); ++ portIter) 
			{
				if(!(IsInnerPort(portIter->dfgPort())) && !(IsTempExternPort(portIter->dfgPort())) && !(portIter->dfgPort()->isImmPort()))  //�Ȳ���innerPortҲ����ExternTempPort,���Ҳ���������,����outPort //2012.5.7 longlee ����������ж�����
				{
						ExternPortRecord = true;
						break;
				}
			}
			if(ExternPortRecord)    //��ǰRCA�д�Ƭ��ֱ�����������
			{
				REDLSumTemp++;
			}
			//��ǰRCA��source RCA�Ƿ������ͬһ��Group��
			//�����ǰRCA��source RCA��û�б�mapped����ǰRCA���ܱ�mapped
			//�����ǰRCA��source RCA��GroupNum<��ǰRCA��GroupNum����ǰRCA����Ҫ��Ƭ����ȡһ�����ݣ�REDL++
			//�����ǰRCA��source RCA������ͬһ��Group�У���������
			//�����ϵ�ǰRCA��source RCA���Զ��ٸ�Group������Ҫ���ٸ�temp Extern Port��REDL��Ŀ
			
			RCA * curRCA = rcaIter->get();
			for(portIter = RCAInport.begin();portIter != RCAInport.end(); ++ portIter) 
			{
				if(IsTempExternPort(portIter->dfgPort()))  //��ExternTempPort
				{
						ExternTempPortRecord = true;
						break;
				}
			}
			if(ExternTempPortRecord)    //��ǰRCA�д�Ƭ��ֱ�����������
			{
				REDLSumTemp++;
			}

			if(REDLSumTemp > CL1_MAX_REDL_CONTEXT)   //��һ��Group��REDL��������8��ʱ���򽫸�RCA���ܱ����ã�����Ҫ�½�һ��Group
			{
				curRCA->setMappedFlag(false);
				//rcaIter--;
				break;                            //���ɳ�����CL1RCA��ɣ��������genCL1Block��һ��ɸѡ
			}


			(*rcaIter)->setCL0GroupNumber(CL0GroupNumber);
			CL1RCA.push_back(rcaIter->get());
			rcaIter++;	
		}

		//PreGenCL1����,������Remap���RCA���Ƿ����
		//////////////////////////////////////////////////////
		Vector<RCA> RCACopy;
		Vector<RCA*> CL1RCACopy;
		List<Ptr<RCA> > rcalist = (*this).rcaList;

		//�Ƚ��е�һ�㸴�ƣ�������ʵ��RCA��������
		List<Ptr<RCA> >::iterator ListIter;
		for(ListIter = rcalist.begin(); ListIter != rcalist.end(); ++ListIter)
		{
			Ptr<RCA> thisPtr = * ListIter;
			RCA realRCA = * thisPtr;
			RCACopy.push_back(realRCA);
		}
		
		//��ʵ��RCA�н��еڶ��㸴�ƣ��ı�src��tgtָ��ָ��ʵ��RCA�еĶ�ӦԪ��
		Vector<RCA>::iterator RCACopyIter;
		for(RCACopyIter = RCACopy.begin();RCACopyIter != RCACopy.end(); ++RCACopyIter)
		{
			Vector<RCA*> srcRCA = (*RCACopyIter).sources();		//��ȡ��ǰRCA��src
			(*RCACopyIter).sources().clear();
			Vector<RCA*> tgtRCA = (*RCACopyIter).targets();		//��ȡ��ǰRCA��tgt
			(*RCACopyIter).targets().clear();
			Vector<RCA*>::iterator  RCAIter;
			Vector<RCA>::iterator RCACopyIter_0;
			for(RCAIter = srcRCA.begin();RCAIter != srcRCA.end(); ++RCAIter)	//�ȱ���һ�ε�ǰRCA��src,�����е�ָ�뻻Ϊʵ��RCA�ĵ�ַ
			{
				int srcSEQ = (*(*RCAIter)).seqNo();		//��ȡԴRCA��SeqNo
				for(RCACopyIter_0 = RCACopy.begin(); RCACopyIter_0 != RCACopy.end(); ++RCACopyIter_0)
				{
					if((*RCACopyIter_0).seqNo() == srcSEQ)
					{
						(*RCACopyIter).sources().push_back(&(*RCACopyIter_0));
						break;
					}
				}
			}
			for(RCAIter = tgtRCA.begin();RCAIter != tgtRCA.end(); ++RCAIter)	//�ٱ���һ�ε�ǰRCA��tgt,�����е�ָ�뻻Ϊʵ��RCA�ĵ�ַ
			{
				int tgtSEQ = (*(*RCAIter)).seqNo();		//��ȡԴRCA��SeqNo
				for(RCACopyIter_0 = RCACopy.begin(); RCACopyIter_0 != RCACopy.end(); ++RCACopyIter_0)
				{
					if((*RCACopyIter_0).seqNo() == tgtSEQ)
					{
						(*RCACopyIter).targets().push_back(&(*RCACopyIter_0));
						break;
					}
				}
			}
		}

		//�����Ѿ���ʵ��RCA����������������src��tgt����ȷָ��
		//���е���������ʵ��RCA��ģ���һ���µ�CL1RCA����CL1RCACopy
		for(CL1RCAIter = CL1RCA.begin(); CL1RCAIter != CL1RCA.end(); ++CL1RCAIter)
		{
			int seq = (*(*CL1RCAIter)).seqNo();
			for(RCACopyIter = RCACopy.begin(); RCACopyIter != RCACopy.end(); ++RCACopyIter)
			{
				if((*RCACopyIter).seqNo() == seq)
				{
					CL1RCACopy.push_back(&(*RCACopyIter));
					break;
				}
			}
		}

		CL1Config cl1config_copy;
		int Total = cl1config_copy.PreGenCL1(CL1RCACopy, this->DFGInBaseAddress());
		for(CL1RCAIter = CL1RCA.begin()+ Total; CL1RCAIter != CL1RCA.end();)
		{
			(*CL1RCAIter)->setCL0GroupNumber(0);
			RCA_MappedFalse.push_back( *CL1RCAIter );
			CL1RCAIter = CL1RCA.erase(CL1RCAIter);
		}

		/* Generate CL1 Block without PseudoRCA */
		CL1BlockMapped = cl1config.genCL1Block(*this, CL1RCA);    //���е�mapRCA����ʹ��RCA��MappedFlagֵ�ı�

		/* push RCA that mapped false into Temp Vector */

		for(  CL1RCAIter= CL1RCA.begin() ; CL1RCAIter != CL1RCA.end();)
		{
			if ( !((*CL1RCAIter)->getMappedFlag()) )
			{
				 (*CL1RCAIter)->setCL0GroupNumber(0);
				 RCA_MappedFalse.push_back( *CL1RCAIter );
				 CL1RCAIter = CL1RCA.erase(CL1RCAIter);
			}
			else
			{
				//2011.5.18 liuxie  ��ǰRCA�����Group��������¼�����ڵ�Group number
				(*CL1RCAIter)->setCL0GroupNumber(CL0GroupNumber);
				
				CL1RCATemp.push_back(*CL1RCAIter);
				CL1RCAIter++;
			}
		}

			
		/* Switch DFGPort Style in TempPortInRIM */        //for temp extern port RCA 
		//2011.5.18 liuxie
		//����CL1RCA�е�����RCA���ҵ���ЩRCA��targetRCA�����ĳ��RCA��target RCA���ڵ�ǰgroup��
		//�򽫸�RCA������outport�е�(_i_)port�ĳ�(ExternTemp_i_)port�����ҽ������е�target RCA����֮��ص�port���ָĳ�(ExternTemp_i_)port
		//���ҽ�tempPortInRIM����Ӧport����Ϣ���£�

  ////////////////////////////////////////////////start//////////////////////////////////////////////////////////
        Vector<RCA *>::iterator CL1RCAtempIter2;
		CL1RCAtempIter2= CL1RCATemp.begin();
		
		for(   ; CL1RCAtempIter2 != CL1RCATemp.end(); CL1RCAtempIter2++)
		{
			RCA * currRCA =  *CL1RCAtempIter2;
			int currGroupNum = currRCA->CL0GroupNumber();
			Vector<RCA *>::iterator targetRCAIter;
			targetRCAIter=currRCA->targets().begin();
			for(; targetRCAIter!=currRCA->targets().end(); targetRCAIter++)
			{
				int targetGroupNum=(*targetRCAIter)->CL0GroupNumber();
				if(currGroupNum == targetGroupNum)     
					continue;
				else //��ʾ��ǰtargetRCA�͵�ǰRCA����ͬһ��Group��
				{
					Vector<RCAPort>::iterator curRCAPortIter;
					curRCAPortIter= currRCA->outports().begin();
					for(; curRCAPortIter != currRCA->outports().end(); curRCAPortIter++)
					{
						if(!(IsInnerPort((curRCAPortIter)->dfgPort())) && !(IsTempExternPort((curRCAPortIter)->dfgPort())))
							continue;
						else //
						{
							DFGPort * currDFGPort = curRCAPortIter->dfgPort();
							//�޸ĸ�targetRCA�����ж�Ӧ��inner inportΪexternal temp Port 
							Vector<RCA *>::iterator targetRCAtempIter;
							targetRCAtempIter=currRCA->targets().begin();
							for(  ; targetRCAtempIter != currRCA->targets().end(); targetRCAtempIter++)
							{
								
								int targetGroupNum2=(*targetRCAtempIter)->CL0GroupNumber();
								if(targetGroupNum2 != currGroupNum)    //��ʾ��ǰtargetRCA�͵�ǰRCA�ڲ�ͬһ��Group��
								{
									Vector<RCAPort>::iterator portTempIter;
									portTempIter=(*targetRCAtempIter)->inports().begin();
									for(; portTempIter != (*targetRCAtempIter)->inports().end(); portTempIter++)
									{
										if(!(IsInnerPort((portTempIter)->dfgPort())) && !(IsTempExternPort((portTempIter)->dfgPort())))
											continue;
										if((portTempIter)->dfgPort() == currDFGPort)
										{
											PortStyleSwitch((portTempIter)->dfgPort());
											portTempIter->setInSameGroup(false);
										}
									}
								}
								else   //��ʾ��ǰtargetRCA�͵�ǰRCA��ͬһ��Group��
								{
									Vector<RCAPort>::iterator portTempIter2;
									portTempIter2=(*targetRCAtempIter)->inports().begin();
									for(; portTempIter2 != (*targetRCAtempIter)->inports().end(); portTempIter2++)
									{
										if(!(IsInnerPort((portTempIter2)->dfgPort())) && !(IsTempExternPort((portTempIter2)->dfgPort())))
											continue;
										if((portTempIter2)->dfgPort() == currDFGPort)
										{
											PortStyleSwitch((portTempIter2)->dfgPort());
											portTempIter2->setInSameGroup(true);
										}
									}

								}
							}
							//��tempPortInRIM�еĶ�Ӧ��temp port���ó�temp External Port
							Vector<RCAPort *> TempPortInRIM =  cl1config.getTempPortInRIM();
							Vector<RCAPort *>::iterator RIMPortIter = TempPortInRIM.begin();
							for( ; RIMPortIter != TempPortInRIM.end(); RIMPortIter++ )
							{
								if((*RIMPortIter)->dfgPort() == currDFGPort)
								{
									PortStyleSwitch((*RIMPortIter)->dfgPort());
								}
							}
						    //PortStyleSwitch((curRCAPortIter)->dfgPort());

						    Vector<RCAPort *>::iterator RIMPortIter2 = TempPortInRIM.begin();
						    for(; RIMPortIter2 != TempPortInRIM.end(); RIMPortIter2++)
						    {
							    if((*RIMPortIter2)->dfgPort() == (curRCAPortIter)->dfgPort())
								   break;
						    }
						    if(RIMPortIter2 ==  TempPortInRIM.end())
						    {
							   cl1config.insertTempPort(&(*curRCAPortIter));
							   int row = curRCAPortIter->RIMRow();
							   
							   assert(0<=row && row<=31);
							   if( row <= 15 && row >=0)
								   cl1config.setAreaTempCounter(1,0);
							   else
								   cl1config.setAreaTempCounter(1,1);

						    }
						}
					}
					
					curRCAPortIter= currRCA->outports().begin();
					for(; curRCAPortIter != currRCA->outports().end() ; curRCAPortIter++)
					{
						PortStyleSwitch((curRCAPortIter)->dfgPort());
					}
				}
			}
			

         }

		///////////////////////////////////////////////end//////////////////////////////////////////////////////////////

		//FIXME : for debug
		//bool test = IsInnerPort((*tempPortIter)->dfgPort());
		//std::cout<<"PortStyleSwitch success? "<<test<<std::endl;
		
		/* Append the pseudo-RCA */
		const Vector<CL1Block> 
			pseudoBlock = cl1config.insertPseudoRCA(*this,CL1RCA,recordPseudoRCA);

		Vector<RCA *>::iterator PseudoRCAIter;
		PseudoRCAIter = recordPseudoRCA.begin();
		for( ; PseudoRCAIter != recordPseudoRCA.end(); PseudoRCAIter++)
		{
			(*PseudoRCAIter)->setCL0GroupNumber(CL0GroupNumber);
		}
		
		Vector<CL1Block>::const_iterator blockIter;
		for(blockIter = pseudoBlock.begin();blockIter != pseudoBlock.end(); ++ blockIter)
		{

			CL1BlockMapped.push_back(*blockIter);
		}

		
		Vector<CL1Block>::iterator BlockIter;
		int lx = 0; 

		for(BlockIter = CL1BlockMapped.begin(); BlockIter != CL1BlockMapped.end(); BlockIter++, lx++)
		{
			(*BlockIter).setBlockBefore(RCANumberBefore + lx);
		}

		/* generate GroupContenxt */
		CL1Context.push_back( cl1config.genRegs( CL1BlockMapped ) );
		//gen CL0 Context information
		genCL0Context( cl1config,CL1RCATemp,recordPseudoRCA );
		//gen CL1 Context information
		genCL2Context( CL1RCA );

		RCANumberBefore += CL1RCA.size();

		//20110719 liuxie ��¼����RPU���Ѿ�ӳ���˶��ٸ�RCA


		if(cl1config.getSSRAMTopAddrIn() < this->DFGInBaseAddress() + 192 )
		{
			//varSSRAMInBaseAddr = cl1config.getSSRAMTopAddrIn();
			varSSRAMInBaseAddr = 0;
			varSSRAMInTopAddr = 192;
		}

		if(cl1config.getSSRAMTopAddrOut() < this->DFGInBaseAddress() + 1024)
		{
			varSSRAMOutBaseAddr = cl1config.getSSRAMTopAddrOut() - this->DFGInBaseAddress();	    
			varSSRAMOutTopAddr = 1024;
		}

		if(cl1config.getSSRAMTempTopAddrOut() < this->DFGInBaseAddress() + 816)
		{
			varSSRAMTempOutBaseAddr = cl1config.getSSRAMTempTopAddrOut() - this->DFGInBaseAddress();
			varSSRAMTempOutTopAddr = 816;
		}



#if 0 //for RCA interface file out
		//2011.6.19 liuxie for port interface
		//--------------------------------start---------------------------------//
		
		Vector<RCA *>::iterator rcaIter;
		for(rcaIter = CL1RCA.begin(); rcaIter != CL1RCA.end(); ++ rcaIter)
		{
			RCA * thisRCA = * rcaIter;

			const int inSize = thisRCA->inports().size();
			const int outSize = thisRCA->outports().size();
			const int rcaSeqNo = thisRCA->seqNo();

			const String & graphName = dfgGraph.name();
		
			// uppercase of name
			String GRAPHNAME = graphName;
			std::transform(GRAPHNAME.begin(), GRAPHNAME.end(), 
				GRAPHNAME.begin(), ::toupper);

		
			headFile<<"// RCA"<<rcaSeqNo<<"\n";
			headFile<<"#define "<<GRAPHNAME<<"_RCA"<<rcaSeqNo<<"_DIN_SIZE\t"<<inSize<<"\n";
			headFile<<"#define "<<GRAPHNAME<<"_RCA"<<rcaSeqNo<<"_DOUT_SIZE\t"<<outSize<<"\n\n";

			// (1) Print declaration
			headFile<<"#define "<<GRAPHNAME<<"_RCA"<<rcaSeqNo<<"_VAR_DEC\t\\\n";
		
			Vector<RCAPort>::iterator portIter;
			int seqNo = 0;

			for(portIter = thisRCA->outports().begin();
				portIter != thisRCA->outports().end(); ++ portIter, ++ seqNo){

				if( IsInnerPort(portIter->dfgPort()) )
					headFile<<"\tshort "<<*portIter->dfgPort()<<";\t\\\n";
			}

			// (2) Print inports
			headFile<<"\n\n#define "<<GRAPHNAME<<"_RCA"<<rcaSeqNo<<"_DIN\t\\\n";
		
			for(portIter = thisRCA->inports().begin();
				portIter != thisRCA->inports().end(); ++ portIter){

					std::stringstream portName;
					portName<<*portIter->dfgPort();
					String varName = portName.str().substr(portName.str().find_last_of('.')+1);

					headFile<<"\tarray_in["<<seqNo<<"] = (short)"<<varName<<";\t\\\n";
			}

			// (3) Print outport
			headFile<<"\n\n#define "<<GRAPHNAME<<"_RCA"<<rcaSeqNo<<"_DOUT\t\\\n";

			seqNo = 0;
			for(portIter = thisRCA->outports().begin();
				portIter != thisRCA->outports().end(); ++ portIter){

					std::stringstream portName;
					portName<<*portIter->dfgPort();
					String varName = portName.str().substr(portName.str().find_last_of('.')+1);

					headFile<<'\t'<<varName<<" = array_out["<<rcaSeqNo<<"];\t\\\n";
			}

			headFile<<std::endl;
		}

	//--------------------------------end---------------------------------//
#endif

#if 0    //for RCA patch file out
	//2011.6.20 liuxie for patch file
	//--------------------------------start patch file for RCA-------------------------------//
		


			for(rcaIter = CL1RCA.begin(); rcaIter != CL1RCA.end(); ++ rcaIter)
			{
				RCA * thisRCA = * rcaIter;
				int seqNo = thisRCA->seqNo();

				patchFile<<"// RCA"<<seqNo<<" execute function definition\n"
				 <<"#define "<<GRAPHNAME<<"_RCA"<<seqNo<<"_EXECUTE\t\\\n"
				 <<"\t\trca_execute(\t\\\n"
				 <<"\t\t\t"<<graphName<<"_RCA"<<seqNo<<"_Config,\t\\\n"
				 <<"\t\t\tDMA_CTL,\t\\\n"
				 <<"\t\t\t0,\t\\\n"
				 <<"\t\t\t"<<GRAPHNAME<<"_RCA"<<seqNo<<"_DIN_SIZE,\t\\\n"
				 <<"\t\t\t"<<GRAPHNAME<<"_RCA"<<seqNo<<"_DOUT_SIZE,\t\\\n"
				 <<"\t\t\t"<<thisRCA->loopNum()<<",\t\\\n"
				 <<"\t\t\tDIN_BASIC_CTL\t\\\n"
				 <<"\t\t);\n\n";

				patchFile<<"#define "<<GRAPHNAME<<"_RCA"<<seqNo<<"_FUNC\t\\\n"
				 <<"\t\t"<<GRAPHNAME<<"_RCA"<<seqNo<<"_DIN\t\\\n"
				 <<"\t\t"<<GRAPHNAME<<"_RCA"<<seqNo<<"_EXECUTE\t\\\n"
				 <<"\t\t"<<GRAPHNAME<<"_RCA"<<seqNo<<"_DOUT\n\n";
			
			}

			patchFile<<"\n\n#define "<<GRAPHNAME<<"_FUNC\t\\\n";
			

			for(rcaIter = CL1RCA.begin(); rcaIter != CL1RCA.end(); ++ rcaIter)
			{
				RCA * thisRCA = * rcaIter;
				const int seqNo = thisRCA->seqNo();

				patchFile<<"\t\t"<<GRAPHNAME<<"_RCA"<<seqNo<<"_FUNC\t\\\n";
			}

			patchFile<<"\n\n#define "<<GRAPHNAME<<"_VAR_DEC\t\\\n";
			for(rcaIter = CL1RCA.begin(); rcaIter != CL1RCA.end(); ++ rcaIter)
			{
				RCA * thisRCA = * rcaIter;
				const int seqNo = thisRCA->seqNo();

				patchFile<<"\t\t"<<GRAPHNAME<<"_RCA"<<seqNo<<"_VAR_DEC\t\\\n";
			}

			patchFile<<std::endl;
		//-----------------------------end patch File for RCA-------------------------//

#endif

	} //end rcaList loop

	return 0;	
}
