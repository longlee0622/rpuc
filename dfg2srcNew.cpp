////////////////////////////////////////////////////////////
//				       ESLCompiler
//==========================================================
//
// Author: Xie L.   Time: 2011
// Department: MicroE of Shanghai Jiao Tong University
//----------------------------------------------------------
// This file generates an executable file for ESL project,
// which needs a DFG file as input and then generate a 
// C code file executed in ESL platform.
//
////////////////////////////////////////////////////////////

#include "dfgraph.h"
#include "rpucfg.h"
#include "optmz.h"
#include "split.h"

#include <iomanip>
#include <string>
#include <algorithm>
#include <cctype>
//#include "remap.h"


#include <iostream>
#include <fstream>

int totalRCA = 0;
int CL0Size = 0;
extern int remapSeqNo;
extern int PseudoRCANum;

int main(int argc, char *argv[])  
{
	using std::cout;
	using std::cerr;
	using std::endl;

	int err = 0;

	char * MultifileName = "dfg.list";

	//char *  MultifileName = argv[1];

	std::ifstream multiDfgFile(MultifileName);
	if(!multiDfgFile)
	{
		cerr<<"Can't open file data.txt'\""<<MultifileName<<"\""<<endl;
		return -1;
	}

	String total_patchFile = "dfg_patch.h";

	std::ofstream allPatchfile(total_patchFile.c_str());

	assert(allPatchfile);

	while(!multiDfgFile.eof())
	{
		String RPUSeq;
		int RPUSeqNo;
		int DFGroupNumber;//��ǰRPU��DFG Group������
		int DFGroupNum;   //��ǰRPU��DFG Group�������׵ı��
		int onRCANum[4];       //��ǰDFG���õ�RCA���
		//String DFGName;   //DFGͼ������
		const char * DFGList[100][100];
		int curGCGMAddr = 0;
		int RCAformerNumTemp = 0; //��ǰRPU�Ѿ�ӳ���RCA������
		int GroupRCANum;          //��ǰ���е�RCA����


		RPUConfig configTotal;

		Vector<reg32> CL0ContextTemp;
		Vector<Vector<reg32> > CL1ContextTemp;
		Vector<Vector<reg32> > CL2ContextTemp;



		multiDfgFile >> RPUSeq;

		char a[4];
		memset(a,'\0',4);

		for(int zz = 0; zz < 4; zz++)
		{
			a[zz]=RPUSeq[zz];
		}
		//ʹ�õ�RPU�ı��
		RPUSeqNo = std::atoi(& a[3]);

		allPatchfile<<"#include \"RPU"<<RPUSeqNo<<"_CWI.h\"\n\n\n";

		String CL0FileName;
		String CL1FileName;
		String CL2FileName;


		if(RPUSeqNo == 0)
		{
			CL0FileName ="RPU0_CWI.h";

			CL1FileName ="RPU0_GCGM.bin";

			CL2FileName ="RPU0_GCCM.bin";
		}
		else
		{
			CL0FileName ="RPU1_CWI.h";

			CL1FileName ="RPU1_GCGM.bin";

			CL2FileName ="RPU1_GCCM.bin";
		}


		std::ofstream CL0file(CL0FileName.c_str());

		assert(CL0file);

		//��ǰRPU��DFG Group������
		multiDfgFile >> DFGroupNumber;


		for(int ll = 0; ll < DFGroupNumber; ll++)   //��������DFG Group
		{

			String DFGName[4];   //DFGͼ������

			//��ǰ������
			multiDfgFile >> DFGroupNum;

			//��ǰRPU Group��RCA������
			multiDfgFile >> GroupRCANum;



			//for(int cc = 0; cc < 4; cc++)    //ÿ��Group����4��RCA
			for(int cc = 0; cc < GroupRCANum; cc++)    //ÿ��Group����GroupRCANum��RCA
			{
				multiDfgFile >> onRCANum[cc];
				if(onRCANum[cc] == 00)
					onRCANum[cc] = 0;
				else if(onRCANum[cc] == 01)
					onRCANum[cc] = 1;
				else if(onRCANum[cc] == 10)
					onRCANum[cc] = 2;
				else
					onRCANum[cc] = 3;

				multiDfgFile >> DFGName[cc];

				DFGList[DFGroupNum][cc] = DFGName[cc].c_str();
			}

			for(int hh = 0; hh < GroupRCANum; hh++)
			{

				String DFGNameTemp1 = DFGName[hh]+".dfg";

				const char * DFGNameTemp2 = DFGNameTemp1.c_str();

				std::ifstream dfgFile(DFGNameTemp2);

				if(!dfgFile)
				{
					//cerr<<"Can't open file \""<<DFGList[DFGroupNum][hh]<<"\""<<endl;
					continue;   //��ǰRCA�����κι��ܣ�������һ��DFGͼ�Ķ�ȡ
				}

				RPUConfig config;

				int DFGBaseInAddress;


				if(RPUSeqNo == 0)
				{
					if(onRCANum[hh] == 0)  DFGBaseInAddress=0;
					else if(onRCANum[hh] == 1)  DFGBaseInAddress=1024;
					else if(onRCANum[hh] == 2)  DFGBaseInAddress=2048;
					else DFGBaseInAddress = 3072;
				}
				else
				{
					if(onRCANum[hh] == 0)  DFGBaseInAddress=4096;
					else if(onRCANum[hh] == 1)  DFGBaseInAddress=5120;
					else if(onRCANum[hh] == 2)  DFGBaseInAddress=6144;
					else DFGBaseInAddress = 7168;
				}

				config.setRPUGroupNum(DFGroupNum);
				config.setGroupRCANumber(GroupRCANum);
				config.setRCANumbefore(RCAformerNumTemp);
				config.setDFGInBaseAddress(DFGBaseInAddress);
				config.setcurGCGMBaseAddr(curGCGMAddr);  //����CL0�������Ӧ��CL1���׵�ַ
				config.setRPUNum(RPUSeqNo);
				config.setRCANum(onRCANum[hh]);
				//config.pasteCL0Context(CL0ContextTemp);
				config.pasteCL1Context(CL1ContextTemp);
				config.pasteCL2Context(CL2ContextTemp);

				for(int zz = 0; zz < GroupRCANum; zz++)
				{
					config.setlocateDFGroupList(DFGList[DFGroupNum][zz],zz);
				}

				err |= config.graph().parse(dfgFile);
				if(err)
				{
					cerr<<"Bad DFG file format!"<<endl;
					return -1;
				}


				const String DFGraphName = config.graph().name();

				const String patchFile = DFGraphName + "_patch.h";

				allPatchfile<<
					"#include \""<<patchFile<<"\"\n";


				err |= config.createPatchFile(patchFile);

				err |= config.mapDFGraph(OptimizeMethod(), SplitMethod());
				err |= config.genContext();

				RCAformerNumTemp = config.RCANumBefore();

				curGCGMAddr=config.curGCGMBaseAddr();


				//create *_interface.h file
				/**************************** start ***************************************************/			
				//for each RCA SSRAM region
				//   baseAddr+0    ---------------------------------------start
				//                             DFG BaseInData                        
				//   baseAddr+191  ---------------------------------------end
				//   baseAddr+192  ---------------------------------------start
				//                             TempOutData
				//
				//   baseAddr+815  ---------------------------------------end
				//   baseAddr+816  ---------------------------------------start
				//                             DFG BaseOutData
				//   baseAddr+1023 ---------------------------------------end

				/*
				   RPU 0:   RCA0 baseAddr = 0;
				   RCA1 baseAddr = 1024;
				   RCA2 baseAddr = 2048;
				   RCA3 baseAddr = 3072;
				   RPU 1:   RCA0 baseAddr = 4096;
				   RCA1 baseAddr = 5120;
				   RCA2 baseAddr = 6144;
				   RCA3 baseAddr = 7168;
				 */

				if(RPUSeqNo == 0)
				{
					if(onRCANum[hh] == 0)
					{
						int SSRAMInBaseAddr = 0;
						int SSRAMOutBaseAddr = 816;
					}
					else if(onRCANum[hh] == 1)
					{
						int SSRAMInBaseAddr = 1024;
						int SSRAMOutBaseAddr = 1840;
					}
					else if(onRCANum[hh] == 2)
					{
						int SSRAMInBaseAddr = 2048;
						int SSRAMOutBaseAddr = 2864;
					}
					else
					{
						int SSRAMInBaseAddr = 3072;
						int SSRAMOutBaseAddr = 3888;
					}
				}
				else
				{
					if(onRCANum[hh] == 0)
					{
						int SSRAMInBaseAddr = 4096;
						int SSRAMOutBaseAddr = 4920;
					}
					else if(onRCANum[hh] == 1)
					{
						int SSRAMInBaseAddr = 5120;
						int SSRAMOutBaseAddr = 5944;
					}
					else if(onRCANum[hh] == 2)
					{
						int SSRAMInBaseAddr = 6144;
						int SSRAMOutBaseAddr = 6968;
					}
					else
					{
						int SSRAMInBaseAddr = 7168;
						int SSRAMOutBaseAddr = 7992;
					}
				}

				const String SSRAM_interface =config.graph().name()+"_interface.h";


				std::ofstream SSRAMinterfaceFile(SSRAM_interface.c_str());
				assert(SSRAMinterfaceFile);

				//SSRAMinterfaceFile<<config.graph().name()<<"\n\n\n";


				DFGraph dfg_graph;
				dfg_graph = config.graph();

				String graphName=dfg_graph.name();


				//Vector<DFGPort> & DFGInports =  (*dfg_graph).inports();
				//Vector<DFGPort> & DFGOutports = (*dfg_graph).outports();
				int inportSize = dfg_graph.inSize();
				int outportSize = dfg_graph.outSize();
				int z;


				String portName;
				int immPortValue;
				int  portSSRAM;
				//Vector<DFGPort>::iterator inportIter;
				//Vector<DFGPort>::iterator outportIter;
				DFGPort * currInport;
				DFGPort * currOutport;
				DFGVarPort * varPort;
				DFGImmPort * immPort;

				char tempUperString[256];
				strcpy(tempUperString,graphName.c_str());
				int graphNamelength,i;
				graphNamelength=strlen(tempUperString);
				for(i=0;i<graphNamelength;i++)
				{
					tempUperString[i]=toupper(tempUperString[i]);
				}
				
				SSRAMinterfaceFile<<
					"#define "<<tempUperString<<"_DIN \\\n";

				//SSRAMinterfaceFile<<"SSRAMInBaseAddr = "<<std::hex<<SSRAMInBaseAddr<<"\n\n";

				//for(inportIter = DFGInports.begin(); inportIter != DFGInports.end(); inportIter++)
				for(z=0; z< inportSize ; z++)
				{
					currInport = (&dfg_graph.inport(z));
					if(!(currInport->isImmPort()))
					{
						varPort = static_cast<DFGVarPort*>(currInport);
						portName = (*varPort).name();

						int mm, last,newCopy;
						int nameSize = portName.size();
						const char *a = portName.c_str();
						for( mm = 0; mm < nameSize; mm++)
						{
							if(a[mm] == '.')
								break;
						}
						last = nameSize-mm-1;
						mm++;
						char newChar[50];
						String portNameNew;

						for(newCopy = 0; newCopy < last ; newCopy++,mm++)
						{
							newChar[newCopy] = a[mm];
						}
						newChar[last]='\0';

						portNameNew = newChar;

						portSSRAM = (*currInport).SSRAMAddress();
						SSRAMinterfaceFile<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<") = (short)"<<portNameNew<<";\\\n";
					}
					else
					{
						immPort = static_cast<DFGImmPort*>(currInport);
						immPortValue = immPort->value();
						portSSRAM = (*currInport).SSRAMAddress();
						SSRAMinterfaceFile<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<") = (short)"<<std::dec<<immPortValue<<";\\\n";
					}
				}

				SSRAMinterfaceFile<<"\n\n\n";

				SSRAMinterfaceFile<<"#define "<<tempUperString<<"_DOUT \\\n";
				//for(outportIter = DFGOutports.begin(); outportIter != DFGOutports.end() ; outportIter++)
				for(z=0; z< outportSize ; z++)
				{
					currOutport = (&dfg_graph.outport(z));
					varPort = static_cast<DFGVarPort*>(currOutport);
					portName = (*varPort).name();

					int mm, last,newCopy;
					int nameSize = portName.size();
					const char *a = portName.c_str();
					for( mm = 0; mm < nameSize; mm++)
					{
						if(a[mm] == '.')
							break;
					}
					last = nameSize-mm-1;
					mm++;
					char newChar[50];
					String portNameNew;

					for(newCopy = 0; newCopy < last ; newCopy++,mm++)
					{
						newChar[newCopy] = a[mm];
					}
					newChar[last]='\0';	
					portNameNew = newChar;


					portSSRAM = (*currOutport).SSRAMAddress();
					//SSRAMinterfaceFile<<portName<<" = "<<"*(RP)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<");"<<"\\\n";
					SSRAMinterfaceFile<<portNameNew<<" = "<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<");"<<"\\\n";
				}

				/*
				SSRAMinterfaceFile <<
					"\n\n\n"
					"#define "<< tempUperString<<"_FLAG\t1\n\n"
					"#define "<< tempUperString<<"_VAR_DEC \\\n\n";
					*/

				SSRAMinterfaceFile<<std::endl;

				/**************************** end ***************************************************/

				Vector<reg32> curCL0 = config.CL0ContextCopy();
				CL0ContextTemp.insert(CL0ContextTemp.end(),curCL0.begin(),curCL0.end());
				//start write CWI file

				/*std::size_t cl0size = config.CL0ContextCopy().size();

				CL0file<<"[CWIPACKET]\n"<<"Config Word Cnt="<<std::dec<<cl0size<<"\n";

				CL0ContextTemp = config.CL0ContextCopy();

				CL0file.fill('0');

				for(std::size_t i =0; i <cl0size ; ++i)
					//CL0file<<"\t\t\t"<<"ConfigWord["<<std::dec<<i<<"]=0x"<<std::setw(8)<<std::hex<<CL0ContextTemp[i]<<";\n";
					CL0file<<"ConfigWord["<<std::dec<<CL0Size+i<<"]=0x"<<std::setw(8)<<std::hex<<CL0ContextTemp[i]<<";\n";

				CL0file<<
					"\n\n\n\n";

					*/


				//CL0ContextTemp = config.CL0ContextCopy();
				CL1ContextTemp = config.CL1ContextCopy();
				CL2ContextTemp = config.CL2ContextCopy();
				totalRCA += config.allRCAs().size();
				remapSeqNo = 0;
				PseudoRCANum =0;
				CL0Size += CL0ContextTemp.size();
			}

			CL0file<<"#define GROUP"<<DFGroupNum<<"_CWI \\\n";
			std::size_t cl0size = CL0ContextTemp.size();

			CL0file.fill('0');

			for(std::size_t i =0; i <cl0size ; ++i)
				CL0file<<"\t\t\twrite_reg(AHB1_S1_RPU0,0x"<<std::setw(8)<<std::hex<<CL0ContextTemp[i]<<");\\\n";

			CL0file<<"\n\n\n\n";
			CL0ContextTemp.clear();
		}

		//configTotal.pasteCL0Context(CL0ContextTemp);
		configTotal.pasteCL1Context(CL1ContextTemp);
		configTotal.pasteCL2Context(CL2ContextTemp);

		//err |= configTotal.createCL0File(CL0FileName);
		err |= configTotal.createCL1File(CL1FileName);
		err |= configTotal.createCL2File(CL2FileName);

		//end of CL0file
		CL0file <<std::endl;

		//����RPU1����Ϣ

		//To do...
		allPatchfile <<"\n\n";

		if( RPUSeqNo == 1) break;
	}
	allPatchfile<<std::endl;
}
































