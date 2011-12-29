// Filename: genCL1info.cpp
//
// Author: Xie Li@ MicroE of SJTU
// Time: July 29th, 2010
// ======================================
// This file generate the infomation for 
// Cl1 configration.
//

#include "platf.h"
#include "cl1.h"
#include "rpucfg.h"
#include "rca.h"
#include "remap.h"

#include <iostream>
#include <fstream>

extern bool IsInnerPort(DFGPort * port);
extern bool IsTempExternPort (DFGPort *port);
	
extern void PortStyleSwitch(DFGPort * port);

// shedule state

#define STA_WAIT	0
#define STA_REDY	1
#define STA_OVER	2

//2011.5.11 liuxie
//#define BYTE_PER_LINE	8
//#define LINE_PER_CYCLE	4
#define DATA_PER_LINE	8     //for RIM specification
#define LINE_PER_CYCLE	2

#define BYTE_PER_LINE 16



// Basic routine
/////////////////////////////////////////////////////////////////////

static bool scheduleOver(const Vector<RCA*> & rcas){

	bool over = true;
	Vector<RCA*>::const_iterator rcaIter;
	for(rcaIter = rcas.begin(); rcaIter != rcas.end(); ++ rcaIter){
		
		if((*rcaIter)->state() != STA_OVER){

			over = false;
			break;
		}
	}

	return over;
}


static int areaBelongTo(int row, int col)    
{

	if(row < RIM_HEIGHT/2)
		return 0;
	else
		return 1;
}

///////////////////////////////////////////////////////////////////

int CL1Config::remainRCA(const Vector<RCA*> & rcas){

	int remain = 0;
	Vector<RCA*>::const_iterator rcaIter;
	for(rcaIter = rcas.begin(); rcaIter != rcas.end(); ++ rcaIter){

		if((*rcaIter)->state() != STA_OVER) ++ remain;
	}

	return remain;
}

Vector<RCA*> CL1Config::readyRCA(const Vector<RCA*> & rcas) {

	Vector<RCA*> readyRCAs;
	Vector<RCA*>::const_iterator rcaIter;
	for(rcaIter = rcas.begin(); rcaIter != rcas.end(); ++ rcaIter){

		RCA * thisRCA = *rcaIter;
		if(thisRCA->state() == STA_REDY) 
			{
			readyRCAs.push_back(thisRCA);
			}
	}
	
	return readyRCAs;
}

void CL1Config::updateRCAState(const Vector<RCA*> & rcas , int & ScheduleFlaseFlag,int & remainRCANumVar)
{

	Vector<RCA*>::const_iterator rcaIter = rcas.begin();
	//static int RemainRCANum = 0;
	
	for(; rcaIter != rcas.end(); ++ rcaIter)
	{

		RCA * thisRCA = *rcaIter;

		assert(thisRCA != 0);
		
		if(thisRCA->state() == STA_OVER) 
			{ 
				continue;
			}
		
		const Vector<RCA*> & srcRCAs = thisRCA->sources();
		
		bool thisRCAReady = true;
		Vector<RCA*>::const_iterator srcRCAIter;
		
		for(srcRCAIter = srcRCAs.begin();srcRCAIter != srcRCAs.end(); ++ srcRCAIter)
		{

			RCA * thisSrcRCA = *srcRCAIter;
			
			if(thisSrcRCA->state() != STA_OVER)
			{
				thisRCAReady = false;
				break;
			}
		}

		if(thisRCAReady) 	thisRCA->setState(STA_REDY);
	}


	if( remainRCANumVar != remainRCA(rcas) )
		remainRCANumVar = remainRCA(rcas);
	else
		ScheduleFlaseFlag = 0;    //all the readyRCAs are mapped fail
}

