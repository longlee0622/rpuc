
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
	Vector<RCA *> CL1RCATemp;//CL1RCATemp这个vector存放实体RCA，用于传参到CLO去生成REDL
	Vector<RCA *> RCA_MappedFalse;
	Vector<CL1Block> CL1BlockMapped;
	

	CL1RCA.reserve( MAX_CL1_RCA_NUM );
	CL1RCATemp.reserve( MAX_CL1_RCA_NUM );
	RCA_MappedFalse.reserve(MAX_CL1_RCA_NUM);
	CL1BlockMapped.reserve( MAX_CL1BLOCK_NUM );
	
	List<Ptr<RCA> >::iterator rcaIter = rcaList.begin();
	Vector<RCA *>::iterator CL1RCAIter;

	//yin0915begin
	//定义6个关于ssram的地址变量，
	int varSSRAMInBaseAddr;
	int varSSRAMInTopAddr;
	
	int varSSRAMOutBaseAddr;	     
	int varSSRAMOutTopAddr;

	int varSSRAMTempOutBaseAddr;
	int varSSRAMTempOutTopAddr;

	//这里是相对地址
	varSSRAMInBaseAddr = 0;
	varSSRAMInTopAddr = 192; 

	varSSRAMOutBaseAddr = 816;	    
	varSSRAMOutTopAddr =1024;

	varSSRAMTempOutBaseAddr = 192;
	varSSRAMTempOutTopAddr = 816;
	//yin0915end
		
	//2011.5.18 liuxie
	int CL0GroupNumber; //记录当前的CL0 Group编号
    CL0GroupNumber=0;

	while((rcaIter != rcaList.end()) || (RCA_MappedFalse.size() != 0))
	{    
        //2011.5.18 liuxie
	    int REDLSum=0;
	    int REDSSum=0;
	    int REDLSumTemp=0;
	    int REDSSumTemp=0;
		
		//2011.5.18 liuxie
		CL0GroupNumber++;   //Group初始化为1
		//yin0911begin
		//定义一个vector，存放"倒RIM存储器数据的伪RCA"
		Vector<RCA *> recordPseudoRCA;		
		recordPseudoRCA.reserve(MAX_CL1BLOCK_NUM-MAX_CL1_RCA_NUM);
		recordPseudoRCA.clear();
		//yin0911end

		CL1Config cl1config;

		cl1config.setRPUBlockBefoe(RCANumberBefore);

		int actualSSRAMBaseAddr;

		//20110719 liuxie RPU上各个RCA在SSRAM各有一块专门的区域用于中间数据的暂存
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
		//在SSRAM中固定一块区域供读入数据，固定一块区域供写出数据，固定一块区域供中间写出数据的暂存
		cl1config.setSSRAMBaseAddrIn( actualSSRAMBaseAddr);
		//cl1config.setSSRAMBaseAddrIn(0);
		cl1config.setSSRAMTopAddrIn( actualSSRAMBaseAddr + varSSRAMInTopAddr );

		cl1config.setSSRAMBaseAddrOut(actualSSRAMBaseAddr + varSSRAMOutBaseAddr);
		//cl1config.setSSRAMBaseAddrOut(2048);    //起始地址0x800
		cl1config.setSSRAMTopAddrOut(actualSSRAMBaseAddr + varSSRAMOutTopAddr );		

		cl1config.setSSRAMTempBaseAddrOut(actualSSRAMBaseAddr + varSSRAMTempOutBaseAddr);
		//cl1config.setSSRAMTempBaseAddrOut(768);  //起始地址0x300
		cl1config.setSSRAMTempTopAddrOut(actualSSRAMBaseAddr + varSSRAMTempOutTopAddr);

		//yin0915end
			 
		int RIMFullFlag = 0,RIMUsedNum = 0;
		CL1RCA.clear();
		CL1RCATemp.clear();
		
		//在新建一个Group之前先把之前映射失败的RCA映射
		//按照各种策略把RCA加入到RCA_MappedFalse链表中
		//且在现有情况下不会造成RCA_MappedFalse链表中超过12个RCA，或造成RIM满
		CL1RCAIter = RCA_MappedFalse.begin();               
		for( ; CL1RCAIter != RCA_MappedFalse.end(); CL1RCAIter++) 
		{
			
			//2011.5.18 liuxie 
		    ////////////////////////////////////start////////////////////////////////////////////////
			RCA * RCAtemp = * CL1RCAIter;
		    //记录RCA_MappedFalse链表中所有的RCA所需要的REDL数量
			REDLSumTemp=REDLSum;
		    // 当前RCA是否有从外部的输入，有的话REDL++ 
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
			if(ExternPortRecord)    //当前RCA有从片外直接输入的数据
			{
				REDLSumTemp++;
			}
		   //当前RCA的source RCA是否和它在同一个Group中
		   //如果当前RCA的source RCA还没有被mapped，则当前RCA不能被mapped
		   //如果当前RCA的source RCA的GroupNum<当前RCA的GroupNum，则当前RCA必须要从片外再取一次数据，REDL++
		   //如果当前RCA的source RCA和它在同一个Group中，则不作操作
           //理论上当前RCA的source RCA来自多少个Group，则需要多少个temp Extern Port的REDL数目
			
			/*Vector<RCA *>::iterator srcRCAIter;
			int SourceSize = curRCA->sources().size();
			int recordGroupNum[1000];     //先取了一个足够大的数组
			int z=0;
			bool GroupExist2 =false;*/
			RCA * curRCA = * CL1RCAIter;
			for(portIter = RCAInport.begin();portIter != RCAInport.end(); ++ portIter) 
			{
				if(IsTempExternPort(portIter->dfgPort()))  //是ExternTempPort
				{
					ExternTempPortRecord1 = true;
					break;
				}
			}
			if(ExternTempPortRecord1)    //当前RCA有从片外直接输入的数据
			{
				REDLSumTemp++;
			}


			///////////////////////////////////////end/////////////////////////////////////////////////////
			
			(*CL1RCAIter)->setCL0GroupNumber(CL0GroupNumber);
			CL1RCA.push_back( *CL1RCAIter );

		}//end of for loop
		
		RCA_MappedFalse.clear();   //这里必须要保证上面的RCA_MappedFalse链表中的RCA全部被配置，不然会出错



		
		int MappedRCA = (int)CL1RCA.size();
		Vector<RCAPort>::iterator portIter;
		for(int i =0; i <(MAX_CL1_RCA_NUM - MappedRCA) ; ++ i)
		{

			if(rcaIter == rcaList.end()) break;  
			
			RCA * thisRCA = rcaIter->get();
			assert(thisRCA != 0);


			//2011.5.18 liuxie
			//增加分组的约束条件，条件如下：
			/*
				1.REDL的数量不能超过8个；
				2.REDS的数量不能超过8个；（这个问题不存在，因为最多也就有4个伪RCA）
				3.RCA的数量不能超过12个；（MAX_CL1_RCA_NUM） ***
				4.RIM中的out region中的数据满；***
				5.RIM中temp region的数据在不能被抹去的情况下满；（或者直接设置成满了就分第二组）；
			*/
			//////////////////////////////////////////////////start/////////////////////////////////////////////////////////////////////////////
		
			//REDLSumTemp=REDLSum;                                                                    ·
			/* 当前RCA是否有从外部的输入，有的话REDL++ */
			Vector<RCAPort> & RCAInport = (*rcaIter)->inports();		
			bool ExternPortRecord = false;
			bool ExternTempPortRecord = false;
			Vector<RCAPort>::iterator portIter;

			for(portIter = RCAInport.begin();portIter != RCAInport.end(); ++ portIter) 
			{
				if(!(IsInnerPort(portIter->dfgPort())) && !(IsTempExternPort(portIter->dfgPort())) && !(portIter->dfgPort()->isImmPort()))  //既不是innerPort也不是ExternTempPort,而且不是立即数,才是outPort //2012.5.7 longlee 添加立即数判定条件
				{
						ExternPortRecord = true;
						break;
				}
			}
			if(ExternPortRecord)    //当前RCA有从片外直接输入的数据
			{
				REDLSumTemp++;
			}
			//当前RCA的source RCA是否和它在同一个Group中
			//如果当前RCA的source RCA还没有被mapped，则当前RCA不能被mapped
			//如果当前RCA的source RCA的GroupNum<当前RCA的GroupNum，则当前RCA必须要从片外再取一次数据，REDL++
			//如果当前RCA的source RCA和它在同一个Group中，则不作操作
			//理论上当前RCA的source RCA来自多少个Group，则需要多少个temp Extern Port的REDL数目
			
			RCA * curRCA = rcaIter->get();
			for(portIter = RCAInport.begin();portIter != RCAInport.end(); ++ portIter) 
			{
				if(IsTempExternPort(portIter->dfgPort()))  //是ExternTempPort
				{
						ExternTempPortRecord = true;
						break;
				}
			}
			if(ExternTempPortRecord)    //当前RCA有从片外直接输入的数据
			{
				REDLSumTemp++;
			}

			if(REDLSumTemp > CL1_MAX_REDL_CONTEXT)   //当一个Group的REDL数量超过8个时，则将该RCA不能被放置，且需要新建一个Group
			{
				curRCA->setMappedFlag(false);
				//rcaIter--;
				break;                            //生成初步的CL1RCA完成，供后面的genCL1Block进一步筛选
			}


			(*rcaIter)->setCL0GroupNumber(CL0GroupNumber);
			CL1RCA.push_back(rcaIter->get());
			rcaIter++;	
		}

		//PreGenCL1过程,检查加入Remap后的RCA数是否过多
		//////////////////////////////////////////////////////
		Vector<RCA> RCACopy;
		Vector<RCA*> CL1RCACopy;
		List<Ptr<RCA> > rcalist = (*this).rcaList;

		//先进行第一层复制，把所有实体RCA复制下来
		List<Ptr<RCA> >::iterator ListIter;
		for(ListIter = rcalist.begin(); ListIter != rcalist.end(); ++ListIter)
		{
			Ptr<RCA> thisPtr = * ListIter;
			RCA realRCA = * thisPtr;
			RCACopy.push_back(realRCA);
		}
		
		//在实体RCA中进行第二层复制，改变src、tgt指针指向实体RCA中的对应元素
		Vector<RCA>::iterator RCACopyIter;
		for(RCACopyIter = RCACopy.begin();RCACopyIter != RCACopy.end(); ++RCACopyIter)
		{
			Vector<RCA*> srcRCA = (*RCACopyIter).sources();		//获取当前RCA的src
			(*RCACopyIter).sources().clear();
			Vector<RCA*> tgtRCA = (*RCACopyIter).targets();		//获取当前RCA的tgt
			(*RCACopyIter).targets().clear();
			Vector<RCA*>::iterator  RCAIter;
			Vector<RCA>::iterator RCACopyIter_0;
			for(RCAIter = srcRCA.begin();RCAIter != srcRCA.end(); ++RCAIter)	//先遍历一次当前RCA的src,将其中的指针换为实体RCA的地址
			{
				int srcSEQ = (*(*RCAIter)).seqNo();		//获取源RCA的SeqNo
				for(RCACopyIter_0 = RCACopy.begin(); RCACopyIter_0 != RCACopy.end(); ++RCACopyIter_0)
				{
					if((*RCACopyIter_0).seqNo() == srcSEQ)
					{
						(*RCACopyIter).sources().push_back(&(*RCACopyIter_0));
						break;
					}
				}
			}
			for(RCAIter = tgtRCA.begin();RCAIter != tgtRCA.end(); ++RCAIter)	//再遍历一次当前RCA的tgt,将其中的指针换为实体RCA的地址
			{
				int tgtSEQ = (*(*RCAIter)).seqNo();		//获取源RCA的SeqNo
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

		//至此已经将实体RCA拷贝出来并生成了src、tgt的正确指针
		//进行第三步，从实体RCA中模拟出一个新的CL1RCA——CL1RCACopy
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
		CL1BlockMapped = cl1config.genCL1Block(*this, CL1RCA);    //其中的mapRCA函数使得RCA的MappedFlag值改变

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
				//2011.5.18 liuxie  当前RCA满足该Group条件，记录其所在的Group number
				(*CL1RCAIter)->setCL0GroupNumber(CL0GroupNumber);
				
				CL1RCATemp.push_back(*CL1RCAIter);
				CL1RCAIter++;
			}
		}

			
		/* Switch DFGPort Style in TempPortInRIM */        //for temp extern port RCA 
		//2011.5.18 liuxie
		//遍历CL1RCA中的所有RCA，找到这些RCA的targetRCA，如果某个RCA的target RCA不在当前group中
		//则将该RCA的所有outport中的(_i_)port改成(ExternTemp_i_)port，并且将它所有的target RCA中与之相关的port名字改成(ExternTemp_i_)port
		//并且将tempPortInRIM中相应port的信息更新！

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
				else //表示当前targetRCA和当前RCA不在同一个Group中
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
							//修改该targetRCA的所有对应的inner inport为external temp Port 
							Vector<RCA *>::iterator targetRCAtempIter;
							targetRCAtempIter=currRCA->targets().begin();
							for(  ; targetRCAtempIter != currRCA->targets().end(); targetRCAtempIter++)
							{
								
								int targetGroupNum2=(*targetRCAtempIter)->CL0GroupNumber();
								if(targetGroupNum2 != currGroupNum)    //表示当前targetRCA和当前RCA在不同一个Group中
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
								else   //表示当前targetRCA和当前RCA在同一个Group中
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
							//将tempPortInRIM中的对应的temp port设置成temp External Port
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

		//20110719 liuxie 记录整个RPU组已经映射了多少个RCA


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
