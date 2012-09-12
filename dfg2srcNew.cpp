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
extern int createInterface(const String & fileName, RPUConfig &config);

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

		Vector<Vector<reg32> > CL0ContextTemp;
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

		if( RPUSeqNo == 1) break;

		allPatchfile<<"#include \"RPU"<<RPUSeqNo<<"_CWI.h\"\n\n\n";

		String CL0FileName;
		String CL1FileName;
		String CL2FileName;
		String C_CL0_Name;
		String C_CL1_Name;
		String C_CL2_Name;


		if(RPUSeqNo == 0)
		{
			CL0FileName ="RPU0_CWI.h";

			C_CL0_Name ="GROUP0_CWI.txt";

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


		//std::ofstream CL0file(CL0FileName.c_str());

		//assert(CL0file);

		//当前RPU的DFG Group的套数
		multiDfgFile >> DFGroupNumber;


		std::ofstream CL0File(CL0FileName.c_str());
		assert(CL0File);

		for(int ll = 0; ll < DFGroupNumber; ll++)   //遍历多套DFG Group
		{

			allPatchfile<<"#include \"GROUP"<<ll<<"_link.h\""<<std::endl;
			char buf[20];
			sprintf(buf,"GROUP%d_link.h",ll);
			std::ofstream GRPLink(buf);
			GRPLink<<"#define GROUP"<<ll<<"_CWI \\"<<std::endl;
			String DFGName[4];   //DFG图的名称
			String DFGLibPath[4]; //DFG模板库路径

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

				String DFGLine;
				String::size_type libFound;

				getline(multiDfgFile, DFGLine);
				libFound = DFGLine.find("LIB:");
				std::istringstream stream(DFGLine);
				if (libFound != String::npos)
				{
					stream >> DFGLibPath[onRCANum[cc]] >> DFGLibPath[onRCANum[cc]];
				}


				DFGList[DFGroupNum][onRCANum[cc]] = DFGName[onRCANum[cc]].c_str();
			}

			for(int hh = 0; hh < GroupRCANum; hh++)
			{

				RPUConfig config;

				config.setLibPath(DFGLibPath[hh]);
				if (config.getLibPath().empty())
				{
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

					mapFile.close();
				}


				//const int loopTime = config.getLoopTime();

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
				//config.pasteC_CL0Context(C_CL0ContextTemp);
				//config.pasteCL0Context(CL0ContextTemp);
				config.pasteCL1Context(CL1ContextTemp);
				config.pasteCL2Context(CL2ContextTemp);

				if (config.getLibPath().empty())
				{
					String DFGNameTemp1 = DFGName[hh]+".dfg";

					const char * DFGNameTemp2 = DFGNameTemp1.c_str();

					std::ifstream dfgFile(DFGNameTemp2);

					if(!dfgFile)
					{
						//cerr<<"Can't open file \""<<DFGList[DFGroupNum][hh]<<"\""<<endl;
						continue;   //当前RCA不做任何功能，继续下一个DFG图的读取
					}

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

					const String interfaceFileName(DFGName[hh]);
					createInterface(interfaceFileName, config);
					totalRCA += config.allRCAs().size();
				}
				else {
					const String DFGraphName = DFGName[hh];
					const String patchFile = DFGraphName + "_patch.h";
					allPatchfile<<
						"#include \""<<patchFile<<"\"\n";
					//totalRCA = config.libGenContext(config.getLibPath(), DFGraphName, totalRCA, curGCGMAddr);
					curGCGMAddr = config.libGenContext(config.getLibPath(), DFGraphName, curGCGMAddr);
					
				}

				Vector<Vector<reg32> > curCL0 = config.CL0ContextCopy();
				for(Vector<Vector<reg32> >::iterator CL0GrpIter = curCL0.begin(); CL0GrpIter != curCL0.end(); ++CL0GrpIter)
				{
					Vector<reg32> curCtx = * CL0GrpIter;
					CL0File<<"#define GROUP"<<config.RPUGroupNumber()<<"_"<<config.onRCANumber()<<"_"<<(CL0GrpIter-curCL0.begin())<<"_CWI\\\n";
					char buf[30];
					sprintf(buf,"GROUP%d_%d_%d_CWI",config.RPUGroupNumber(),config.onRCANumber(),CL0GrpIter-curCL0.begin());
					GRPLink<<"\t\t\t"<<buf<<"\\"<<std::endl;
					GRPLink<<"\t\t\twhile(!RPU0_done){}\\"<<std::endl;
					GRPLink<<"\t\t\tRPU0_done = 0;\\"<<std::endl;
					CL0File.fill('0');
					for(Vector<reg32>::iterator CtxIter = curCtx.begin(); CtxIter != curCtx.end(); ++CtxIter)
					{
						CL0File<<"\t\t\twrite_reg(AHB1_S1_RPU0,0x"<<std::setw(8)<<std::hex<<(*CtxIter)<<");\\\n";
					}
					
					if (config.getLibPath().empty()){
						CL0File<<"\t\t\twrite_reg(AHB1_S1_RPU0,0x0000000"<<std::setw(1)<<std::hex<<config.onRCANumber()<<");\\\n";
						CL0File<<"\t\t\twrite_reg(AHB1_S1_RPU0,0x00008000);\n\n\n\n\n";
					}
				}

				C_CL0ContextTemp = config.C_CL0ContextCopy();
				CL0ContextTemp = config.CL0ContextCopy();
				CL1ContextTemp = config.CL1ContextCopy();
				CL2ContextTemp = config.CL2ContextCopy();
				
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

		//configTotal.pasteC_CL0Context(C_CL0ContextTemp);
		//configTotal.pasteCL0Context(CL0ContextTemp);
		configTotal.pasteCL1Context(CL1ContextTemp);
		configTotal.pasteCL2Context(CL2ContextTemp);

		if( RPUSeqNo != 1)
		{
			//err |= configTotal.createCL0File(CL0FileName,C_CL0_Name);
			err |= configTotal.createCL1File(CL1FileName,C_CL1_Name);
			err |= configTotal.createCL2File(CL2FileName,C_CL2_Name);
		}
		//end of CL0file


		//处理RPU1的信息

		//To do...
		allPatchfile <<"\n\n";

		//if( RPUSeqNo == 1) break;
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
