//longlee
const Vector<CL1Block> CL1Config::PreMapRCA(Vector<RCA*> rcas,Vector<RCA*> &tmpGrpRCA,Vector<RCA*> &RCAS,RPUConfig & config)
{
	Vector<CL1Block> mapBlocks;

	Vector<RCA*>::iterator rcaIter;
	for(rcaIter = rcas.begin(); rcaIter != rcas.end(); ++ rcaIter)
	{

		RCA * thisRCA = * rcaIter;

		assert(thisRCA != 0);

		Vector<RCAPort> & rcaOutport = thisRCA->outports();


		/* Count the output inner ports and external ports */
		int totalInternPort = 0, totalExternPort = 0, totalTempExternPort = 0;
		Vector<RCAPort>::iterator portIter;

		//ͳ���ڲ��ڵ���ⲿ�ڵ�
		for(portIter = rcaOutport.begin();portIter != rcaOutport.end(); ++ portIter) 
		{

			(IsInnerPort(portIter->dfgPort()) || IsTempExternPort(portIter->dfgPort()))?  
				++ totalInternPort : ++ totalExternPort;
		}

		
		//��DFGͼ������ڵ��ǣ���ֹ���ֱ��ĺ�ʶ���Extern Temp Port
		for(portIter = rcaOutport.begin();portIter != rcaOutport.end(); ++ portIter) 
		{

			if(!((IsInnerPort(portIter->dfgPort()) || IsTempExternPort(portIter->dfgPort()))))  
			{
				portIter->setDFGExternPort(true);
			}
			else
				portIter->setDFGExternPort(false);

		}

		/* Allocate the RIM for CDS */
		//2011.7.19 longlee������RIM״̬���ݣ�������RIFԽ�����ʱ��RIM���ݻָ�mapRCA֮ǰ��״̬
		CL1RIM RIM_backup;		
		Vector<RCAPort*> tempPortInRIM_backup,outPortInRIM_backup;
		Vector<RCAPort*>::iterator tempPortInRIMIter,outPortInRIMIter;
		for (tempPortInRIMIter = tempPortInRIM.begin(); tempPortInRIMIter != tempPortInRIM.end(); ++tempPortInRIMIter)
		{
			tempPortInRIM_backup.push_back(*tempPortInRIMIter);
		}

		for (outPortInRIMIter = outPortInRIM.begin(); outPortInRIMIter != outPortInRIM.end(); ++outPortInRIMIter)
		{
			outPortInRIM_backup.push_back(*outPortInRIMIter);
		}

		//longlee:20111005
		Vector<int> tempAreaCounter_backup = tempAreaCounter;
		Vector<int> outAreaCounter_backup = outAreaCounter;
		
		RIM_backup.copy(RIM);
		CL1Data CDSData = RIM.allocate(thisRCA->seqNo(), totalInternPort, totalExternPort, thisRCA->getRemapFlag());


		if(CDSData.baseAddress() == -1) 
		{ // Allocation fail
			if(thisRCA->getRemapFlag())
			{
				Vector<RCA*>::iterator thisIter = rcas.begin();
				for (;thisIter != rcas.end() ; ++thisIter)
				{
					(*thisIter)->setMappedFlag(false);
					(*thisIter)->setState(STA_OVER);
				}
				break;
				//continue;
			}
			else
			{
				thisRCA->setMappedFlag(false);
				continue;
			}
		}
		/////////////////////////////////////////////////////////////////////////////////
		//
		// Mark the ROF and RIM location of port, meanwhile record the output data.
		//------------------------------------------------------------------------------

		int outRegionIndex = 0, tempRegionIndex = 0;
		for(portIter = rcaOutport.begin();portIter != rcaOutport.end(); ++ portIter) 
		{
 
			if(IsInnerPort(portIter->dfgPort()))
			{ //It is an internal port

				//2011.6.9 liuxie
				portIter->setROFRow(tempRegionIndex / TEMP_REGION_WIDTH);
				int a_temp = tempRegionIndex / TEMP_REGION_WIDTH;
				portIter->setRIMRow(CDSData.baseAddress() + a_temp);

				//2011.5.11 liuxie
				portIter->setROFCol(tempRegionIndex % TEMP_REGION_WIDTH);
				int b_temp = tempRegionIndex % TEMP_REGION_WIDTH;
				portIter->setRIMCol(b_temp *2);

				++ tempRegionIndex;
			
				tempAreaCounter[(portIter->RIMRow() < RIM_HEIGHT/2) ? 0 : 1] ++;
				
				tempPortInRIM.push_back(&(*portIter));
			
			} 
			else if(!(IsTempExternPort(portIter->dfgPort())))
			{  //It is an external port

				portIter->setROFRow(outRegionIndex % CDSData.height());			
		
				portIter->setRIMRow(CDSData.baseAddress() + portIter->ROFRow());
				//2011.5.11 liuxie
				portIter->setROFCol(CDSData.length() - outRegionIndex / CDSData.height() - 1);
				portIter->setRIMCol(CDSData.offset() + portIter->ROFCol()*2);

				++ outRegionIndex;
			
				/*****************************************************************************************/
				outAreaCounter[areaBelongTo(portIter->RIMRow(), portIter->RIMCol())] ++;

				outPortInRIM.push_back(&(*portIter));
				/*****************************************************************************************/

			}
			else  //it's a external temp port,�����������ڴ˴���û�н�������ת���������������û������
			{
				//2011.6.9  liuxie   //�˴��Ѿ���TEMP_REGION_WIDTH�Ŀ�����ó�8��TEMP_REGION_WIDTH_BYTE = 16 bytes��
				portIter->setROFRow(tempRegionIndex / TEMP_REGION_WIDTH);
				int c_temp = tempRegionIndex / TEMP_REGION_WIDTH;
				portIter->setRIMRow(CDSData.baseAddress() + c_temp);

				//2011.5.11 liuxie
				portIter->setROFCol(tempRegionIndex % TEMP_REGION_WIDTH);
				int d_temp = tempRegionIndex % TEMP_REGION_WIDTH;
				portIter->setRIMCol(d_temp * 2);

				++ tempRegionIndex;
			
				tempAreaCounter[(portIter->RIMRow() < RIM_HEIGHT/2) ? 0 : 1] ++;
				
				tempPortInRIM.push_back(&(*portIter));
			}
		}

		// Mark the input ports ,which will be transfered into RIF.
		//----------------------------------------------------------

		Vector<RCAPort> & rcaInport = thisRCA->inports();

		//ͳ�Ƶ�ǰRCA�������е�tempPort�Ӽ�������ssram��������������

		Vector<RCA*> & currentRCASource = thisRCA->sources();//��ǰRCA��ԴRCA
		int numberOfSection;
		numberOfSection = currentRCASource.size();            //���ж��ٸ���RCA

		List<List<int> > tempDataBlockList(numberOfSection);

		List<List<int> >::iterator listIterTemp;

		for(portIter = rcaInport.begin();portIter != rcaInport.end(); ++ portIter) 
		{
						
			DFGPort * port = portIter->dfgPort();
			assert( port!=0 );
				
			if( IsTempExternPort( portIter->dfgPort()) && !(portIter->IsInSameGroup()) ) 
			{	
				//�������tempExternPort�Ǵ��ĸ�ԴRCA���ģ�

				for(unsigned int counterSourceRCA_i=0; counterSourceRCA_i < currentRCASource.size(); counterSourceRCA_i ++ )
				{
					Vector<RCAPort>::iterator sourceRCAOutportsIter = currentRCASource[counterSourceRCA_i]->outports().begin();
					Vector<RCAPort>::iterator sourceRCAOutportsIterEnd = currentRCASource[counterSourceRCA_i]->outports().end();

					for( ; sourceRCAOutportsIter!=sourceRCAOutportsIterEnd; ++ sourceRCAOutportsIter)
					{
						if(sourceRCAOutportsIter->dfgPort() == portIter->dfgPort())
						{
							listIterTemp=tempDataBlockList.begin();
							for(unsigned int tempI=0;tempI<counterSourceRCA_i;tempI++)
								listIterTemp++;

							listIterTemp->push_back(port->SSRAMAddress());
							listIterTemp->sort();//sorting

							break;
						}
					}
				}
			}
		}

		thisRCA->setTempDataBlockList(tempDataBlockList);//�ݴ��list�����ں�������REDL;


		// Count the input inner ports and external ports
		totalInternPort = 0;
		totalExternPort = 0;
		

		Vector<RCAPort*>::reverse_iterator r_tempPortInRIMIter;
		//tempRIMBaseRow & tempRIMTopRow ������¼��ǰRCA�����е��ڲ��˿���RIM temp�����л���ַ�Ͷ���ַ��������һ������ҪCIDL������RIF�С�
		int tempRIMBaseRow;
		int tempRIMTopRow;

		tempRIMBaseRow = RIM_HEIGHT;
		tempRIMTopRow  = 0;

		int MaxPortSSRAMAddress = 0;                //���externTemp Port
		int MinPortSSRAMAddress = 0x800;            //���externTemp Port

		int MaxExPortNo = -1;              //���Extern Port
		int MinExPortNo = 10000;          //���Extern Port


		for(portIter = rcaInport.begin();portIter != rcaInport.end(); ++ portIter) 
		{
						
				DFGPort * port = portIter->dfgPort();
				assert( port!=0 );
				
				// for inner port
				if( IsInnerPort( portIter->dfgPort() ) || ((IsTempExternPort(portIter->dfgPort())) && portIter->IsInSameGroup()))
				{
					++ totalInternPort ;

					//���������ڲ��˿ڣ������Ҫ���ҳ���Щ�˿���RIM��temp����ķ�Χ���Ա�CIDL���뵽RIF�У�ͬʱҲΪ�˷���thisRCA��RIF��λ��
					//����remap�¼����RCA����RIM��Ӧ��2����ͬ��inport����һ��Ϊ��ʵ��Ҫ��inport���ڶ���ΪRCA�Լ�����������������ȡ��һ��
					if(thisRCA->getRemapFlag())		
					{
						for(tempPortInRIMIter = tempPortInRIM.begin(); tempPortInRIMIter != tempPortInRIM.end(); tempPortInRIMIter ++ )
						{
							if((*tempPortInRIMIter)->dfgPort() == portIter->dfgPort())
							{
								if( tempRIMBaseRow > ((*tempPortInRIMIter)->RIMRow()))
									tempRIMBaseRow = (*tempPortInRIMIter)->RIMRow();

								if( tempRIMTopRow < ((*tempPortInRIMIter)->RIMRow()))
									tempRIMTopRow = (*tempPortInRIMIter)->RIMRow();

								break;
							}
						}
					}
					//����������RCA��Ӧ��Ҳ��2����ͬ��inport��RIM�У�Ӧ���������ȡ��RCA�����������remap����û��Ч��
					else
					{
						for(r_tempPortInRIMIter = tempPortInRIM.rbegin(); r_tempPortInRIMIter != tempPortInRIM.rend(); r_tempPortInRIMIter ++ )
						{
							if((*r_tempPortInRIMIter)->dfgPort() == portIter->dfgPort())
							{
								if( tempRIMBaseRow > ((*r_tempPortInRIMIter)->RIMRow()))
									tempRIMBaseRow = (*r_tempPortInRIMIter)->RIMRow();

								if( tempRIMTopRow < ((*r_tempPortInRIMIter)->RIMRow()))
									tempRIMTopRow = (*r_tempPortInRIMIter)->RIMRow();

								break;
							}
						}
					}
					
				}
                //for external port or extern temp port
				else 
				{
					if( IsTempExternPort( portIter->dfgPort()) && (!(portIter->IsInSameGroup())))   //external temp port
					{					
						totalTempExternPort++;

						if( port->SSRAMAddress() > MaxPortSSRAMAddress)
							MaxPortSSRAMAddress = port->SSRAMAddress();	

						if( port->SSRAMAddress() < MinPortSSRAMAddress)
							MinPortSSRAMAddress = port->SSRAMAddress();	
					} 
					else //external input port
					{
						++ totalExternPort;
						if( port->seqNo() > MaxExPortNo)
							MaxExPortNo = port->seqNo();	

						if( port->seqNo() <  MinExPortNo)
							 MinExPortNo = port->seqNo();	
					}
				}
		}

		std::cout<<"tempRIMBaseRow = "<<tempRIMBaseRow<<std::endl;
		std::cout<<"tempRIMTopRow = "<<tempRIMTopRow<<std::endl;

		std::cout<<"MaxPortSSRAMAddress = "<<MaxPortSSRAMAddress<<std::endl;
		std::cout<<"MinPortSSRAMAddress = "<<MinPortSSRAMAddress<<std::endl;
				
		std::cout<<"MaxExPortNo = "<<MaxExPortNo<<std::endl;
		std::cout<<"MinExPortNo = "<<MinExPortNo<<std::endl;


		
		int externFIFOIndex = 0;

		if(MinExPortNo == 10000)
			MinExPortNo = -1;

		int externInportSize;

		if(MaxExPortNo >= 0 && MinExPortNo>=0)
			externInportSize = MaxExPortNo- 0 + 1;  //������Ļ���ַȡֵ
		else
			externInportSize = 0;

		
		//�����ⲿֱ�����������
		//2011.5.11 liuxie
		//const int externalRowNum = (totalExternPort/FIFO_WIDTH_DATA + ((totalExternPort % FIFO_WIDTH_DATA)? 1:0));
		const int externalRowNum = (externInportSize/FIFO_WIDTH_DATA + ((externInportSize % FIFO_WIDTH_DATA)? 1:0));

		//�����ⲿtemp��������� ����temp Extern Portռ�õ���С�ռ�
		int externalTempRow;
		externalTempRow=0;		
		
		int ExterntempSSRAMBaseAddr = 816;
		int ExterntempSSRAMTopAddr = 192;
		
		if( totalTempExternPort !=0 )
		{
			List<List<int> >::iterator listIter =tempDataBlockList.begin();
			//List<List<int> >::iterator listIter2 =tempDataBlockList.begin();
			List<int>::iterator listIntIter;
			//List<int>::iterator listIntIter2;

            //for extern temp inport
			for( ; listIter !=tempDataBlockList.end(); listIter++)
			{
				for(listIntIter = (*listIter).begin() ; listIntIter != (*listIter).end(); listIntIter++)
				{
					if((*listIntIter) < ExterntempSSRAMBaseAddr)
						ExterntempSSRAMBaseAddr = (*listIntIter);
					if(ExterntempSSRAMTopAddr < (*listIntIter))
						ExterntempSSRAMTopAddr = (*listIntIter);
				}
			}

			//2011.6.17 liuxie
			//int Addrdelta = ExterntempSSRAMTopAddr - ExterntempSSRAMBaseAddr;
			ExterntempSSRAMBaseAddr = (ExterntempSSRAMBaseAddr/FIFO_WIDTH) * FIFO_WIDTH;
			int Addrdelta = ExterntempSSRAMTopAddr - ExterntempSSRAMBaseAddr;
			assert(ExterntempSSRAMTopAddr >= 192);
			std::cout<<"FixedMaxPortSSRAMAddress = "<<ExterntempSSRAMTopAddr<<std::endl;
			std::cout<<"FixedMinPortSSRAMAddress = "<<ExterntempSSRAMBaseAddr<<std::endl;
			//int Addrdelta = ExterntempSSRAMTopAddr  - 768;

			Addrdelta = Addrdelta/2 + 1;
			
			externalTempRow = Addrdelta/FIFO_WIDTH_DATA + ((Addrdelta%FIFO_WIDTH_DATA)?1:0);
		}

		const int externalTempRowNum = externalTempRow;

		const int externDataHeight = externalRowNum +  externalTempRowNum;


		int internFIFOIndexBase = externDataHeight * FIFO_WIDTH_DATA ;
		//internFIFOIndexBase ���ⲿ����ĺ��棬��ʾ��������ط���ʼ���ڲ����룻����ע�����ֵ�ı仯����ΪCIDL��CEDL����ִ�С�

		int internFIFOIndex=0;
		int externTempFIFOIndex;
		int farestexterndata = 0;        //externdata��SSRAM����Զ��ַ����externdata����ʼ��ַ�� 0x00��
		int faresttempexterndata = 0;    //tempexterndata��SSRAM����Զ��ַ����tempexterndata����ʼ��ַ ��ַδ���壩

		std::cout<<"The tempRIMBaseRow is "<<tempRIMBaseRow<<std::endl;
		std::cout<<"The tempRIMTopRow is "<<tempRIMTopRow<<std::endl;
		
		for(portIter = rcaInport.begin();portIter != rcaInport.end(); ++ portIter) 
		{

			std:: cout<<"RCA SEQNo is "<<thisRCA->seqNo()<<std::endl;

			if( IsInnerPort(portIter->dfgPort()) || ((IsTempExternPort(portIter->dfgPort())) && portIter->IsInSameGroup()))
			{   // It's an internal port

				//���������ڲ��˿ڣ������Ҫ���ҳ���Щ�˿���RIM��temp����ķ�Χ���Ա�CIDL���뵽RIF�У�ͬʱҲΪ�˷���thisRCA��RIF��λ��
				if(thisRCA->getRemapFlag())		//remapRCA��RIM�˿��������
				{
					for(tempPortInRIMIter = tempPortInRIM.begin(); tempPortInRIMIter != tempPortInRIM.end(); tempPortInRIMIter ++ )
					{
						if((*tempPortInRIMIter)->dfgPort() == portIter->dfgPort())
						{
							//����thisRCA������ڲ��˿ڣ�ͨ��CIDL���غ���RIF��λ�á�
							internFIFOIndex = internFIFOIndexBase + (((*tempPortInRIMIter)->RIMRow()) - tempRIMBaseRow)*TEMP_REGION_WIDTH + ((*tempPortInRIMIter)->RIMCol()/2);
							break;
						}
					}
				}
				else		//��remapRCA��RIM�˿��������
				{
					for(r_tempPortInRIMIter = tempPortInRIM.rbegin(); r_tempPortInRIMIter != tempPortInRIM.rend(); r_tempPortInRIMIter ++ )
					{
						if((*r_tempPortInRIMIter)->dfgPort() == portIter->dfgPort())
						{
							//����thisRCA������ڲ��˿ڣ�ͨ��CIDL���غ���RIF��λ�á�
							internFIFOIndex = internFIFOIndexBase + (((*r_tempPortInRIMIter)->RIMRow()) - tempRIMBaseRow)*TEMP_REGION_WIDTH + ((*r_tempPortInRIMIter)->RIMCol()/2);
							break;
						}
					}
				}

				//2011.6.9  liuxie
				portIter->setRIFRow(internFIFOIndex / FIFO_WIDTH_DATA);
				portIter->setRIFCol(internFIFOIndex % FIFO_WIDTH_DATA);
			} 
			else 
			{ // It's an external port or a temp external port
			
				int PortSSRAMAddress = portIter->dfgPort()->SSRAMAddress();    //for external temp data
				
				/* SSRAM memory allocation */
				//����Extern Port���ݵ�SSRAM allocation
				if( !(IsTempExternPort(portIter->dfgPort())) ) 
				{   //It's an external port
					//2011.4.22 liuxie
					//REDL��ȫ˳����루��ѡ�񣩣�CIDL��ȫ˳����루��ѡ�񣩣���ѡ������Ӧ��ֵ
					//�磺��deblocking�ļ򻯺����У���һ��RCA����13���ⲿport��˳���Ѿ���DFG�ⲿ�����17�����벻һ�������ܵ�����һ�����ض���RIF
                    int DFGInportSeq;
					DFGInportSeq = portIter->dfgPort()->seqNo();
					                  	
					//20110615 liuxie
					//portIter->setRIFRow( externFIFOIndex / FIFO_WIDTH_DATA);
					//2011.5.11 liuxie for 16bit data
					//portIter->setRIFCol(externFIFOIndex % FIFO_WIDTH_DATA);
					portIter->setRIFRow( DFGInportSeq / FIFO_WIDTH_DATA);
					portIter->setRIFCol( DFGInportSeq % FIFO_WIDTH_DATA);
					
                    //2011.5.11 liuxie  ����λ��Ϊ16bit
					portIter->dfgPort()->setSSRAMAddress(SSRAMInBaseAddr + DFGInportSeq*2);
				
					++ externFIFOIndex;
				} 
				else 
				{  // It's a temp external port
					//�ҵ�����˿�λ������
					/*List<List<int> >::iterator tempListIter =tempDataBlockList.begin();
					List<int>::iterator tempListIntIter;

					bool findFlag;
					findFlag =false;

					int thisPortLocalRow;
					thisPortLocalRow =0;//init

					int deltaValue;//ƫ����
					deltaValue=0;

					for( ; tempListIter !=tempDataBlockList.end(); ++ tempListIter)
					{
						if(tempListIter->size())
						{
							tempListIntIter = (*tempListIter).begin();

							for( ; tempListIntIter != (*tempListIter).end(); ++tempListIntIter)
							{
								if((portIter->dfgPort()->SSRAMAddress())== *tempListIntIter)
								{
									thisPortLocalRow += ((*tempListIntIter)/FIFO_WIDTH - (*((*tempListIter).begin()))/FIFO_WIDTH);

									deltaValue = (*tempListIntIter) % FIFO_WIDTH;

									findFlag=true;
									break;
								}
							}

							if(findFlag) break;

							thisPortLocalRow = (*(--(*tempListIter).end()))/FIFO_WIDTH - (*(*tempListIter).begin())/FIFO_WIDTH;
						}
					}

					externTempFIFOIndex = (externalRowNum + thisPortLocalRow) * FIFO_WIDTH + deltaValue;

					portIter->setRIFRow( externTempFIFOIndex / FIFO_WIDTH_DATA );
					portIter->setRIFCol( externTempFIFOIndex % FIFO_WIDTH_DATA );
					*/
					List<List<int> >::iterator tempListIter =tempDataBlockList.begin();
					List<int>::iterator tempListIntIter;

					bool findFlag;
					findFlag =false;

					int tempExPortSeqNo = -1;
					int deltaValue;

					int thisPortLocalRow;

					for( ; tempListIter !=tempDataBlockList.end(); ++ tempListIter)
					{
						if(tempListIter->size())
						{
							tempListIntIter = (*tempListIter).begin();

							for( ; tempListIntIter != (*tempListIter).end(); ++tempListIntIter)
							{
								if((portIter->dfgPort()->SSRAMAddress())== *tempListIntIter)
								{
									
									//tempExPortSeqNo = (portIter->dfgPort()->SSRAMAddress() - 768) / 2;
									tempExPortSeqNo = (portIter->dfgPort()->SSRAMAddress() - ExterntempSSRAMBaseAddr) / 2;
									findFlag=true;
									break;
								}
							}

							if(!findFlag) continue;;

							deltaValue = tempExPortSeqNo % FIFO_WIDTH_DATA;

							thisPortLocalRow = (tempExPortSeqNo) / FIFO_WIDTH_DATA + ((tempExPortSeqNo % FIFO_WIDTH_DATA?1:0));
							if (thisPortLocalRow == 0) thisPortLocalRow = 1;

							break;
						}
					}


					externTempFIFOIndex = (externalRowNum + thisPortLocalRow - 1) * FIFO_WIDTH_DATA + deltaValue;

					portIter->setRIFRow( externTempFIFOIndex / FIFO_WIDTH_DATA );
					portIter->setRIFCol( externTempFIFOIndex % FIFO_WIDTH_DATA );

				}
				
			}
		}
		
		//2011.7.19 longlee RIFԽ���鼰���Ų���
        int maxRIFRow = 0;
		bool remapFlag = false;
		for(portIter = rcaInport.begin();portIter != rcaInport.end(); ++ portIter) 
		{
			maxRIFRow = (portIter->RIFRow() > maxRIFRow)? portIter->RIFRow():maxRIFRow;
			if ( maxRIFRow > 7) 
			{
				remapFlag = true;	//˵����Ҫ����remap;
				//RIM״̬�ع�
				RIM.copy(RIM_backup);
				tempPortInRIM.clear();
				outPortInRIM.clear();
				for (tempPortInRIMIter = tempPortInRIM_backup.begin(); tempPortInRIMIter != tempPortInRIM_backup.end(); ++tempPortInRIMIter)
				{
					tempPortInRIM.push_back(*tempPortInRIMIter);
				}
				for (outPortInRIMIter = outPortInRIM_backup.begin(); outPortInRIMIter != outPortInRIM_backup.end(); ++outPortInRIMIter)
				{
					outPortInRIM.push_back(*outPortInRIMIter);
				}

				//longlee:20111005
				tempAreaCounter = tempAreaCounter_backup;
				outAreaCounter = outAreaCounter_backup;

				//��ȡ��ǰRCA������λ��
				Vector<RCA*>::iterator RCAIter;
				int index = 0;
				for (RCAIter = RCAS.begin();RCAIter != RCAS.end(); RCAIter++,index ++)
				{
					if(*RCAIter != *rcaIter) continue;
					else break;
				}
				//�����Ų�����RCA���޸Ķ˿ڹ�ϵ
				//int index = (*RCAIter)->seqNo();
				remap(RCAS,index,tempPortInRIM,config,1);
				//��ǵ�ǰRCAmapʧ�ܣ�����
				break;
			}
		}

		if (remapFlag) 
		{
			thisRCA->setMappedFlag(false);
			CL1Block RemapBlock;
			RemapBlock.setRemapFlag(true);
			mapBlocks.push_back(RemapBlock);
			return mapBlocks;
		}

		//******************************************start**********************************************************************
		
		//����ֱ�Ӵ�Ƭ��SSRAM����Ļ���ַΪ0������Ҫ�������е�inport�ҵ������ַ��Զ���Ǹ�port��λ��Ϊtop address�ṩ��REDL
		Vector<RCAPort>::iterator RCAInportIter;
		for( RCAInportIter = rcaInport.begin(); RCAInportIter != rcaInport.end(); ++  RCAInportIter)
		{
			    DFGPort * port = RCAInportIter->dfgPort();
				assert( port!=0 );
			    // for inner port
				if( IsInnerPort( RCAInportIter->dfgPort() ) )
					continue;
				else if(IsTempExternPort( RCAInportIter->dfgPort()) && RCAInportIter->IsInSameGroup())
					continue;
				else //external temp data
				{
					if( IsTempExternPort( RCAInportIter->dfgPort()) )   //external temp port
					{
						if(port->SSRAMAddress() > faresttempexterndata)
							faresttempexterndata = port->SSRAMAddress();
					}
					else
					{    //external input port					
						 if(port->SSRAMAddress() > farestexterndata)
							farestexterndata=port->SSRAMAddress();     //ָ����Զ���Ǹ�data��SSRAM�е���ʼλ��
					}
				}
		}
		//******************************************end**********************************************************************

		
		//2011.5.28 liuxie for extern input Port		
		thisRCA->setRCASSRAMInBaseAddr(0);//���õ�ǰRCA�ⲿֱ������Ļ���ַ		
        
        if(farestexterndata!=0)
			SSRAMInTopAddr = (farestexterndata/FIFO_WIDTH + ((farestexterndata+1)%FIFO_WIDTH?1:0))*FIFO_WIDTH;
		else
			SSRAMInTopAddr = (farestexterndata/FIFO_WIDTH + ((farestexterndata)%FIFO_WIDTH?1:0))*FIFO_WIDTH;
		//2011.5.11 liuxie
		thisRCA->setRCASSRAMInTopAddr(SSRAMInTopAddr);

		//2011.5.28 liuxie for ExternTemp input Port
		thisRCA->setRCASSRAMTempInBaseAddr((ExterntempSSRAMBaseAddr/FIFO_WIDTH)*FIFO_WIDTH);

		if(faresttempexterndata >= 192)
			SSRAMTempInTopAddr = (faresttempexterndata/FIFO_WIDTH + ((faresttempexterndata+1)%FIFO_WIDTH?1:0))*FIFO_WIDTH;
		else
			SSRAMTempInTopAddr = (faresttempexterndata/FIFO_WIDTH + ((faresttempexterndata)%FIFO_WIDTH?1:0))*FIFO_WIDTH;

		thisRCA->setRCASSRAMTempInTopAddr(SSRAMTempInTopAddr);

		//////////////////////////////////////////////////////////////////
        // Map the block data

		CL1Block block;

		// Set CEDL
		block.CEDLData().setTarget(CEDL_TGT_RIF);         //CEDL��Ŀ�������RIFҲ������RIM
		block.CEDLData().setHeight(externDataHeight);	

		// Set CIDL
		//--------------------------------------------
		//  CIDL need to Find all output data of source 
		//  RCA in temperary data region of RIM, and 
		//  connect them to one 2D data. That need to 
		//  find the lowest base address and height of
		//  the end address first.

		int lowestBaseAddr = RIM_HEIGHT;    //��ǰrca��source rca��RIM�е���͵�ַ
		int highestEndAddr = 0;             //��ǰrca��source rca��RIM�е���ߵ�ַ

		Vector<RCA*>::iterator srcIter;
		Vector<RCA*> & thisRCASource = thisRCA->sources();

		Vector<CL1Block>::iterator blockMappedIter;

		Vector<RCA*>::iterator tmpGrpRCAIter;//yin0831
		bool CIDLEnableValue;
		CIDLEnableValue=false;

		for(srcIter = thisRCASource.begin();srcIter != thisRCASource.end(); ++ srcIter)
		{
			
			for(tmpGrpRCAIter = tmpGrpRCA.begin();tmpGrpRCAIter != tmpGrpRCA.end();++tmpGrpRCAIter)
			{
				if(((*srcIter)->seqNo()) == ((*tmpGrpRCAIter)->seqNo()))//yin0831 ��ͬһgroup���Ҫ����cidl������
				{
					CIDLEnableValue=true;//Setting CIDLEnableValue

					std::cout<<"rca seqNo = "<<(*srcIter)->seqNo()<<std::endl;
					int srcRCABaseAddr = RIM.getBaseAddress((*srcIter)->seqNo());
					int srcRCAEndAddr = RIM.getEndAddress((*srcIter)->seqNo());

					if(srcRCABaseAddr < lowestBaseAddr) 
						lowestBaseAddr = srcRCABaseAddr;

					if(highestEndAddr < srcRCAEndAddr) 
						highestEndAddr = srcRCAEndAddr;
				}
			}
		}
		std::cout<<"tempRIMBaseRow is "<<tempRIMBaseRow<<std::endl;
		std::cout<<"tempRIMTopRow is "<<tempRIMTopRow<<std::endl;
		std::cout<<"lowestBaseAddr is "<<lowestBaseAddr<<std::endl;
		std::cout<<"highestEndAddr is "<<highestEndAddr<<std::endl<<std::endl;
		
		
		
		////2011.5.11 liuxie ��CIDL��дģʽ��4��8byteƴ�� �޸ĳ�Ϊ 2��16Byteƴ��
		lowestBaseAddr = tempRIMBaseRow;
		highestEndAddr = tempRIMTopRow + 1;//��������(�߶�)
		//yin0909end

		block.CIDLData().setInputMode(MODE_IN_V2D);    //2D����ģʽ,�׵�ַ�ɱ�

		block.CIDLData().setBaseAddress(lowestBaseAddr); 
		//2011.5.11 liuxie
        block.CIDLData().setLength(TEMP_REGION_WIDTH_BYTE); //ȫ����16ByteΪ����д��
		//setHeight 2011.4.25 liuxie
		//////////////////////////////////////////////////////////////////
		block.CIDLData().setHeight(highestEndAddr - lowestBaseAddr);
		//////////////////////////////////////////////////////////////////
		//2011.5.11 liuxie
		block.CIDLData().setOutputMode(MODE_OUT_2L);    //����2D����ÿ��2�ȷ֣��������ƴ�����
		block.CIDLData().setOffset(0);                

		// Set CDS
		//--------------------------------------------

		CDSData.setTarget(CDS_TGT_RIM);
		block.setCDSData(CDSData);

		// Set Core

		block.setRCAIndex(thisRCA->seqNo());
		block.setRCACoreLoop(1);
		block.setConst1Address(0);
		block.setConst2Address(0);
		block.setRIDLEnable(false);

		bool CEDLEnable = externDataHeight? true : false;
		
		// setting the CIDLEnable value
		bool CIDLEnable = CIDLEnableValue;

		block.setCEDLEnable(CEDLEnable);
		block.setCIDLEnable(CIDLEnable);

		mapBlocks.push_back(block);
		thisRCA->setState(STA_OVER);
		thisRCA->setMappedFlag(true);

		tmpGrpRCA.push_back(thisRCA);		
		
	}

	return mapBlocks;
}

