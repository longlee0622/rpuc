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


#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

int totalRCA = 0;
int CL0Size = 0;
int ConstMem[16];		//2012.5.7 longlee 模拟一个常数存储器

extern int remapSeqNo;
extern int PseudoRCANum;

int main(int argc, char *argv[])  
{
	using std::cout;
	using std::cerr;
	using std::endl;

	int err = 0;
	for (int i= 0;i <16;++i) ConstMem[i]=-1;

	char * MultifileName = "dfg.list";
	
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
		int DFGroupNumber;//当前RPU的DFG Group的套数
		int DFGroupNum;   //当前RPU的DFG Group的所在套的编号
		int onRCANum[4];       //当前DFG所用的RCA编号
		const char * DFGList[100][100];
		int curGCGMAddr = 0;
		int RCAformerNumTemp = 0; //当前RPU已经映射的RCA的数量
		int GroupRCANum;          //当前组中的RCA数量


		RPUConfig configTotal;

		Vector<reg32> CL0ContextTemp;
		Vector<Vector<reg32> > C_CL0ContextTemp;
		Vector<Vector<reg32> > CL1ContextTemp;
		Vector<Vector<reg32> > CL2ContextTemp;



		multiDfgFile >> RPUSeq;

		char a[4];
		memset(a,'\0',4);

		for(int zz = 0; zz < 4; zz++)
		{
			a[zz]=RPUSeq[zz];
		}
		//使用的RPU的编号
		RPUSeqNo = std::atoi(& a[3]);

		if(RPUSeqNo == 0)
		{
		allPatchfile<<"#include \"RPU"<<RPUSeqNo<<"_CWI.h\"\n\n\n";
		}				
		String CL0FileName;
		String CL1FileName;
		String CL2FileName;
		String C_CL0_Name;
		String C_CL1_Name;
		String C_CL2_Name;


		if(RPUSeqNo == 0)
		{
			CL0FileName ="RPU0_CWI.h";

			C_CL0_Name ="CWIPacket.txt";

			CL1FileName ="ContextGroupMem_ini.txt_Data.data";

			C_CL1_Name ="ContextGroupMem_ini.txt";

			CL2FileName ="CoreContextMem_ini.txt_Data.data";

			C_CL2_Name = "CoreContextMem_ini.txt";
		}
		else
		{
			break;	//对于RPU1直接跳出
			CL0FileName ="RPU1_CWI.h";

			CL1FileName ="RPU1_GCGM.bin";

			CL2FileName ="RPU1_GCCM.bin";
		}


		std::ofstream CL0file(CL0FileName.c_str());

		assert(CL0file);

		//当前RPU的DFG Group的套数
		multiDfgFile >> DFGroupNumber;


		for(int ll = 0; ll < DFGroupNumber; ll++)   //遍历多套DFG Group
		{

			allPatchfile<<"#include \"GROUP"<<ll<<"_link.h\""<<std::endl;
			char buf[20];
			sprintf(buf,"GROUP%d_link.h",ll);
			std::ofstream GRPLink(buf);
			GRPLink<<"#define GROUP"<<ll<<"_CWI \\"<<std::endl;
			String DFGName[4];   //DFG图的名称

			//当前组的组号
			multiDfgFile >> DFGroupNum;

			//当前RPU Group中RCA的数量
			multiDfgFile >> GroupRCANum;



			//for(int cc = 0; cc < 4; cc++)    //每个Group都有4个RCA
			for(int cc = 0; cc < GroupRCANum; cc++)    //每个Group都有GroupRCANum个RCA
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

				multiDfgFile >> DFGName[onRCANum[cc]];

				DFGList[DFGroupNum][onRCANum[cc]] = DFGName[onRCANum[cc]].c_str();
			}

			for(int hh = 0; hh < GroupRCANum; hh++)
			{

				String DFGNameTemp1 = DFGName[hh]+".dfg";
				
				std::cout<<DFGNameTemp1<<std::endl;	
				const char * DFGNameTemp2 = DFGNameTemp1.c_str();

				std::ifstream dfgFile(DFGNameTemp2);

				if(!dfgFile)
				{
					//cerr<<"Can't open file \""<<DFGList[DFGroupNum][hh]<<"\""<<endl;
					continue;   //当前RCA不做任何功能，继续下一个DFG图的读取
				}

				RPUConfig config;

				//const char * underline = "__"; 
				//const char * mapPostfix = "_formal_map.h\0";
				//char fileNameTemp1[50];
				//int fileNamePos = DFGNameTemp1.find(underline, 0);
				//strncpy(fileNameTemp1, DFGNameTemp1.c_str(), fileNamePos);
				//std::string fileNameTemp2 = fileNameTemp1;
				//fileNameTemp2.replace(fileNamePos, 13, "_formal_map.h");
				//fileNameTemp2 = fileNameTemp2 + "_formal_map.h";
				//int fileNameLength = fileNamePos + 13;
				//fileNameTemp2.erase(fileNameLength);
				String mapNameTemp = DFGName[hh] + "_formal_map.h";
				const char * mapFileName = mapNameTemp.c_str();
				std::ifstream mapFile(mapFileName);
				if(!mapFile)
				{
					cerr<<"Can't open file data.txt'\""<<mapFileName<<"\""<<endl;
					return -1;
				}

				int mapLoopTime = 0;
				std::string line;
				std::string tempLoopData;
				getline(mapFile, line);
				std::istringstream stream(line);
				while (stream>>tempLoopData)
				{
					mapLoopTime++;
				}

				mapLoopTime = mapLoopTime - 2;
				config.setLoopTime(mapLoopTime);


				const int loopTime = config.getLoopTime();


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
				config.setcurGCGMBaseAddr(curGCGMAddr);  //用于CL0生成相对应的CL1的首地址
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


				err |= config.mapDFGraph(SplitMethod());
				err |= config.genContext();

				err |= config.createPatchFile(patchFile);

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
				   RPU 0:   
				   RCA0 baseAddr = 0;
				   RCA1 baseAddr = 1024;
				   RCA2 baseAddr = 2048;
				   RCA3 baseAddr = 3072;
				   RPU 1:   
				   RCA0 baseAddr = 4096;
				   RCA1 baseAddr = 5120;
				   RCA2 baseAddr = 6144;
				   RCA3 baseAddr = 7168;
				 */
				//RCA * ptr =config.rcas().begin()->get();
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
				/******************************************************************************************/
				// Read map.h
				int inNum = 0;
				int outNum = 0;
				char * equal = "_scalar";
				DFGraph dfg_graph;
				dfg_graph = config.graph();

				String graphName=dfg_graph.name();

				int inportSize = dfg_graph.inSize();
				int outportSize = dfg_graph.outSize();

				
				std::string tempScalar[32];
				std::string **In = new std::string *[inportSize];
				for(int j=0; j<inportSize; j++)
				{
					In[j] = new std::string[loopTime];
				}

				std::string **Out = new std::string *[outportSize];
				for(int j=0; j<outportSize; j++)
				{
					Out[j] = new std::string[loopTime];
				}
				

				do {
					if (line == "") continue;
					std::istringstream stream(line);
					for (int i=0; i<(loopTime+2); i++)
					{
						stream>>tempScalar[i];
					}

					if (strncmp(tempScalar[0].data(), equal,7) == 0)
					{
						for (int j=0; j<loopTime; j++)
						{
							In[inNum][j] = tempScalar[j+2];
						}
						inNum++;
					}
					else if (strncmp(tempScalar[loopTime+1].data(), equal,7) == 0)
					{
						for (int j=0; j<loopTime; j++)
						{
							Out[outNum][j] = tempScalar[j];
						}
						outNum++;
					}
					else std::cout<<"Incorrect!"<<std::endl;
				}
				while (getline(mapFile, line));

				const String SSRAM_interface =config.graph().name()+"_interface.h";


				std::ofstream SSRAMinterfaceFile(SSRAM_interface.c_str());
				assert(SSRAMinterfaceFile);

				String portName;
				int  portSSRAM;
				DFGPort * currInport;
				DFGPort * currOutport;
				DFGVarPort * varPort;
	
				int z;
				int scalarInNum;
				int ssramOutTopAddr, ssramOutBaseAddr;


				RCA * ssramInPtr = config.rcas().begin()->get();
				ssramOutTopAddr = config.rcas().back()->rcaSSRAMOutTopAddr();
				ssramOutBaseAddr = config.rcas().back()->rcaSSRAMOutBaseAddr();
			/*	RCA ssramOutTopSize =config.rcas().back()->rcaSSRAMOutBaseAddr();*/
				int ssramInSize = ssramInPtr->rcaSSRAMInTopAddr() - ssramInPtr->rcaSSRAMInBaseAddr();
				int ssramOutSize = ssramOutTopAddr - ssramOutBaseAddr;
			/*	ssramInPtr = ssramInPtr + config.allRCAs().size()-1;*/
	/*			int ssramOutTopSize = ssramOutPtr->rcaSSRAMOutTopAddr();
				int	ssramOutBaseSize =  ssramOutPtr->rcaSSRAMOutBaseAddr();*/
				


				char tempUperString[256];
				strcpy(tempUperString,graphName.c_str());
				int graphNamelength,i;
				graphNamelength=strlen(tempUperString);
				for(int i=0;i<graphNamelength;i++)
				{
					tempUperString[i]=toupper(tempUperString[i]);
				}
				
				SSRAMinterfaceFile<<
					"#define "<<tempUperString<<"_DIN \\\n";


				for (int currLoopTime=0; currLoopTime < loopTime; currLoopTime++)	
				{
					scalarInNum = 0;
					for(z=0; z<inportSize; z++)
					{
						currInport = (&dfg_graph.inport(z));
						if(!(currInport->isImmPort()))
						{
							portName = static_cast<DFGVarPort*>(currInport)->name();
							if (portName.find("scalar") != -1)
							{
								portName = In[scalarInNum][currLoopTime];
								portSSRAM = (*currInport).SSRAMAddress() + currLoopTime * ssramInSize;
								if ((z == inportSize-1) && (currLoopTime == loopTime-1))
									SSRAMinterfaceFile<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<") = (short)"<<portName<<";\n";
								else SSRAMinterfaceFile<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<") = (short)"<<portName<<";\\\n";
								scalarInNum++;
							}
							else
							{
								portName = portName.substr(portName.find(".")+1);
								portSSRAM = (*currInport).SSRAMAddress() + currLoopTime * ssramInSize;
								if ((z == inportSize-1) && (currLoopTime == loopTime-1))
									SSRAMinterfaceFile<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<") = (short)"<<portName<<";\n";
								else SSRAMinterfaceFile<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<") = (short)"<<portName<<";\\\n";
								//scalarInNum++;
							}
							
						}
			
						else
						{
							//2012.5.7 longlee 立即数不进入SSRAM输入区
							//immPort = static_cast<DFGImmPort*>(currInport);
							//immPortValue = immPort->value();
							//portSSRAM = (*currInport).SSRAMAddress();
							//SSRAMinterfaceFile<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<") = (short)"<<std::dec<<immPortValue<<";\\\n";						}
						}
					}
				}

				SSRAMinterfaceFile<<"\n\n\n";
				SSRAMinterfaceFile<<"#define "<<tempUperString<<"_DOUT \\\n";
				for (int currLoopTime=0; currLoopTime < loopTime; currLoopTime++)
				{
					int scalarOutNum = 0;
					for(z=0; z<outportSize ; z++)
					{
						currOutport = (&dfg_graph.outport(z));
						portName = static_cast<DFGVarPort*>(currOutport)->name();
						//portName = Out[z][currLoopTime];
						if (portName.find("scalar") != -1)
						{
							portName = Out[scalarOutNum][currLoopTime];
							portSSRAM = (*currOutport).SSRAMAddress() + currLoopTime * ssramOutSize;
							if ((z==outportSize-1) && (currLoopTime==loopTime-1))
								SSRAMinterfaceFile<<portName<<" = "<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<");"<<"\n";
							else SSRAMinterfaceFile<<portName<<" = "<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<");"<<"\\\n";
							scalarOutNum++;
						}
						else
						{
							portName = portName.substr(portName.find(".")+1);
							portSSRAM = (*currOutport).SSRAMAddress() + currLoopTime * ssramOutSize;
							if ((z==outportSize-1) && (currLoopTime==loopTime-1))
								SSRAMinterfaceFile<<portName<<" = "<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<");"<<"\n";
							else SSRAMinterfaceFile<<portName<<" = "<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<");"<<"\\\n";
						}

					}
				}

				SSRAMinterfaceFile<<std::endl;

				/**************************** end ***************************************************/


				Vector<Vector<reg32> > curCL0 = config.CL0ContextCopy();
				Vector<Vector<reg32> > C_CL0 = config.C_CL0ContextCopy();
				C_CL0ContextTemp.insert(C_CL0ContextTemp.end(),C_CL0.begin(),C_CL0.end());
				for(Vector<Vector<reg32> >::iterator CL0GrpIter = curCL0.begin(); CL0GrpIter != curCL0.end(); ++CL0GrpIter)
				{
					Vector<reg32> curCtx = * CL0GrpIter;
					CL0file<<"#define GROUP"<<std::dec<<config.RPUGroupNumber()<<"_"<<config.onRCANumber()<<"_"<<(CL0GrpIter-curCL0.begin())<<"_CWI\\\n";
					char buf[30];
					sprintf(buf,"GROUP%d_%d_%d_CWI",config.RPUGroupNumber(),config.onRCANumber(),CL0GrpIter-curCL0.begin());
					GRPLink<<"\t\t\t"<<buf<<"\\"<<std::endl;
					GRPLink<<"\t\t\twhile(!RPU0_done){}\\"<<std::endl;
					//GRPLink<<"\t\t\tRPU0_done = 0;\\"<<std::endl;
					GRPLink<<"\t\t\tRPU0_done = 0;"<<std::endl;
					CL0file.fill('0');
					for(Vector<reg32>::iterator CtxIter = curCtx.begin(); CtxIter != curCtx.end(); ++CtxIter)
					{
						CL0file<<"\t\t\twrite_reg(AHB1_S1_RPU0,0x"<<std::setw(8)<<std::hex<<(*CtxIter)<<");\\\n";
					}
					
					CL0file<<"\t\t\twrite_reg(AHB1_S1_RPU0,0x0000000"<<std::setw(1)<<std::hex<<config.onRCANumber()<<");\\\n";
					CL0file<<"\t\t\twrite_reg(AHB1_S1_RPU0,0x00008000);\n\n\n\n\n";
				}
				
				//CL0ContextTemp = config.CL0ContextCopy();
				CL1ContextTemp = config.CL1ContextCopy();
				CL2ContextTemp = config.CL2ContextCopy();
				totalRCA += config.allRCAs().size();
				remapSeqNo = 0;
				PseudoRCANum =0;
				CL0Size += CL0ContextTemp.size();
			}
			char Cbuf[20];
			sprintf(Cbuf,"GROUP%d_CWI.txt",DFGroupNum);
			std::ofstream tempCL0(Cbuf);
			std::size_t cl0cnt = 0;
			
			for(Vector<Vector<reg32> >::iterator CtxGrpIter = C_CL0ContextTemp.begin(); CtxGrpIter != C_CL0ContextTemp.end(); ++ CtxGrpIter)
			{
				Vector<reg32> CtxGrp = * CtxGrpIter;
				cl0cnt += CtxGrp.size();
			}

			tempCL0<<"[CWIPACKET]\n"<<"Config Word Cnt="<<std::dec<<cl0cnt<<"\n";

			tempCL0.fill('0');

			std::size_t ctxCnt = 0;
			for(Vector<Vector<reg32> >::iterator CtxGrpIter = C_CL0ContextTemp.begin(); CtxGrpIter != C_CL0ContextTemp.end(); ++ CtxGrpIter)
			{
				Vector<reg32> CtxGrp = * CtxGrpIter;
				for(Vector<reg32>::iterator CtxIter = CtxGrp.begin(); CtxIter != CtxGrp.end(); ++CtxIter)
				{
					tempCL0<<"ConfigWord["<<std::dec<<ctxCnt<<"]=0x"<<std::setw(8)<<std::hex<<(*CtxIter)<<";\n";
					ctxCnt++;
				}
			}

			tempCL0<<"\n\n\n\n";
			tempCL0.close();
			C_CL0ContextTemp.clear();
		}

		//configTotal.pasteCL0Context(CL0ContextTemp);
		configTotal.pasteCL1Context(CL1ContextTemp);
		configTotal.pasteCL2Context(CL2ContextTemp);

		if( RPUSeqNo != 1)
		{
			//err |= configTotal.createCL0File(CL0FileName);
			err |= configTotal.createCL1File(CL1FileName,C_CL1_Name);
			err |= configTotal.createCL2File(CL2FileName,C_CL2_Name);
		}
		//end of CL0file
		CL0file <<std::endl;
		CL0file.close();

		//处理RPU1的信息

		//To do...
		allPatchfile <<"\n\n";

		if( RPUSeqNo == 1) break;
	}
	allPatchfile<<std::endl;
	FILE* CMfile=fopen("ConstMem_ini.txt","w");
	fprintf(CMfile,"[Const Memory]\nConst Cnt=4\n");
	for (int i = 0;i < 4;++i)
	{
		fprintf(CMfile,"Const[%d]=[0x%02x]0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",i,i*8,
			ConstMem[i*4+0]&0xff,(ConstMem[i*4+0]>>8)&0xff,ConstMem[i*4+1]&0xff,(ConstMem[i*4+1]>>8)&0xff,
			ConstMem[i*4+2]&0xff,(ConstMem[i*4+2]>>8)&0xff,ConstMem[i*4+3]&0xff,(ConstMem[i*4+3]>>8)&0xff);
	}
	fclose(CMfile);
	//system("pause");
}
































