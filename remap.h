/******************************************
*	2011.7.19 longlee
*	本函数用于在mapRCA过程中出现RIF越界
*	时插入缓冲RCA重排RCA间的端口关系
*
******************************************/
#include <vector>
#include <algorithm>
#include <iostream>
#include <strstream>
#include <string>
#include "cl1cfg.h"
#include "rpucfg.h"

extern bool IsInnerPort(DFGPort * port);
extern bool IsTempExternPort (DFGPort *port);
extern void ChangeName(DFGPort * port);
int remapSeqNo = 0;
int PseudoRCANum = 0;

static bool PortSort(
	RCAPort left, RCAPort right){
		if(left.RIFRow() != right.RIFRow()) return left.RIFRow() <  right.RIFRow();
		else return left.RIFCol() < right.RIFCol();
}

/*
void remap(Vector<RCA*> & rcas, int index, Vector<RCAPort *> & tempPortInRIM,RPUConfig & config,int PreFlag) {

	
	//将三个新RCA添加进RCAlist
	int Base;
	if (PreFlag == 1) Base =200;
	else Base = config.rcas().size();
	RCA * buf1 = new RCA(Base + remapSeqNo + PseudoRCANum);
	++remapSeqNo;
    RCA * buf2 = new RCA(Base + remapSeqNo + PseudoRCANum);
	++remapSeqNo;
	RCA * buf3 = new RCA(Base + remapSeqNo + PseudoRCANum);
	++remapSeqNo;
	buf1->setRemapFlag(true);	//标记这个RCA为Remap添加的RCA，在遍历tempPortInRIM过程中用到
	buf2->setRemapFlag(true);
	buf3->setRemapFlag(true);
	rcas.insert(rcas.begin()+index,buf1);
	rcas.insert(rcas.begin()+index+1,buf2);
	rcas.insert(rcas.begin()+index+2,buf3);

	RCA* thisRCA =*(rcas.begin()+index+3);	//thisRCA指针固定，专指需要remap的目标RCA，便于RCA间的定位
	buf1->setCL0GroupNumber(thisRCA->CL0GroupNumber());
	buf2->setCL0GroupNumber(thisRCA->CL0GroupNumber());
	buf3->setCL0GroupNumber(thisRCA->CL0GroupNumber());
	Vector<RCAPort> & rcaInport = thisRCA->inports();
	sort(rcaInport.begin(),rcaInport.end(),PortSort);
	Vector<RCAPort>::iterator portIter;
	Vector<RCAPort*>::iterator outIter;
	
	int buf1portCnt = 0;
	int buf2portCnt = 0;
	int buf3portCnt = 0;
	int buf1Base = 0;
	bool buf2Enable = true;
	bool buf3Enable = false;		//需要时再开启buf3
	int Cnt = 0;
	int ImmdtCnt=0;
	for(portIter = rcaInport.begin();portIter != rcaInport.end(); ++ portIter, ++Cnt) 
	{
		if (portIter->dfgPort()->isImmPort())	
		{
			ImmdtCnt++;
			continue;
		}
		if((portIter->RIFRow() > 7) || Cnt >= 64) break;
		++buf1portCnt;
	}
	Cnt = 0;
	int buf2Base = (rcaInport.begin() + buf1portCnt)->RIFRow();
	for(portIter = rcaInport.begin() + buf1portCnt; portIter != rcaInport.end(); ++portIter,++Cnt)
	{
		if (portIter->dfgPort()->isImmPort())
		{
			ImmdtCnt++;
			continue;
		}
		if((portIter->RIFRow() > buf2Base + 7) || Cnt >= 64) break;
		++buf2portCnt;
	}
	if (buf1portCnt + buf2portCnt < rcaInport.size()-ImmdtCnt) buf3Enable = true;	//说明前两个RCA未能放下所有的inports，需要第三个
	Cnt = 0;
	if(buf3Enable)
	{
		int buf3Base = (rcaInport.begin() + buf1portCnt + buf2portCnt)->RIFRow();
		for(portIter = rcaInport.begin() + buf1portCnt; portIter != rcaInport.end(); ++portIter,++Cnt)
		{
			if((portIter->RIFRow() > buf3Base + 7) || Cnt >= 64) break;
			++buf3portCnt;
		}
	}


	if(buf2Enable)					//判断加入remapRCA后会不会导致CL1RCA个数越界
	{
		if(rcas.size()>12)
		{
			RCA* mapFalseRCA =*(rcas.begin()+rcas.size() - 1);
			mapFalseRCA->setTooManyFlag(true);
		}
	}
	else							//buf2Enable对rcas实际的最终个数有影响，分情况处理
	{
		if(rcas.size()-1 >12)
		{
			RCA* mapFalseRCA =*(rcas.begin()+rcas.size() - 1);
			mapFalseRCA->setTooManyFlag(true);
		}
	}

	Vector<RCAPort> buf1Inport,buf2Inport,buf1Outport,buf2Outport,buf3Inport,buf3Outport;
		
	//将inports按照每8个RIFRow为单位分组，为往新RCA上映射做准备
	Cnt = 0;
	for(portIter = rcaInport.begin();portIter != rcaInport.end(); ++ portIter,++Cnt)
	{
		//2012.5.7 longlee remapRCA 里不处理Immdt输入
		if(portIter->dfgPort()->isImmPort()) 
		{
			Cnt--;
			continue;
		}

		if (Cnt < buf1portCnt)
		{
			buf1Inport.push_back(portIter->dfgPort());
			buf1Inport.at(buf1Inport.size()-1).setInSameGroup(portIter->IsInSameGroup());	//准备将新RCA设成与原始RCA相同
		}
		else if (Cnt < buf1portCnt + buf2portCnt)
		{
			buf2Inport.push_back(portIter->dfgPort());
			buf2Inport.at(buf2Inport.size()-1).setInSameGroup(portIter->IsInSameGroup());	//准备将新RCA设成与原始RCA相同
		}else
		{
			buf3Inport.push_back(portIter->dfgPort());
			buf3Inport.at(buf3Inport.size()-1).setInSameGroup(portIter->IsInSameGroup());	//准备将新RCA设成与原始RCA相同
		}

	//	if( IsInnerPort(portIter->dfgPort()) || ((IsTempExternPort(portIter->dfgPort())) && portIter->IsInSameGroup())){}	//内部节点，不修改组内组外
	//	else if( !(IsTempExternPort(portIter->dfgPort())) ) {}	//外部节点，什么都不做
	//	else portIter->setInSameGroup(true);	//外部临时节点，由组外输入换为组内输入
		portIter->setInSameGroup(true);
	}
	
	for(int choice = 0;choice <3;++choice)
	{//开始进行bufferRCA的重排
		RCA * buffRCA;
		if(choice == 0) buffRCA = * (rcas.begin()+index);
		if(choice == 1) buffRCA = * (rcas.begin()+index+1);
		if(choice == 2) buffRCA = * (rcas.begin()+index+2);
		buffRCA->targets().push_back(thisRCA);
		if(choice == 0)
		{
			buffRCA->inports().empty();
			buffRCA->inports().insert(buffRCA->inports().begin(),buf1Inport.begin(),buf1Inport.end());
			buffRCA->outports().empty();
			buffRCA->outports().insert(buffRCA->outports().begin(),buffRCA->inports().begin(),buffRCA->inports().end());

		}
		else if (choice == 1)
		{
			buffRCA->inports().empty();
			buffRCA->inports().insert(buffRCA->inports().begin(),buf2Inport.begin(),buf2Inport.end());
			buffRCA->outports().empty();
			buffRCA->outports().insert(buffRCA->outports().begin(),buffRCA->inports().begin(),buffRCA->inports().end());
	
		}
		else if (choice ==2)
		{
			buffRCA->inports().empty();
			buffRCA->inports().insert(buffRCA->inports().begin(),buf3Inport.begin(),buf3Inport.end());
			buffRCA->outports().empty();
			buffRCA->outports().insert(buffRCA->outports().begin(),buffRCA->inports().begin(),buffRCA->inports().end());
			if (!buf3Enable) continue;
		}
			
			
		//将inports摆放至RCA节点上
		int i;
		Vector<RCA*>::iterator rcaIter=rcas.begin();
		for (portIter = buffRCA->inports().begin() ,i=0; portIter !=buffRCA->inports().end(); ++portIter, ++i)
		{
			if ( IsInnerPort(portIter->dfgPort()) || ((IsTempExternPort(portIter->dfgPort())) && portIter->IsInSameGroup()))
			{	//对于内部节点的排布
				Vector<RCAPort*>::iterator outIter;
				for (outIter = tempPortInRIM.begin(); outIter != tempPortInRIM.end(); ++outIter)
				{
					RCAPort *thisPort = * outIter;
					if (portIter->dfgPort() != thisPort->dfgPort()) continue;
					DFGNode * bypNode = config.newBypsNode();	
					bypNode->addSource(thisPort->dfgPort());
					bypNode->addTarget(thisPort->dfgPort());
					if(i <64)	buffRCA->node(i).setDFGNode(bypNode);
					else buffRCA->temp(i-64).setDFGNode(bypNode);
				}

				int SeqNo = -1;
				String VarName = static_cast <DFGVarPort*>(portIter->dfgPort())->name();
				sscanf(VarName.c_str(),"%*[^0-9]%i",&SeqNo); //取得父节点的SeqNo
				for (rcaIter = rcas.begin();rcaIter != rcas.begin()+index;++rcaIter)
				{
					RCA * sourceRCA = * rcaIter;
					for (int rc=0;rc != RC_REG_NUM; ++rc)
					{
						RCANode * thisNode = &sourceRCA->node(rc);
						DFGNode * sourceDfgNode = thisNode->dfgNode();
						if (sourceDfgNode == 0) continue;
						if(sourceDfgNode->seqNo() ==SeqNo)		//找到父节点
						{
							//SsourceRCA->targets().unique();
							Vector<RCA*>::iterator TgtIter = sourceRCA->targets().begin();
							for(;TgtIter != sourceRCA->targets().end();++TgtIter)
							{
								if(*TgtIter == thisRCA)
								{
									//sourceRCA->targets().erase(TgtIter); //erase不能在循环中做，否则在一个source输出到连续两个新RCA时，第二个RCA找不到源
									sourceRCA->targets().push_back(buffRCA);
									buffRCA->sources().push_back(sourceRCA);
									break;
								}
							}
						}
					}
				}
			}
			else if( (IsTempExternPort(portIter->dfgPort())) && !(portIter->IsInSameGroup()) )
			{	//对于外部临时节点的排布
				DFGNode * bypNode = config.newBypsNode();
				bypNode->addSource(portIter->dfgPort());
				bypNode->addTarget(portIter->dfgPort());
				if (i < 64)	buffRCA->node(i).setDFGNode(bypNode);
				else buffRCA->temp(i-64).setDFGNode(bypNode);

				int SeqNo = -1;
				String VarName = static_cast <DFGVarPort*>(portIter->dfgPort())->name();
				sscanf(VarName.c_str(),"%*[^0-9]%i",&SeqNo); //取得父节点的SeqNo
				for (rcaIter = thisRCA->sources ().begin();rcaIter != thisRCA->sources ().end();++rcaIter)
				{
					RCA * sourceRCA = * rcaIter;
					for (int rc=0;rc != RC_REG_NUM; ++rc)
					{
						RCANode * thisNode = &sourceRCA->node(rc);
						DFGNode * sourceDfgNode = thisNode->dfgNode();
						if (sourceDfgNode == 0) continue;
						if(sourceDfgNode->seqNo() ==SeqNo)		//找到父节点
						{
							Vector<RCA*>::iterator TgtIter = sourceRCA->targets().begin();
							for(;TgtIter != sourceRCA->targets().end();++TgtIter)
							{
								if(*TgtIter == thisRCA)
								{
									//sourceRCA->targets().erase(TgtIter); //erase不能在循环中做，否则在一个source输出到连续两个新RCA时，第二个RCA找不到源
									sourceRCA->targets().push_back(buffRCA);
									buffRCA->sources().push_back(sourceRCA);
									break;
								}
							}
						}
					}
				}
			}else
			{	//对于外部节点的排布
				DFGNode * fakeNode = config.newFakeNode();
				DFGPort * fakePort = config.insertPortOf(fakeNode);
				if(portIter->dfgPort()->isImmPort())
				{
					int Value = static_cast<DFGImmPort*>(portIter->dfgPort())->value();
					using namespace std;
					strstream ss;
					string s;
					ss << Value;
					ss >> s;
					static_cast<DFGVarPort*>(fakePort)->setName("_i_I" + s);
				}
				else
				{
					static_cast<DFGVarPort*>(fakePort)->setName("_i_E" + static_cast<DFGVarPort*>(portIter->dfgPort())->name());
				}
				fakeNode->addSource(portIter->dfgPort());
				fakeNode->addTarget(fakePort);
				if (i < 64) buffRCA->node(i).setDFGNode(fakeNode);
				else buffRCA->temp(i-64).setDFGNode(fakeNode);
				buffRCA->outports().push_back(fakePort);
				thisRCA->inports().push_back(fakePort);

				Vector<RCAPort>::iterator BuffOutIter = buffRCA->outports().begin();
				for( ;BuffOutIter != buffRCA->outports().end(); ++BuffOutIter )			//用新Port 替换BuffRCA原有的输出Port
				{
					if (BuffOutIter->dfgPort() != portIter->dfgPort()) continue;
					buffRCA->outports().erase(BuffOutIter);
					break;
				}

				Vector<RCAPort>::iterator InIter = thisRCA->inports().begin();			//用新Port 替换thisRCA原有的输入Port
				for( ;InIter != thisRCA->inports().end(); ++InIter)
				{
					if(InIter->dfgPort() != portIter->dfgPort()) continue;
					thisRCA->inports().erase(InIter);
					break;
				}

				if(!PreFlag)		//修改thisRCA的Node上的source关系，在premap时不启动，否则会造成map时source清空
				{
					for (int rc=0;rc != RC_REG_NUM * 2; ++rc)
					{
						RCANode * thisNode;
						DFGNode * Node;
						if(rc < 64)
						{
							thisNode = &thisRCA->node(rc);
							Node = thisNode->dfgNode();
						}
						else
						{
							thisNode = &thisRCA->temp(rc-64);
							Node = thisNode->dfgNode();
						}
						if (Node == 0) continue;
						Vector<DFGVex*> & source = Node->sources();
						Vector<DFGVex*>::iterator srcIter = Node->sources().begin();
						for( ;srcIter != Node->sources().end(); ++srcIter)
						{
							if(portIter->dfgPort()-> id() != (*srcIter)->id()) continue;
							/*Node->sources().erase(srcIter);
							Node->sources().push_back(static_cast <DFGVarPort*>(fakePort));
							*/
							/**srcIter = static_cast <DFGVarPort*>(fakePort);
							break;
						}
					
					}
				}
			}
		}
		Vector<RCA*> RCA_unique = buffRCA->sources();
		sort(RCA_unique.begin(),RCA_unique.end());
		buffRCA->sources().clear();
		buffRCA->sources().insert(buffRCA->sources().begin(),RCA_unique.begin(),unique(RCA_unique.begin(),RCA_unique.end()));

	}

	Vector<RCA*>::iterator SourceIter = thisRCA->sources().begin();
	for(;SourceIter != thisRCA->sources().end(); ++SourceIter)
	{
		RCA * sourceRCA = * SourceIter;
		Vector<RCA*>::iterator TgtIter = sourceRCA->targets().begin();
		for(;TgtIter != sourceRCA->targets().end(); ++TgtIter)
		{
			if(*TgtIter == thisRCA)	
			{
				sourceRCA->targets().erase(TgtIter); //erase操作单独处理
				break;
			}
		}
	}
	thisRCA->sources().clear();
	thisRCA->sources().push_back(buf1);
	if(buf2Enable)	thisRCA->sources().push_back(buf2);
	else 
	{
		rcas.erase(rcas.begin()+index+1);
		--remapSeqNo;
	}
	if(buf3Enable) thisRCA->sources().push_back(buf3);
	else
	{
		if(buf2Enable)
		{
			rcas.erase(rcas.begin()+index+2);
			--remapSeqNo;
		}
		else
		{
			rcas.erase(rcas.begin()+index+1);
			--remapSeqNo;
		}
	}
	return;
}
*/