//2011.4.20 liuxie mapRCA�����涨��RIF��RIM�ķ���ԭ��
Vector<CL1Block> CL1Config::mapRCA(Vector<RCA*> rcas,Vector<RCA*> &tmpGrpRCA,Vector<RCA*> &RCAS,RPUConfig & config)
{
	Vector<CL1Block> mapBlocks;

	Vector<RCA*>::iterator rcaIter;
	for(rcaIter = rcas.begin(); rcaIter != rcas.end(); ++ rcaIter)
	{

		RCA * thisRCA = * rcaIter;
		assert(thisRCA != 0);

		Vector<RCAPort> & rcaOutport = thisRCA->outports();


		/* Count the output inner ports and external ports */
		int totalInternPort = 0, totalExternPort = 0, totalTempExternPort = 0;
		Vector<RCAPort>::iterator portIter;

		//ͳ���ڲ��ڵ���ⲿ�ڵ�
		for(portIter = rcaOutport.begin();portIter != rcaOutport.end(); ++ portIter) 
		{

			(IsInnerPort(portIter->dfgPort()) || IsTempExternPort(portIter->dfgPort()))?  
				++ totalInternPort : ++ totalExternPort;
		}

		
		//��DFGͼ������ڵ��ǣ���ֹ���ֱ��ĺ�ʶ���Extern Temp Port
		for(portIter = rcaOutport.begin();portIter != rcaOutport.end(); ++ portIter) 
		{

			if(!((IsInnerPort(portIter->dfgPort()) || IsTempExternPort(portIter->dfgPort()))))  
			{
				portIter->setDFGExternPort(true);
			}
			else
				portIter->setDFGExternPort(false);

		}

		/* Allocate the RIM for CDS */
		//2011.7.19 longlee������RIM״̬���ݣ�������RIFԽ�����ʱ��RIM���ݻָ�mapRCA֮ǰ��״̬
		CL1RIM RIM_backup;		
		Vector<RCAPort*> tempPortInRIM_backup,outPortInRIM_backup;
		Vector<RCAPort*>::iterator tempPortInRIMIter,outPortInRIMIter;
		for (tempPortInRIMIter = tempPortInRIM.begin(); tempPortInRIMIter != tempPortInRIM.end(); ++tempPortInRIMIter)
		{
			tempPortInRIM_backup.push_back(*tempPortInRIMIter);
		}

		for (outPortInRIMIter = outPortInRIM.begin(); outPortInRIMIter != outPortInRIM.end(); ++outPortInRIMIter)
		{
			outPortInRIM_backup.push_back(*outPortInRIMIter);
		}

		//longlee:20111005
		Vector<int> tempAreaCounter_backup = tempAreaCounter;
		Vector<int> outAreaCounter_backup = outAreaCounter;
		
		RIM_backup.copy(RIM);
		CL1Data CDSData = RIM.allocate(thisRCA->seqNo(), totalInternPort, totalExternPort, thisRCA->getRemapFlag());


 		if(CDSData.baseAddress() == -1) 
		{ // Allocation fail
			thisRCA->setMappedFlag(false);
			continue;

		}
		
		/////////////////////////////////////////////////////////////////////////////////
		//
		// Mark the ROF and RIM location of port, meanwhile record the output data.
		//------------------------------------------------------------------------------

		int outRegionIndex = 0, tempRegionIndex = 0;
		for(portIter = rcaOutport.begin();portIter != rcaOutport.end(); ++ portIter) 
		{
 
			if(IsInnerPort(portIter->dfgPort()))
			{ //It is an internal port

				//2011.6.9 liuxie
				portIter->setROFRow(tempRegionIndex / TEMP_REGION_WIDTH);
				int a_temp = tempRegionIndex / TEMP_REGION_WIDTH;
				portIter->setRIMRow(CDSData.baseAddress() + a_temp);

				//2011.5.11 liuxie
				portIter->setROFCol(tempRegionIndex % TEMP_REGION_WIDTH);
				int b_temp = tempRegionIndex % TEMP_REGION_WIDTH;
				portIter->setRIMCol(b_temp *2);

				++ tempRegionIndex;
			
				tempAreaCounter[(portIter->RIMRow() < RIM_HEIGHT/2) ? 0 : 1] ++;
				
				tempPortInRIM.push_back(&(*portIter));
			
			} 
			else if(!(IsTempExternPort(portIter->dfgPort())))
			{  //It is an external port

				portIter->setROFRow(outRegionIndex % CDSData.height());			
		
				portIter->setRIMRow(CDSData.baseAddress() + portIter->ROFRow());
				//2011.5.11 liuxie
				portIter->setROFCol(CDSData.length() - outRegionIndex / CDSData.height() - 1);
				portIter->setRIMCol(CDSData.offset() + portIter->ROFCol()*2);

				++ outRegionIndex;
			
				/*****************************************************************************************/
				outAreaCounter[areaBelongTo(portIter->RIMRow(), portIter->RIMCol())] ++;

				outPortInRIM.push_back(&(*portIter));
				/*****************************************************************************************/

			}
			else  //it's a external temp port,�����������ڴ˴���û�н�������ת���������������û������
			{
				//2011.6.9  liuxie   //�˴��Ѿ���TEMP_REGION_WIDTH�Ŀ�����ó�8��TEMP_REGION_WIDTH_BYTE = 16 bytes��
				portIter->setROFRow(tempRegionIndex / TEMP_REGION_WIDTH);
				int c_temp = tempRegionIndex / TEMP_REGION_WIDTH;
				portIter->setRIMRow(CDSData.baseAddress() + c_temp);

				//2011.5.11 liuxie
				portIter->setROFCol(tempRegionIndex % TEMP_REGION_WIDTH);
				int d_temp = tempRegionIndex % TEMP_REGION_WIDTH;
				portIter->setRIMCol(d_temp * 2);

				++ tempRegionIndex;
			
				tempAreaCounter[(portIter->RIMRow() < RIM_HEIGHT/2) ? 0 : 1] ++;
				
				tempPortInRIM.push_back(&(*portIter));
			}
		}

		// Mark the input ports ,which will be transfered into RIF.
		//----------------------------------------------------------

		Vector<RCAPort> & rcaInport = thisRCA->inports();

		//ͳ�Ƶ�ǰRCA�������е�tempPort�Ӽ�������ssram��������������

		Vector<RCA*> & currentRCASource = thisRCA->sources();//��ǰRCA��ԴRCA
		int numberOfSection;
		numberOfSection = currentRCASource.size();            //���ж��ٸ���RCA

		List<List<int> > tempDataBlockList(numberOfSection);

		List<List<int> >::iterator listIterTemp;

		for(portIter = rcaInport.begin();portIter != rcaInport.end(); ++ portIter) 
		{
						
			DFGPort * port = portIter->dfgPort();
			assert( port!=0 );
				
			if( IsTempExternPort( portIter->dfgPort()) && !(portIter->IsInSameGroup()) ) 
			{	
				//�������tempExternPort�Ǵ��ĸ�ԴRCA���ģ�

				for(unsigned int counterSourceRCA_i=0; counterSourceRCA_i < currentRCASource.size(); counterSourceRCA_i ++ )
				{
					Vector<RCAPort>::iterator sourceRCAOutportsIter = currentRCASource[counterSourceRCA_i]->outports().begin();
					Vector<RCAPort>::iterator sourceRCAOutportsIterEnd = currentRCASource[counterSourceRCA_i]->outports().end();

					for( ; sourceRCAOutportsIter!=sourceRCAOutportsIterEnd; ++ sourceRCAOutportsIter)
					{
						if(sourceRCAOutportsIter->dfgPort() == portIter->dfgPort())
						{
							listIterTemp=tempDataBlockList.begin();
							for(unsigned int tempI=0;tempI<counterSourceRCA_i;tempI++)
								listIterTemp++;

							listIterTemp->push_back(port->SSRAMAddress());
							listIterTemp->sort();//sorting

							break;
						}
					}
				}
			}
		}

		thisRCA->setTempDataBlockList(tempDataBlockList);//�ݴ��list�����ں�������REDL;


		// Count the input inner ports and external ports
		totalInternPort = 0;
		totalExternPort = 0;
		

		Vector<RCAPort*>::reverse_iterator r_tempPortInRIMIter;
		
		//tempRIMBaseRow & tempRIMTopRow ������¼��ǰRCA�����е��ڲ��˿���RIM temp�����л���ַ�Ͷ���ַ��������һ������ҪCIDL������RIF�С�
		int tempRIMBaseRow;
		int tempRIMTopRow;

		tempRIMBaseRow = RIM_HEIGHT;
		tempRIMTopRow  = 0;
		
		int MaxPortSSRAMAddress = 0;		//���externTemp Port
		int MinPortSSRAMAddress = 0x330;	//���externTemp Port

		int MaxExPortNo = -1;              //���Extern Port
		int MinExPortNo = 10000;          //���Extern Port


		for(portIter = rcaInport.begin();portIter != rcaInport.end(); ++ portIter) 
		{
						
				DFGPort * port = portIter->dfgPort();
				assert( port!=0 );
				
				// for inner port
				if( IsInnerPort( portIter->dfgPort() ) || ((IsTempExternPort(portIter->dfgPort())) && portIter->IsInSameGroup()))
				{
					++ totalInternPort ;

					//���������ڲ��˿ڣ������Ҫ���ҳ���Щ�˿���RIM��temp����ķ�Χ���Ա�CIDL���뵽RIF�У�ͬʱҲΪ�˷���thisRCA��RIF��λ��
					//����remap�¼����RCA����RIM��Ӧ��2����ͬ��inport����һ��Ϊ��ʵ��Ҫ��inport���ڶ���ΪRCA�Լ�����������������ȡ��һ��
					if(thisRCA->getRemapFlag())		
					{
						for(tempPortInRIMIter = tempPortInRIM.begin(); tempPortInRIMIter != tempPortInRIM.end(); tempPortInRIMIter ++ )
						{
							if((*tempPortInRIMIter)->dfgPort() == portIter->dfgPort())
							{
								if( tempRIMBaseRow > ((*tempPortInRIMIter)->RIMRow()))
									tempRIMBaseRow = (*tempPortInRIMIter)->RIMRow();

								if( tempRIMTopRow < ((*tempPortInRIMIter)->RIMRow()))
									tempRIMTopRow = (*tempPortInRIMIter)->RIMRow();

								break;
							}
						}
					}
					//����������RCA��Ӧ��Ҳ��2����ͬ��inport��RIM�У�Ӧ���������ȡ��RCA�����������remap����û��Ч��
					else
					{
						for(r_tempPortInRIMIter = tempPortInRIM.rbegin(); r_tempPortInRIMIter != tempPortInRIM.rend(); r_tempPortInRIMIter ++ )
						{
							if((*r_tempPortInRIMIter)->dfgPort() == portIter->dfgPort())
							{
								if( tempRIMBaseRow > ((*r_tempPortInRIMIter)->RIMRow()))
									tempRIMBaseRow = (*r_tempPortInRIMIter)->RIMRow();

								if( tempRIMTopRow < ((*r_tempPortInRIMIter)->RIMRow()))
									tempRIMTopRow = (*r_tempPortInRIMIter)->RIMRow();

								break;
							}
						}
					}
				}
                //for external port or extern temp port
				else 
				{
					if( IsTempExternPort( portIter->dfgPort()) && (!(portIter->IsInSameGroup())))   //external temp port
					{					
						totalTempExternPort++;

						if( port->SSRAMAddress() > MaxPortSSRAMAddress)
							MaxPortSSRAMAddress = port->SSRAMAddress();	

						if( port->SSRAMAddress() < MinPortSSRAMAddress)
							MinPortSSRAMAddress = port->SSRAMAddress();	
					} 
					else //external input port
					{
						++ totalExternPort;
						if( port->seqNo() > MaxExPortNo)
							MaxExPortNo = port->seqNo();	

						if( port->seqNo() <  MinExPortNo)
							 MinExPortNo = port->seqNo();	
					}
				}
		}

		std::cout<<"tempRIMBaseRow = "<<tempRIMBaseRow<<std::endl;
		std::cout<<"tempRIMTopRow = "<<tempRIMTopRow<<std::endl;

		std::cout<<"MaxPortSSRAMAddress = "<<MaxPortSSRAMAddress<<std::endl;
		std::cout<<"MinPortSSRAMAddress = "<<MinPortSSRAMAddress<<std::endl;

		std::cout<<"MaxExPortNo = "<<MaxExPortNo<<std::endl;
		std::cout<<"MinExPortNo = "<<MinExPortNo<<std::endl;


		
		int externFIFOIndex = 0;

		if(MinExPortNo == 10000)
			MinExPortNo = -1;

		int externInportSize;

		if(MaxExPortNo >= 0 && MinExPortNo>=0)
			externInportSize = MaxExPortNo- 0 + 1;  //������Ļ���ַȡֵ
		else
			externInportSize = 0;

		
		//�����ⲿֱ�����������
		//2011.5.11 liuxie
		//const int externalRowNum = (totalExternPort/FIFO_WIDTH_DATA + ((totalExternPort % FIFO_WIDTH_DATA)? 1:0));
		const int externalRowNum = (externInportSize/FIFO_WIDTH_DATA + ((externInportSize % FIFO_WIDTH_DATA)? 1:0));

		//�����ⲿtemp��������� ����temp Extern Portռ�õ���С�ռ�
		int externalTempRow;
		externalTempRow=0;		
		
		int ExterntempSSRAMBaseAddr = 816;
		int ExterntempSSRAMTopAddr = 192;
		
		if( totalTempExternPort !=0 )
		{
			List<List<int> >::iterator listIter =tempDataBlockList.begin();
			//List<List<int> >::iterator listIter2 =tempDataBlockList.begin();
			List<int>::iterator listIntIter;
			//List<int>::iterator listIntIter2;
			
			//for extern temp inport
			for( ; listIter !=tempDataBlockList.end(); listIter++)
			{
				for(listIntIter = (*listIter).begin() ; listIntIter != (*listIter).end(); listIntIter++)
				{
					if((*listIntIter) < ExterntempSSRAMBaseAddr)
						ExterntempSSRAMBaseAddr = (*listIntIter);
					if(ExterntempSSRAMTopAddr < (*listIntIter))
						ExterntempSSRAMTopAddr = (*listIntIter);
				}
			}

			//2011.6.17 liuxie
			//int Addrdelta = ExterntempSSRAMTopAddr - ExterntempSSRAMBaseAddr;
			ExterntempSSRAMBaseAddr = (ExterntempSSRAMBaseAddr/FIFO_WIDTH) * FIFO_WIDTH;
			int Addrdelta = ExterntempSSRAMTopAddr - ExterntempSSRAMBaseAddr;
			assert(ExterntempSSRAMTopAddr >= 192);
			std::cout<<"FixedMaxPortSSRAMAddress = "<<ExterntempSSRAMTopAddr<<std::endl;
			std::cout<<"FixedMinPortSSRAMAddress = "<<ExterntempSSRAMBaseAddr<<std::endl;
			
			assert(ExterntempSSRAMTopAddr - SSRAMInBaseAddr >= 192);


			Addrdelta = Addrdelta/2 + 1;
			
			externalTempRow = Addrdelta/FIFO_WIDTH_DATA + ((Addrdelta%FIFO_WIDTH_DATA)?1:0);
		}

		const int externalTempRowNum = externalTempRow;

		const int externDataHeight = externalRowNum +  externalTempRowNum;


		int internFIFOIndexBase = externDataHeight * FIFO_WIDTH_DATA ;
		//internFIFOIndexBase ���ⲿ����ĺ��棬��ʾ��������ط���ʼ���ڲ����룻����ע�����ֵ�ı仯����ΪCIDL��CEDL����ִ�С�

		int internFIFOIndex=0;
		int externTempFIFOIndex;
		int farestexterndata = 0;        //externdata��SSRAM����Զ��ַ����externdata����ʼ��ַ�� 0x00��
		int faresttempexterndata = 0;    //tempexterndata��SSRAM����Զ��ַ����tempexterndata����ʼ��ַ ��ַδ���壩

		std::cout<<"The tempRIMBaseRow is "<<tempRIMBaseRow<<std::endl;
		std::cout<<"The tempRIMTopRow is "<<tempRIMTopRow<<std::endl;
		
		for(portIter = rcaInport.begin();portIter != rcaInport.end(); ++ portIter) 
		{

			std:: cout<<"RCA SEQNo is "<<thisRCA->seqNo()<<std::endl;

			if( IsInnerPort(portIter->dfgPort()) || ((IsTempExternPort(portIter->dfgPort())) && portIter->IsInSameGroup()))
			{   // It's an internal port

				//���������ڲ��˿ڣ������Ҫ���ҳ���Щ�˿���RIM��temp����ķ�Χ���Ա�CIDL���뵽RIF�У�ͬʱҲΪ�˷���thisRCA��RIF��λ��
				if(thisRCA->getRemapFlag())		//remapRCA��RIM�˿��������
				{
					for(tempPortInRIMIter = tempPortInRIM.begin(); tempPortInRIMIter != tempPortInRIM.end(); tempPortInRIMIter ++ )
					{
						if((*tempPortInRIMIter)->dfgPort() == portIter->dfgPort())
						{
							portIter->setRIMRow((*tempPortInRIMIter)->RIMRow());
							portIter->setRIMCol((*tempPortInRIMIter)->RIMCol());
							//����thisRCA������ڲ��˿ڣ�ͨ��CIDL���غ���RIF��λ�á�
							internFIFOIndex = internFIFOIndexBase + (((*tempPortInRIMIter)->RIMRow()) - tempRIMBaseRow)*TEMP_REGION_WIDTH + ((*tempPortInRIMIter)->RIMCol()/2);
							break;
						}
					}
				}
				else		//��remapRCA��RIM�˿��������
				{
					for(r_tempPortInRIMIter = tempPortInRIM.rbegin(); r_tempPortInRIMIter != tempPortInRIM.rend(); r_tempPortInRIMIter ++ )
					{
						if((*r_tempPortInRIMIter)->dfgPort() == portIter->dfgPort())
						{
							portIter->setRIMRow((*r_tempPortInRIMIter)->RIMRow());
							portIter->setRIMCol((*r_tempPortInRIMIter)->RIMCol());
							//����thisRCA������ڲ��˿ڣ�ͨ��CIDL���غ���RIF��λ�á�
							internFIFOIndex = internFIFOIndexBase + (((*r_tempPortInRIMIter)->RIMRow()) - tempRIMBaseRow)*TEMP_REGION_WIDTH + ((*r_tempPortInRIMIter)->RIMCol()/2);
							break;
						}
					}
				}

				//2011.6.9  liuxie
				portIter->setRIFRow(internFIFOIndex / FIFO_WIDTH_DATA);
				portIter->setRIFCol(internFIFOIndex % FIFO_WIDTH_DATA);
			} 
			else 
			{ // It's an external port or a temp external port
			
				int PortSSRAMAddress = portIter->dfgPort()->SSRAMAddress();    //for external temp data
				
				/* SSRAM memory allocation */
				//����Extern Port���ݵ�SSRAM allocation
				if( !(IsTempExternPort(portIter->dfgPort())) ) 
				{   //It's an external port
					//2011.4.22 liuxie
					//REDL��ȫ˳����루��ѡ�񣩣�CIDL��ȫ˳����루��ѡ�񣩣���ѡ������Ӧ��ֵ
					//�磺��deblocking�ļ򻯺����У���һ��RCA����13���ⲿport��˳���Ѿ���DFG�ⲿ�����17�����벻һ�������ܵ�����һ�����ض���RIF
                    int DFGInportSeq;
					DFGInportSeq = portIter->dfgPort()->seqNo();
					                  	
					//20110615 liuxie
					//portIter->setRIFRow( externFIFOIndex / FIFO_WIDTH_DATA);
					//2011.5.11 liuxie for 16bit data
					//portIter->setRIFCol(externFIFOIndex % FIFO_WIDTH_DATA);
					portIter->setRIFRow( DFGInportSeq / FIFO_WIDTH_DATA);
					portIter->setRIFCol( DFGInportSeq % FIFO_WIDTH_DATA);
					
                    //2011.5.11 liuxie  ����λ��Ϊ16bit
					portIter->dfgPort()->setSSRAMAddress(SSRAMInBaseAddr + DFGInportSeq*2);
				
					++ externFIFOIndex;
				} 
				else 
				{  // It's a temp external port
					//�ҵ�����˿�λ������
					/*List<List<int> >::iterator tempListIter =tempDataBlockList.begin();
					List<int>::iterator tempListIntIter;

					bool findFlag;
					findFlag =false;

					int thisPortLocalRow;
					thisPortLocalRow =0;//init

					int deltaValue;//ƫ����
					deltaValue=0;

					for( ; tempListIter !=tempDataBlockList.end(); ++ tempListIter)
					{
						if(tempListIter->size())
						{
							tempListIntIter = (*tempListIter).begin();

							for( ; tempListIntIter != (*tempListIter).end(); ++tempListIntIter)
							{
								if((portIter->dfgPort()->SSRAMAddress())== *tempListIntIter)
								{
									thisPortLocalRow += ((*tempListIntIter)/FIFO_WIDTH - (*((*tempListIter).begin()))/FIFO_WIDTH);

									deltaValue = (*tempListIntIter) % FIFO_WIDTH;

									findFlag=true;
									break;
								}
							}

							if(findFlag) break;

							thisPortLocalRow = (*(--(*tempListIter).end()))/FIFO_WIDTH - (*(*tempListIter).begin())/FIFO_WIDTH;
						}
					}

					externTempFIFOIndex = (externalRowNum + thisPortLocalRow) * FIFO_WIDTH + deltaValue;

					portIter->setRIFRow( externTempFIFOIndex / FIFO_WIDTH_DATA );
					portIter->setRIFCol( externTempFIFOIndex % FIFO_WIDTH_DATA );
					*/
					List<List<int> >::iterator tempListIter =tempDataBlockList.begin();
					List<int>::iterator tempListIntIter;

					bool findFlag;
					findFlag =false;

					int tempExPortSeqNo = -1;
					int deltaValue;

					int thisPortLocalRow;

					for( ; tempListIter !=tempDataBlockList.end(); ++ tempListIter)
					{
						if(tempListIter->size())
						{
							tempListIntIter = (*tempListIter).begin();

							for( ; tempListIntIter != (*tempListIter).end(); ++tempListIntIter)
							{
								if((portIter->dfgPort()->SSRAMAddress())== *tempListIntIter)
								{
									
									//tempExPortSeqNo = (portIter->dfgPort()->SSRAMAddress() - 768) / 2;
									tempExPortSeqNo = (portIter->dfgPort()->SSRAMAddress() - ExterntempSSRAMBaseAddr) / 2;
									findFlag=true;
									break;
								}
							}

							if(!findFlag) continue;;

							deltaValue = tempExPortSeqNo % FIFO_WIDTH_DATA;

							thisPortLocalRow = (tempExPortSeqNo) / FIFO_WIDTH_DATA + ((tempExPortSeqNo % FIFO_WIDTH_DATA?1:0));
							if ((thisPortLocalRow == 0) && (tempExPortSeqNo != 0)) thisPortLocalRow = 1;

							break;
						}
					}


					if (deltaValue == 0)
					{
						externTempFIFOIndex = (externalRowNum + thisPortLocalRow) * FIFO_WIDTH_DATA + deltaValue;
					}
					else
					{
						externTempFIFOIndex = (externalRowNum + thisPortLocalRow - 1) * FIFO_WIDTH_DATA + deltaValue;
					}
					portIter->setRIFRow( externTempFIFOIndex / FIFO_WIDTH_DATA );
					portIter->setRIFCol( externTempFIFOIndex % FIFO_WIDTH_DATA );

				}
				
			}
		}

		//2011.7.19 longlee RIFԽ���鼰���Ų���
        int maxRIFRow = 0;
		bool remapFlag = false;
		for(portIter = rcaInport.begin();portIter != rcaInport.end(); ++ portIter) 
		{
			maxRIFRow = (portIter->RIFRow() > maxRIFRow)? portIter->RIFRow():maxRIFRow;
			if ( maxRIFRow > 7) 
			{
				remapFlag = true;	//˵����Ҫ����remap;
				//RIM״̬�ع�
				RIM.copy(RIM_backup);
				tempPortInRIM.clear();
				outPortInRIM.clear();
				for (tempPortInRIMIter = tempPortInRIM_backup.begin(); tempPortInRIMIter != tempPortInRIM_backup.end(); ++tempPortInRIMIter)
				{
					tempPortInRIM.push_back(*tempPortInRIMIter);
				}
				for (outPortInRIMIter = outPortInRIM_backup.begin(); outPortInRIMIter != outPortInRIM_backup.end(); ++outPortInRIMIter)
				{
					outPortInRIM.push_back(*outPortInRIMIter);
				}

				//longlee:20111005
				tempAreaCounter = tempAreaCounter_backup;
				outAreaCounter = outAreaCounter_backup;

				//��ȡ��ǰRCA������λ��
				Vector<RCA*>::iterator RCAIter;
				int index = 0;
				for (RCAIter = RCAS.begin();RCAIter != RCAS.end(); RCAIter++,index ++)
				{
					if(*RCAIter != *rcaIter) continue;
					else break;
				}
				//�����Ų�����RCA���޸Ķ˿ڹ�ϵ
				//int index = (*RCAIter)->seqNo();
				remap(RCAS,index,tempPortInRIM,config,0);
				//��ǵ�ǰRCAmapʧ�ܣ�����
				break;
			}
		}
		
		if (remapFlag) 
		{
			thisRCA->setMappedFlag(false);
			CL1Block RemapBlock;
			RemapBlock.setRemapFlag(true);
			mapBlocks.push_back(RemapBlock);
			return mapBlocks;
		}

		//******************************************start**********************************************************************
		
		//����ֱ�Ӵ�Ƭ��SSRAM����Ļ���ַΪ0������Ҫ�������е�inport�ҵ������ַ��Զ���Ǹ�port��λ��Ϊtop address�ṩ��REDL
		Vector<RCAPort>::iterator RCAInportIter;
		for( RCAInportIter = rcaInport.begin(); RCAInportIter != rcaInport.end(); ++  RCAInportIter)
		{
			    DFGPort * port = RCAInportIter->dfgPort();
				assert( port!=0 );
			    // for inner port
				if( IsInnerPort( RCAInportIter->dfgPort() ) )
					continue;
				else if(IsTempExternPort( RCAInportIter->dfgPort()) && RCAInportIter->IsInSameGroup())
					continue;
				else //external temp data
				{
					if( IsTempExternPort( RCAInportIter->dfgPort()) )   //external temp port
					{
						if(port->SSRAMAddress() > faresttempexterndata)
							faresttempexterndata = port->SSRAMAddress();
					}
					else
					{    //external input port					
						 if(port->SSRAMAddress() > farestexterndata)
							farestexterndata=port->SSRAMAddress();     //ָ����Զ���Ǹ�data��SSRAM�е���ʼλ��
					}
				}
		}
		//******************************************end**********************************************************************


		//2011.5.28 liuxie for extern input Port		
		thisRCA->setRCASSRAMInBaseAddr(SSRAMInBaseAddr);//���õ�ǰRCA�ⲿֱ������Ļ���ַ		
        
        if(farestexterndata!=0)
			SSRAMInTopAddr = (farestexterndata/FIFO_WIDTH + ((farestexterndata+1)%FIFO_WIDTH?1:0))*FIFO_WIDTH;
		else
			SSRAMInTopAddr = (farestexterndata/FIFO_WIDTH + ((farestexterndata)%FIFO_WIDTH?1:0))*FIFO_WIDTH;
		//2011.5.11 liuxie
		thisRCA->setRCASSRAMInTopAddr(SSRAMInTopAddr);

		//2011.5.28 liuxie for ExternTemp input Port
		thisRCA->setRCASSRAMTempInBaseAddr((ExterntempSSRAMBaseAddr/FIFO_WIDTH)*FIFO_WIDTH);

		if((faresttempexterndata - SSRAMInBaseAddr) >= 192)
			SSRAMTempInTopAddr = (faresttempexterndata/FIFO_WIDTH + ((faresttempexterndata+1)%FIFO_WIDTH?1:0))*FIFO_WIDTH;
		else
			SSRAMTempInTopAddr = (faresttempexterndata/FIFO_WIDTH + ((faresttempexterndata)%FIFO_WIDTH?1:0))*FIFO_WIDTH;

		thisRCA->setRCASSRAMTempInTopAddr(SSRAMTempInTopAddr);

		//////////////////////////////////////////////////////////////////
        // Map the block data

		CL1Block block;

		// Set CEDL
		block.CEDLData().setTarget(CEDL_TGT_RIF);         //CEDL��Ŀ�������RIFҲ������RIM
		block.CEDLData().setHeight(externDataHeight);	

		// Set CIDL
		//--------------------------------------------
		//  CIDL need to Find all output data of source 
		//  RCA in temperary data region of RIM, and 
		//  connect them to one 2D data. That need to 
		//  find the lowest base address and height of
		//  the end address first.

		int lowestBaseAddr = RIM_HEIGHT;    //��ǰrca��source rca��RIM�е���͵�ַ
		int highestEndAddr = 0;             //��ǰrca��source rca��RIM�е���ߵ�ַ

		Vector<RCA*>::iterator srcIter;
		Vector<RCA*> & thisRCASource = thisRCA->sources();

		Vector<CL1Block>::iterator blockMappedIter;

		Vector<RCA*>::iterator tmpGrpRCAIter;//yin0831
		bool CIDLEnableValue;
		CIDLEnableValue=false;

		for(srcIter = thisRCASource.begin();srcIter != thisRCASource.end(); ++ srcIter)
		{
			
			for(tmpGrpRCAIter = tmpGrpRCA.begin();tmpGrpRCAIter != tmpGrpRCA.end();++tmpGrpRCAIter)
			{
				if(((*srcIter)->seqNo()) == ((*tmpGrpRCAIter)->seqNo()))//yin0831 ��ͬһgroup���Ҫ����cidl������
				{
					CIDLEnableValue=true;//Setting CIDLEnableValue

					std::cout<<"rca seqNo = "<<(*srcIter)->seqNo()<<std::endl;
					int srcRCABaseAddr = RIM.getBaseAddress((*srcIter)->seqNo());
					int srcRCAEndAddr = RIM.getEndAddress((*srcIter)->seqNo());

					if(srcRCABaseAddr < lowestBaseAddr) 
						lowestBaseAddr = srcRCABaseAddr;

					if(highestEndAddr < srcRCAEndAddr) 
						highestEndAddr = srcRCAEndAddr;
				}
			}
		}
		std::cout<<"tempRIMBaseRow is "<<tempRIMBaseRow<<std::endl;
		std::cout<<"tempRIMTopRow is "<<tempRIMTopRow<<std::endl;
		std::cout<<"lowestBaseAddr is "<<lowestBaseAddr<<std::endl;
		std::cout<<"highestEndAddr is "<<highestEndAddr<<std::endl<<std::endl;
		
		
		
		////2011.5.11 liuxie ��CIDL��дģʽ��4��8byteƴ�� �޸ĳ�Ϊ 2��16Byteƴ��
		lowestBaseAddr = tempRIMBaseRow;
		highestEndAddr = tempRIMTopRow + 1;//��������(�߶�)
		//yin0909end

		block.CIDLData().setInputMode(MODE_IN_V2D);    //2D����ģʽ,�׵�ַ�ɱ�

		block.CIDLData().setBaseAddress(lowestBaseAddr); 
		//2011.5.11 liuxie
        block.CIDLData().setLength(TEMP_REGION_WIDTH_BYTE); //ȫ����16ByteΪ����д��
		//setHeight 2011.4.25 liuxie
		//////////////////////////////////////////////////////////////////
		block.CIDLData().setHeight(highestEndAddr - lowestBaseAddr);
		//////////////////////////////////////////////////////////////////
		//2011.5.11 liuxie
		block.CIDLData().setOutputMode(MODE_OUT_2L);    //����2D����ÿ��2�ȷ֣��������ƴ�����
		block.CIDLData().setOffset(0);                

		// Set CDS
		//--------------------------------------------

		CDSData.setTarget(CDS_TGT_RIM);
		block.setCDSData(CDSData);

		// Set Core

		block.setRCAIndex(thisRCA->seqNo());
		block.setRCACoreLoop(1);
		block.setConst1Address(0);
		block.setConst2Address(0);
		block.setRIDLEnable(false);

		bool CEDLEnable = externDataHeight? true : false;
		
		// setting the CIDLEnable value
		bool CIDLEnable = CIDLEnableValue;

		block.setCEDLEnable(CEDLEnable);
		block.setCIDLEnable(CIDLEnable);

		mapBlocks.push_back(block);
		thisRCA->setState(STA_OVER);
		thisRCA->setMappedFlag(true);

		tmpGrpRCA.push_back(thisRCA);	
/*		//longlee ֻ�ͷŵ�ǰRCA��source
		//freeRIMSpace(RCAS);
		Vector<RCA*> srcVec = thisRCA->sources();
		if(!thisRCA->getRemapFlag()) freeRIMSpace(srcVec);
		else 
		{
			Vector<RCA*>::iterator srcVecIter = srcVec.begin();
			Vector<RCA*> thisGrpSrc;
			for (; srcVecIter != srcVec.end(); ++srcVecIter)
			{
				if((*srcVecIter)->CL0GroupNumber() == thisRCA->CL0GroupNumber()) thisGrpSrc.push_back(*srcVecIter);
			}
			if (thisGrpSrc.size() != 0) freeRIMSpace(thisGrpSrc);
		}
		*/
	}

	return mapBlocks;
}


void CL1Config::freeRIMSpace(const Vector<RCA*> rcas){

	Vector<RCA*>::const_iterator rcaIter;
	for(rcaIter = rcas.begin(); rcaIter != rcas.end(); ++ rcaIter){

		RCA * thisRCA = *rcaIter;
		assert(thisRCA != 0);
		if(thisRCA->state() != STA_OVER)continue;

		const Vector<RCA*> & rcaTarget = thisRCA->targets();
		Vector<RCA*>::const_iterator tgtIter;
		bool canBeFree = true;
		for(tgtIter = rcaTarget.begin(); 
			tgtIter != rcaTarget.end(); ++ tgtIter){

			if((*tgtIter)->state() != STA_OVER){

				canBeFree = false;
				break;
			}
		}

		if(canBeFree)
		{
			Vector<int> FreeRIMAddr;
			FreeRIMAddr = RIM.free(thisRCA->seqNo());
			/*
			for(int i = 0; i < (int)FreeRIMAddr.size(); i++ )
			  ( FreeRIMAddr[i] < RIM_HEIGHT/2 ) ? tempAreaCounter[0]--
			  									: tempAreaCounter[1]-- ;
			*/
			Vector<RCAPort> & rcaOutport = thisRCA->outports();
			Vector<RCAPort>::iterator portIter;
			Vector<RCAPort *>::iterator tempPortIter;
			//FIXME : aglrithm can be better
			for(portIter = rcaOutport.begin(); 
			    portIter != rcaOutport.end(); ++ portIter)
				if (thisRCA->getRemapFlag())	//remapRCA����������ڶ������ͷ�
				{
					for(tempPortIter = tempPortInRIM.begin(); tempPortIter != tempPortInRIM.end(); ++tempPortIter )
					if(  (*tempPortIter)->RIMRow() ==  portIter->RIMRow() &&  (*tempPortIter)->RIMCol() ==  portIter->RIMCol()  
						&& (*tempPortIter)->dfgPort() == portIter->dfgPort())
 
						{						
						(portIter->RIMRow() < RIM_HEIGHT/2) ? tempAreaCounter[0]--
															: tempAreaCounter[1]-- ;
						tempPortInRIM.erase( tempPortIter );
						break;
						}
					
				}
				else
				{
					if (IsInnerPort(portIter->dfgPort()))		//����RCA���ڲ���������ͷ�
					{
					for(tempPortIter = tempPortInRIM.begin(); tempPortIter != tempPortInRIM.end(); ++tempPortIter )
						if(  (*tempPortIter)->RIMRow() ==  portIter->RIMRow() &&  (*tempPortIter)->RIMCol() ==  portIter->RIMCol()  
							&& (*tempPortIter)->dfgPort() == portIter->dfgPort())
 
							{						
							(portIter->RIMRow() < RIM_HEIGHT/2) ? tempAreaCounter[0]--
																: tempAreaCounter[1]-- ;
							tempPortInRIM.erase( tempPortIter );
							break;
							}
					}
				}
			//else
				//continue;
				
		}
	}
}

//////////////////////////////////////////////////////////////////////////

#define RCA_BYPASS_SUM      128

int CL1Config::sumPseudoRCA(){

	int sum = 0;
	
	for(int i =0 ; i < MAX_OUT_AREA_SUM ; i = i + 2){

		int dataSum = outAreaCounter[i] + outAreaCounter[i+1];
		sum += (dataSum > RCA_BYPASS_SUM)? 2:1 ;
	}
	
	for(int i =0; i < MAX_TEMP_AREA_SUM; i = i + 2){
		int dataSum = tempAreaCounter[i] + tempAreaCounter[i+1];
		sum += (dataSum > RCA_BYPASS_SUM)? 2:1;
	}
	return sum;
}


#define TRANS_MODE_0	0 
#define TRANS_MODE_1	1 


//2011.5.18 liuxie  
///////////////////////////////////////////start////////////////////////////////////////////////////////////////////////////
const Vector<CL1Block> CL1Config::insertPseudoRCA(RPUConfig & config, Vector<RCA *> & CL1RCA, Vector<RCA *> & recordPseduRCA)
{
	Vector<CL1Block> pseudoBlock;

//	int transferMode = TRANS_MODE_1;
int i;

	//for out region data
	for(i=0; i<MAX_AREA_SUM/2; ++i)
	{
		bool isUpArea = (i & 0x01)? false : true;
		bool isDownArea = !isUpArea;

		if(outAreaCounter[i] == 0)
			continue;
		
		RCA * thisRCA = new RCA( config.rcas().size() + PseudoRCANum + remapSeqNo);
		PseudoRCANum++;

		assert(thisRCA != 0);

		CL1RCA.push_back(thisRCA);
		 recordPseduRCA.push_back(thisRCA);

		thisRCA->setPseudoRCAFlag(1);
		//check here!!!!!
		//thisRCA->setPseudoRCAMode(transferMode);

		Vector<RCAPort> & rcaInports = thisRCA->inports();
		Vector<RCAPort> & rcaOutports = thisRCA->outports();

		// Judge the data in out region of RIM whether is in 
		// data source area of this RCA.
		Vector<RCAPort*>::iterator outIter;
		
		const int edgeRow = RIM_HEIGHT/2;
		int rcIndex = 0;
		int ROFHeight = 0;
			
		for(outIter = outPortInRIM.begin(); outIter != outPortInRIM.end(); ++outIter)
		{
			RCAPort * thisPort = * outIter;
				
			//bytes 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 | 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
			//bytes 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 | 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
            //data   0     1     2     3     4     5     6     7    |  0     1     2     3     4     5     6     7
            //                       temp                           |                     out
			//                       ����2                          |                     ����0
			//                       ����3                          |                     ����1 
			//data  15    14    13    12    11    10     9     8    |  7     6     5     4     3     2     1     0  
				
			const int colIndex = RIM_WIDTH/2 - 1 - thisPort->RIMCol()/2;   //������RIM�еı�ŷ�ת����

			bool colBelongTo = (i/2 * DATA_PER_LINE <= colIndex) && (colIndex < (i/2+1) * DATA_PER_LINE); 

			bool rowBelongTo = false;
				
			if(i==0 && isUpArea && thisPort->RIMRow() < edgeRow && thisPort->RIMRow() >= 0)
				rowBelongTo = true;
			if(i==1 && isDownArea && thisPort->RIMRow() >= edgeRow && thisPort->RIMRow() < RIM_HEIGHT)
				rowBelongTo = true;
			
			if(colBelongTo && rowBelongTo)
			{
				//Add new inport and outport to pseudo-RCA
				rcaInports.push_back(*thisPort);
				rcaInports.back().setRCA(thisRCA);

				rcaOutports.push_back(*thisPort);
				rcaOutports.back().setRCA(thisRCA);

				// Add bypass node to pseudo-RCA
				DFGNode * bypNode = config.newBypsNode();
				bypNode->addSource(thisPort->dfgPort());
				bypNode->addTarget(thisPort->dfgPort());

				if(rcIndex < RC_TEMP_NUM)
					thisRCA->temp(rcIndex).setDFGNode(bypNode);
				else
					thisRCA->node(rcIndex - RC_TEMP_NUM).setDFGNode(bypNode);

				++ rcIndex;
			}
		}

		// Set port location in RIF and ROF
		//����һ�������һ����16bytes����RIF��ROFһ����32bytes������αRCA�Ķ���ΪRIM������ƴ�ӳ�RIF��ROFһ�У�
		Vector<RCAPort>::iterator portIter;

		for(portIter = rcaInports.begin();portIter != rcaInports.end(); ++ portIter)
		{
			const int AreaIndex = portIter->RIMRow() * DATA_PER_LINE + (portIter->RIMCol()/2 - DATA_PER_LINE);

			const int RIFIndex = AreaIndex - ( (i==1)? edgeRow * DATA_PER_LINE : 0);

			portIter->setRIFRow( RIFIndex / FIFO_WIDTH_DATA);
			portIter->setRIFCol( RIFIndex % FIFO_WIDTH_DATA);
		}

		int ROFIndex = 0;

		for(portIter = rcaOutports.begin(); portIter != rcaOutports.end(); ++portIter)
		{
			portIter->setROFRow( ROFIndex / FIFO_WIDTH_DATA);
			portIter->setROFCol( ROFIndex % FIFO_WIDTH_DATA);

			DFGPort * port = portIter->dfgPort(); 
			assert(port != 0);
			port->setSSRAMAddress(SSRAMOutBaseAddr + ROFIndex*2);
				
			++ROFIndex;
		}
			
		ROFHeight = ROFIndex/FIFO_WIDTH_DATA + ((ROFIndex % FIFO_WIDTH_DATA)? 1 : 0);

		thisRCA->setRCASSRAMOutBaseAddr(SSRAMOutBaseAddr);

		SSRAMOutBaseAddr += ROFHeight * FIFO_WIDTH;
		SSRAMOutTopAddr = SSRAMOutBaseAddr;
		thisRCA->setRCASSRAMOutTopAddr(SSRAMOutTopAddr);

		CL1Block block;

		const int baseAddr = ( isDownArea) ? edgeRow : 0;

		//2011.5.27 liuxie
		//const int height = (transferMode == TRANS_MODE_0)? RIM_HEIGHT : RIM_HEIGHT/2;
		const int height =  RIM_HEIGHT/2;

		CL1Data & CIDLData = block.CIDLData();
		CIDLData.setBaseAddress(baseAddr);
		CIDLData.setLength(BYTE_PER_LINE);
		CIDLData.setHeight(height);
		CIDLData.setOffset(RIM_WIDTH - BYTE_PER_LINE);
		CIDLData.setInputMode(MODE_IN_V2D);    //2D�����׵�ַ�ɱ�
		CIDLData.setOutputMode(MODE_OUT_2L);   //ÿ��2�ȷݣ��������ƴ�����

		CL1Data & CDSData = block.CDSData();
		CDSData.setTarget(CDS_TGT_ESDF);
		CDSData.setHeight(ROFHeight); 

		block.setRCAIndex(thisRCA->seqNo());
		block.setRCACoreLoop(1);
		block.setConst1Address(0);
		block.setConst2Address(0);
		block.setRIDLEnable(false);
		block.setCEDLEnable(false);
		block.setCIDLEnable(true);

		pseudoBlock.push_back(block);
	
		config.addRCAToAllRCAVec(thisRCA);//yin0901-��αRCA�ӵ�Vec�д洢����
	}

	int curTempSSRAMBase = 0;
	for(i=2; i< MAX_AREA_SUM ; ++i)
	{
		bool isUpArea = (i & 0x01)? false : true;
		bool isDownArea = !isUpArea;

		if(tempAreaCounter[i-2] == 0)
			continue;
		RCA * thisRCA = new RCA( config.rcas().size() + PseudoRCANum + remapSeqNo);
		PseudoRCANum++;

		assert(thisRCA != 0);

		CL1RCA.push_back(thisRCA);
		recordPseduRCA.push_back(thisRCA);

		thisRCA->setPseudoRCAFlag(1);
		//check here!!!!!
		//thisRCA->setPseudoRCAMode(transferMode);

		Vector<RCAPort> & rcaInports = thisRCA->inports();
		Vector<RCAPort> & rcaOutports = thisRCA->outports();

		// Judge the data in out region of RIM whether is in 
		// data source area of this RCA.
		Vector<RCAPort*>::iterator outIter;
		
		const int edgeRow = RIM_HEIGHT/2;
		int rcIndex = 0;
		int ROFHeight = 0;

		SSRAMTempOutBaseAddr += curTempSSRAMBase;

		for(outIter = tempPortInRIM.begin();outIter != tempPortInRIM.end(); ++ outIter)
		{
			RCAPort * thisPort = * outIter;
			const int colIndex = RIM_WIDTH/2 - 1 - thisPort->RIMCol()/2;
			bool colBelongTo = (8 <= colIndex) && (colIndex < 16);
			bool rowBelongTo = false;
			if(i==2 && isUpArea && thisPort->RIMRow() < RIM_HEIGHT/2 && thisPort->RIMRow() >= 0)
				rowBelongTo = true;
			if(i==3 && isDownArea && thisPort->RIMRow() >= RIM_HEIGHT/2 && thisPort->RIMRow() < RIM_HEIGHT)
				rowBelongTo=  true;
				
			if( colBelongTo && rowBelongTo )
			{
				// Add new inport and outport to pseudo-RCA
				rcaInports.push_back(*thisPort);
				rcaInports.back().setRCA(thisRCA);

				rcaOutports.push_back(*thisPort);
				rcaOutports.back().setRCA(thisRCA);

				// Add bypass node to pseudo-RCA
				DFGNode * bypNode = config.newBypsNode();
				bypNode->addSource(thisPort->dfgPort());
				bypNode->addTarget(thisPort->dfgPort());
					
				if(rcIndex < RC_TEMP_NUM)
					thisRCA->temp(rcIndex).setDFGNode(bypNode);
				else
					thisRCA->node(rcIndex - RC_TEMP_NUM).setDFGNode(bypNode);

				++ rcIndex;
			}
		}

		Vector<RCAPort>::iterator portIter;

		for(portIter = rcaInports.begin();portIter != rcaInports.end(); ++ portIter)
		{
			const int AreaIndex = portIter->RIMRow() * DATA_PER_LINE + portIter->RIMCol()/2;

			const int RIFIndex = AreaIndex - ((i == 3) ? edgeRow*DATA_PER_LINE : 0);

			portIter->setRIFRow( RIFIndex / FIFO_WIDTH_DATA);
			portIter->setRIFCol( RIFIndex % FIFO_WIDTH_DATA);
		}

		int ROFIndex = 0;
		for(portIter = rcaOutports.begin();	portIter != rcaOutports.end(); ++ portIter)
		{
			
			//2011.6.18 liuxie
			portIter->setROFRow(ROFIndex / FIFO_WIDTH_DATA);
			portIter->setROFCol(ROFIndex % FIFO_WIDTH_DATA);

			DFGPort * port = portIter->dfgPort(); 
			assert(port != 0);
				
			//2011.5.27 liuxie
			port->setSSRAMAddress(SSRAMTempOutBaseAddr + ROFIndex*2);

			ROFIndex++;

		}

		ROFHeight = (ROFIndex)/FIFO_WIDTH_DATA + (((ROFIndex) % FIFO_WIDTH_DATA)? 1 : 0);

		//2011  liuxie
		/*SSRAMTempOutBaseAddr = SSRAMInBaseAddr;

		thisRCA->setRCASSRAMTempOutBaseAddr(SSRAMTempOutBaseAddr);

		SSRAMInBaseAddr +=  ROFHeight * FIFO_WIDTH ;
		SSRAMInTopAddr = SSRAMInBaseAddr;
			
		SSRAMTempOutTopAddr = SSRAMInTopAddr;
		thisRCA->setRCASSRAMTempOutTopAddr(SSRAMTempOutTopAddr);
		*/

		thisRCA->setRCASSRAMTempOutBaseAddr(SSRAMTempOutBaseAddr);

		SSRAMTempOutTopAddr = SSRAMTempOutBaseAddr + ROFHeight * FIFO_WIDTH;
		curTempSSRAMBase = ROFHeight * FIFO_WIDTH;

		thisRCA->setRCASSRAMTempOutTopAddr(SSRAMTempOutTopAddr);
			
		CL1Block block;

		const int baseAddr = (isDownArea) ? edgeRow : 0;

		//2011.5.27 liuxie
		//const int height = (transferMode == TRANS_MODE_0)? RIM_HEIGHT : RIM_HEIGHT/2;
		const int height = RIM_HEIGHT/2;

		CL1Data & CIDLData = block.CIDLData();
		CIDLData.setBaseAddress(baseAddr);
		CIDLData.setLength(BYTE_PER_LINE);
		CIDLData.setHeight(height);
		CIDLData.setOffset(0);
		CIDLData.setInputMode(MODE_IN_V2D);    //2D�����׵�ַ�ɱ�
		CIDLData.setOutputMode(MODE_OUT_2L);   //ÿ��2�ȷݣ��������ƴ�����

		CL1Data & CDSData = block.CDSData();
		CDSData.setTarget(CDS_TGT_ESDF);
		CDSData.setHeight(ROFHeight); 

		block.setRCAIndex(thisRCA->seqNo());
		block.setRCACoreLoop(1);
		block.setConst1Address(0);
		block.setConst2Address(0);
		block.setRIDLEnable(false);
		block.setCEDLEnable(false);
		block.setCIDLEnable(true);

		pseudoBlock.push_back(block);

		config.addRCAToAllRCAVec(thisRCA);//yin0901-��αRCA�ӵ�Vec�д洢����
	
		//return pseudoBlock;

	}

///////////////////////////////////////////////////////////////end//////////////////////////////////////////////////////////////////////////////////////////////////////
	
	return pseudoBlock;


} //end of insertPseudoRCA

int CL1Config::PreGenCL1(Vector<RCA*> & rcas)
{
	// Initialize the schedule states
	Vector<RCA*>::iterator rcaIter;

	for(rcaIter = rcas.begin(); 
		rcaIter != rcas.end(); ++ rcaIter){

			(*rcaIter)->setState(STA_WAIT);
	}

	// Initialize RIM
	RIM.clear();

	// Schedule begin
	int ScheduleFlaseFlag = 0;
	int GroupSSRAMInBase = 0;
	
	int remainRCANumber;
	int &remainRCANum =remainRCANumber;
	remainRCANum=0;

	Vector<RCA*> tmpGroupRCA;//�ݴ�ͬһgroup��RCA,������PseudoRCA
	tmpGroupRCA.reserve( MAX_CL1_RCA_NUM );

	int remapSeqNo_backup = remapSeqNo;
	RPUConfig config_fake;
	while(!scheduleOver(rcas))
	{
		updateRCAState(rcas, ScheduleFlaseFlag,remainRCANum);
		
		if(ScheduleFlaseFlag == 1)  break;
		if(rcas.size() > 25 ) break;
		const Vector<CL1Block> 
			blocks = PreMapRCA(readyRCA(rcas),tmpGroupRCA,rcas,config_fake);
		
         //yin20101119 revised begin

		if(blocks.empty()) break;
		//yin20101119 revised end


		freeRIMSpace(rcas);
	}
	
	remapSeqNo = remapSeqNo_backup;

	int TotalOrigin = 0;	//��¼���Ա�����CL1RCA�е�ԭʼRCA����
	int TotalRemap = 0;	//��¼���뵽CL1Copy�е�Remap����
	for (rcaIter = rcas.begin(); rcaIter != rcas.end(); ++rcaIter)
	{
		RCA * thisRCA =* rcaIter;
		if(thisRCA->getRemapFlag())
		{
			TotalRemap +=1;
			continue;
		}

		if(TotalOrigin + TotalRemap + 1 > 12)
			return TotalOrigin;
		else if(thisRCA->getMappedFlag()) TotalOrigin +=1;
	}
	return TotalOrigin;
}

Vector<CL1Block> 
CL1Config::genCL1Block( RPUConfig & config, Vector<RCA*> & rcas){

	// Initialize the schedule states
	Vector<RCA*>::iterator rcaIter;

	for(rcaIter = rcas.begin(); 
		rcaIter != rcas.end(); ++ rcaIter){

			(*rcaIter)->setState(STA_WAIT);
	}

	// Initialize RIM
	RIM.clear();

	// Schedule begin
	Vector<CL1Block> blockMapped;
	blockMapped.reserve( MAX_CL1_RCA_NUM );
	int ScheduleFlaseFlag = 0;
	int GroupSSRAMInBase = 0;
	
	int remainRCANumber;
	int &remainRCANum =remainRCANumber;
	remainRCANum=0;

	Vector<RCA*> tmpGroupRCA;//�ݴ�ͬһgroup��RCA,������PseudoRCA
	tmpGroupRCA.reserve( MAX_CL1_RCA_NUM );

	while(!scheduleOver(rcas))
	{
		updateRCAState(rcas, ScheduleFlaseFlag,remainRCANum);
		
		if(ScheduleFlaseFlag == 1)  break;
		
		Vector<CL1Block> blocks = mapRCA(readyRCA(rcas),tmpGroupRCA,rcas,config);
		
		Vector<CL1Block>::iterator blockIter;
		if(blocks.size() > 1)
		{
			for(blockIter = blocks.begin(); blockIter != blocks.end(); ++ blockIter)
			{
				blockIter->setRemapFlag(false);
			}
		}
		
        //yin20101119 revised begin
		if(blocks.empty()) break;
		if(blocks.begin()->getRemapFlag()) continue;
		if(blocks.begin()->getTMFlag()) break;
		//yin20101119 revised end


		freeRIMSpace(rcas);

		//Append the block mapped 
		Vector<CL1Block>::const_iterator cblockIter;
		for(cblockIter = blocks.begin(); 
			cblockIter != blocks.end(); ++ cblockIter){
			blockMapped.push_back(*cblockIter);
		}
	}


	Vector<RCA*>::iterator tmpGrpRCAIter_0;
	for(tmpGrpRCAIter_0 = tmpGroupRCA.begin();tmpGrpRCAIter_0!=tmpGroupRCA.end(); tmpGrpRCAIter_0++)
		config.addRCAToAllRCAVec(*tmpGrpRCAIter_0);//yin0901-��mapped RCA�ӵ�Vec�д洢����


	return blockMapped;	
}


const Vector<reg32> & CL1Config::genRegs(const Vector<CL1Block> & blocks){

	CL1Regs CL1ConfigRegs;

	CL1CoreLength = static_cast<int>(blocks.size());
	assert(CL1CoreLength != 0);

	Vector<CL1Block>::const_iterator iter, end;

	CL1RIDLLength = 0;

	int CoreIndex = 0;
	int BlcokMappedBefore = RPUBlockMappedBefore;
	for(iter = blocks.begin(); iter != blocks.end(); ++ iter, ++CoreIndex){
		
		const CL1Block & curBlock = *iter;

		// Set RIDL Context
		if(curBlock.RIDLEnable()){

			RIDLReg & curRIDLReg = CL1ConfigRegs.RIDLContext(CL1RIDLLength++);

			const CL1Data & data = curBlock.RIDLData();

			curRIDLReg.setSourceType(data.source());
			curRIDLReg.setInputBaseAddress(data.baseAddress());
			curRIDLReg.setInputLength(data.length());
			curRIDLReg.setInputHeight(data.height());
			curRIDLReg.setInputMode(data.inputMode());
 
			if(curRIDLReg.targetType() == RIDL_TGT_RIM){

				//!TODO: Allocate CL1RIM Space ...
			}
		}

		// Set CoreContext
		//--------------------------------------------------------------

		// (1) Core
		RCACoreReg & curCoreReg = CL1ConfigRegs.RCContext(CoreIndex);

		//20110719 liuxie for CL2 Number
		

		//curCoreReg.setCoreIndex(BlcokMappedBefore + curBlock.RCAIndex());    //ѡ����Ӧ��CL2��ͨ����ǰblock������ĵ�ǰ���RCA���
		curCoreReg.setCoreIndex(curBlock.RCAIndex());    //ѡ����Ӧ��CL2��ͨ����ǰblock������ĵ�ǰ���RCA���
		curCoreReg.setCoreLoop(curBlock.RCACoreLoop());
		curCoreReg.setConst1Address(curBlock.const1Address());
		curCoreReg.setConst2Address(curBlock.const2Address());
		curCoreReg.setCEDLEnable(curBlock.CEDLEnable());
		curCoreReg.setCIDLEnable(curBlock.CIDLEnable());

		// (2) CEDL
		if(curBlock.CEDLEnable()){

			CEDLReg & curCEDLReg = CL1ConfigRegs.CEDLContext(CoreIndex);

			const CL1Data & data = curBlock.CEDLData();

			curCEDLReg.setTargetType(data.target());
			curCEDLReg.setDataSum(data.height());
		}

		// (3) CIDL
		if(curBlock.CIDLEnable()){
			
			CIDLReg & curCIDLReg = CL1ConfigRegs.CIDLContext(CoreIndex);

			const CL1Data & dataCIDL = curBlock.CIDLData();

			curCIDLReg.reset();
			curCIDLReg.setInputMode(dataCIDL.inputMode());
			curCIDLReg.setInput1BaseAddress(dataCIDL.baseAddress());
	
			curCIDLReg.setInput1Length(dataCIDL.length());
			curCIDLReg.setInputHeight(dataCIDL.height());
			
			/* merge four line to one */
			curCIDLReg.setOutputMode(dataCIDL.outputMode()); 
			curCIDLReg.setInputOffset(dataCIDL.offset());
		
		}

		// (4) CDS
		CDSReg & curCDSReg = CL1ConfigRegs.CDSContext(CoreIndex);

		const CL1Data & dataCDS = curBlock.CDSData();

		int CDSTarget = dataCDS.target();
		curCDSReg.setTargetType(CDSTarget);

		curCDSReg.setDataSum(dataCDS.height());
		switch(CDSTarget){
			case CDS_TGT_RIF: break;
			case CDS_TGT_RIM: 
				curCDSReg.setRIMAddress(dataCDS.baseAddress());
				curCDSReg.setRIMLength(dataCDS.length()*2);
				curCDSReg.setRIMOffset(dataCDS.offset());
				break;
			case CDS_TGT_ESDF: /* Add code here */ break;
			case CDS_TGT_MB: /* Add code here */ break;
			case CDS_TGT_RB: /* Add code here */ break;
			default: assert(0); // Unknown target;
		}
	}

	CL1TotalLength = 1 + CL1RIDLLength + 4 * CL1CoreLength;

	// Set GroupContext
	GroupHeadReg & groupReg = CL1ConfigRegs.groupContext();

	groupReg.reset();
	
	if(CL1RIDLLength != 0){
		groupReg.setRIDLSum(CL1RIDLLength);
		groupReg.setRIDLEnable(true);
	}

	groupReg.setRCASum(CL1CoreLength);

	// Generate Hardware registors

	Regs.push_back(groupReg.reg());

	for(int i =0; i <CL1RIDLLength; ++i)
		Regs.push_back( CL1ConfigRegs.RIDLContext(i).reg() );

	for(int i =0; i <CL1CoreLength; ++i){

		Regs.push_back( CL1ConfigRegs.RCContext(i).reg() );
		Regs.push_back( CL1ConfigRegs.CEDLContext(i).reg() );
		Regs.push_back( CL1ConfigRegs.CIDLContext(i).reg() );
		Regs.push_back( CL1ConfigRegs.CDSContext(i).reg() );
	}

	return Regs;
}
