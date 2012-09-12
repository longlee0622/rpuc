////////////////////////////////////////////////////////////
//				       ESLCompiler
//==========================================================
//
// Author: TangSZ   Time: 2012
// Department: MicroE of Shanghai Jiao Tong University
//----------------------------------------------------------
// This file read the CL0, CL1, CL2 file from model library
// to generate new configure words for ESL project. 
// 
//
////////////////////////////////////////////////////////////
#include "dfgraph.h"
#include "rpucfg.h"

#include <iomanip>
#include <string>
#include <algorithm>
#include <iterator>
#include <cctype>
#include <bitset>

#include <iostream>
#include <fstream>
#include <sstream>

extern int totalRCA;
extern int ConstMem[];

int RPUConfig::libGenContext(const String & fileLibPath, const String & DFGraphName, int curGCGMAddr){

	using std::endl;
	using std::cerr;

	String lib_C_CL0Name = fileLibPath+"_GROUP0_CWI.txt";
	String libCL0Name = fileLibPath+"_RPU0_CWI.h";
	String libCL1Name = fileLibPath+"_ContextGroupMem_ini.txt";
	String libCL2Name = fileLibPath+"_CoreContextMem_ini.txt";
	String libInterfaceName = fileLibPath+"_interface_update.h";
	String libConstMemName = fileLibPath+"_ConstMem_ini.txt";

	std::ifstream libConstMemFile(libConstMemName.c_str());
	if(!libConstMemFile)
	{
		cerr<<"Can't open file ConstMem_ini.txt'\""<<libConstMemName<<"\""<<endl;
		return -1;
	}

	unsigned short libConstMem[16];
	String ConstMemLine;
	int ConstMemCnt = 0;
	while( getline(libConstMemFile,  ConstMemLine))
	{
		String::size_type ConstMemPos = ConstMemLine.find("]0x");
		if ( ConstMemPos != String::npos )
		{
			std::istringstream ConstMemStr(ConstMemLine.substr(ConstMemPos+1));
			for (int i=0; i<4; i++)
			{
				unsigned short lowBits, highBits;
				ConstMemStr >> std::hex >> lowBits >> std::hex >> highBits;
				libConstMem[ConstMemCnt*4+i] = (highBits<<8)+lowBits;
			
			}
			ConstMemCnt++;
		}
	}

	int DFGroupNum = RPUGroupNumber();
	int RCAformerNumTemp = totalRCA;
	//int RCAformerNumTemp = RCANumBefore();
	//int curGCGMAddr = curGCGMBaseAddr();

	const String patchFileName = DFGraphName + "_patch.h";

	std::ofstream patchFile(patchFileName.c_str());
	assert(patchFile);

	patchFile << "#include \"" << DFGraphName << "_interface.h\"" << endl;
	patchFile << "\n\n\n";
	patchFile << "#define ";

	String tempUpperName(DFGraphName);
	for (std::string::size_type i=0; i!=DFGraphName.size(); i++)
		tempUpperName[i] = toupper(DFGraphName[i]);

	patchFile << tempUpperName << "_FUNC \\" << endl;
	patchFile << "\t\t\t" << tempUpperName << "_DIN\\" << endl;
	patchFile << "\t\t\t" << "GROUP" << DFGroupNum << "_CWI\\" << endl;
	patchFile << "\t\t\t" << tempUpperName << "_DOUT" << endl;

	patchFile.close();

	//allPatchfile<<
	//	"#include \""<<patchFileName<<"\"\n";


	////////////////////////////////////////////////////////////
	//Update Interface
	std::ifstream interfaceIn(libInterfaceName.c_str(),std::ios::binary);
	if(!interfaceIn)
	{
		cerr<<"Can't open file interface_update.h'\""<<libInterfaceName<<"\""<<endl;
		return -1;
	}
	String interfaceOutName = DFGraphName+"_interface.h";
	std::ofstream interfaceOut(interfaceOutName.c_str(),std::ios::binary);
	assert(interfaceOut);

	interfaceOut << interfaceIn.rdbuf();
					
	interfaceIn.close();
	interfaceOut.close();

	////////////////////////////////////////////////////////////
	// Read C_CL0 (GROUP0_CWI.txt) file
	std::ifstream lib_C_CL0File(lib_C_CL0Name.c_str());
	if(!lib_C_CL0File)
	{
		cerr<<"Can't open file GROUP0_CWI.txt'\""<<lib_C_CL0Name<<"\""<<endl;
		return -1;
	}

	// Store C_CL0 to C_CL0ContextTemp
	Vector<reg32> C_CL0vec;

	std::string CLline;
	while(getline(lib_C_CL0File, CLline))
	{
		std::string::size_type reg32Pos;
		reg32Pos = CLline.find("=0x");
		if (reg32Pos!=std::string::npos)
		{
			std::istringstream C_CL0_CWstr(CLline.substr(reg32Pos+3, 8));
			reg32 C_CL0_CW;
			C_CL0_CWstr >> std::hex >> C_CL0_CW;
			if (CLline.find("ConfigWord[1]")!=std::string::npos)
			{
				// 2012.07.27 TangSZ avoid CL1 beyond the GCGM
				int index = static_cast<int>((C_CL0_CW & 0x00001fff) + curGCGMAddr);
				assert(index>=0 && index<=0x1fff);
				C_CL0_CW += curGCGMAddr;
			}
			C_CL0vec.push_back(C_CL0_CW);
		}
	}

	Vector<Vector<reg32> > C_CL0;
	C_CL0.push_back(C_CL0vec);
	C_CL0Context.insert(C_CL0Context.end(),C_CL0.begin(),C_CL0.end());
	lib_C_CL0File.close();

	////////////////////////////////////////////////////////////
	// Read CL0 (RPU0_CWI.h) file
	std::ifstream libCL0File(libCL0Name.c_str());
	if(!libCL0File)
	{
		cerr<<"Can't open file GROUP0_CWI.h'\""<<libCL0Name<<"\""<<endl;
		return -1;
	}


	// Store C_CL0 to C_CL0ContextTemp
	Vector<reg32> CL0vec;

	unsigned int CL0LineCnt = 0;
	while(getline(libCL0File, CLline))
	{
		std::string::size_type reg32Pos;
		reg32Pos = CLline.find("0x");
		if (reg32Pos!=std::string::npos)
		{
			std::istringstream CL0_CWstr(CLline.substr(reg32Pos+2, 8));
			reg32 CL0_CW;
			CL0_CWstr >> std::hex >> CL0_CW;
			if (CL0LineCnt == 1)
			{
				CL0_CW += curGCGMAddr;
			}
			CL0LineCnt++;
			/*if (CLline.find("ConfigWord[1]")!=std::string::npos)
			{
				CL0_CW += curGCGMAddr;
			}*/
			CL0vec.push_back(CL0_CW);
		}
	}

	Vector<Vector<reg32> > CL0;
	CL0.push_back(CL0vec);
	CL0Context.insert(CL0Context.end(),CL0.begin(),CL0.end());
	////////////////////////////////////////////////////////////
	// CL0 Context Generator
	//for(Vector<Vector<reg32> >::iterator CL0GrpIter = CL0.begin(); CL0GrpIter != CL0.end(); ++CL0GrpIter)
	//{
	//	Vector<reg32> curCtx = * CL0GrpIter;
	//	CL0file<<"#define GROUP"<<config.RPUGroupNumber()<<"_"<<config.onRCANumber()<<"_"<<(CL0GrpIter-CL0.begin())<<"_CWI\\\n";
	//	char buf[30];
	//	sprintf(buf,"GROUP%d_%d_%d_CWI",config.RPUGroupNumber(),config.onRCANumber(),CL0GrpIter-CL0.begin());
	//	GRPLink<<"\t\t\t"<<buf<<"\\"<<std::endl;
	//	GRPLink<<"\t\t\twhile(!RPU0_done){}\\"<<std::endl;
	//	GRPLink<<"\t\t\tRPU0_done = 0;\\"<<std::endl;
	//	CL0file.fill('0');
	//	for(Vector<reg32>::iterator CtxIter = curCtx.begin(); CtxIter != curCtx.end(); ++CtxIter)
	//	{
	//		CL0file<<"\t\t\twrite_reg(AHB1_S1_RPU0,0x"<<std::setw(8)<<std::hex<<(*CtxIter)<<");\\\n";
	//	}
	//				
	//	CL0file<<"\t\t\twrite_reg(AHB1_S1_RPU0,0x0000000"<<std::setw(1)<<std::hex<<config.onRCANumber()<<");\\\n";
	//	CL0file<<"\t\t\twrite_reg(AHB1_S1_RPU0,0x00008000);\n\n\n\n\n";
	//}

	setcurGCGMBaseAddr(curGCGMAddr);

	////////////////////////////////////////////////////////////
	// Read CL1 (GCGM.bin) file
	std::ifstream libCL1File(libCL1Name.c_str());
	if(!libCL1File)
	{
		cerr<<"Can't open file RPU0_GCGM.bin'\""<<libCL1Name<<"\""<<endl;
		return -1;
	}

	// Store CL1 to CL1ContextTemp
	Vector<reg32> CL1vec;
	int CL1IndexCnt = -42;

	while(getline(libCL1File, CLline))
	{
		int CL0Index;
		std::string::size_type reg32Pos;
		std::string::size_type CL0IndexPos;
		reg32Pos = CLline.find("=0x");
		CL0IndexPos = CLline.find("Group Cnt=");
		if (CL0IndexPos!=std::string::npos)
		{
			std::istringstream CL0IndexStr(CLline.substr(CL0IndexPos+10));
			CL0IndexStr >> std::dec >> CL0Index;
			curGCGMAddr += CL0Index;
		}
		if (reg32Pos!=std::string::npos)
		{	
			///////////////////////////////////////////////////////////
			//
			// 2012.07.27 TangSZ 
			//  >Avoid error if RIDL enable
			//  >Increace the RC Core ConfigWord from 127 to 255
			// by adding the bit[13] as the MSB
			//  >Adding limitation of RC Core ConfigWord
			//
			////////////////////////////////////////////////////////////
			std::istringstream CL1_CWstr(CLline.substr(reg32Pos+3, 8));
			reg32 CL1_CW;
			CL1_CWstr >> std::hex >> CL1_CW;
			if (CLline.find("GroupWord[0]")!=std::string::npos)
			{
				if (CL1_CW & 0x00000008)
					CL1IndexCnt = 0 - (CL1_CW & 0x00000007);
				else 
					CL1IndexCnt = -1;
			}
			else if ( CL1IndexCnt==0 )
			{
				int index = static_cast<int>((CL1_CW & 0x00002000)>>6)|(CL1_CW & 0x0000007f);
				index += RCAformerNumTemp;
				assert(0 <= index && index <= 0xff);
				CL1_CW = (CL1_CW & 0xffffdf80) | static_cast<reg32>( ((index>>7)<<13) | (index & 0x0000007f) );
				CL1IndexCnt = 0;
			}
			else if ( (CL1IndexCnt>0) && (CL1IndexCnt%4)==0 )
			{
				int index = static_cast<int>((CL1_CW & 0x00002000)>>6)|(CL1_CW & 0x0000007f);
				index += RCAformerNumTemp;
				assert(0 <= index && index <= 0xff);
				CL1_CW = (CL1_CW & 0xffffdf80) | static_cast<reg32>( ((index>>7)<<13) | (index & 0x0000007f) );
			}
			CL1IndexCnt++;
			CL1vec.push_back(CL1_CW);



			//std::istringstream CL1_CWstr(CLline.substr(reg32Pos+3, 8));
			//reg32 CL1_CW;
			//CL1_CWstr >> std::hex >> CL1_CW;
			//if (CLline.find("GroupWord[1]")!=std::string::npos)
			//{
			//	CL1_CW += RCAformerNumTemp;
			//	CL1IndexCnt = 0;
			//}
			//else if ( (CL1IndexCnt>0) && (CL1IndexCnt%4)==0 )
			//{
			//	CL1_CW += RCAformerNumTemp;
			//}
			//CL1IndexCnt++;
			//CL1vec.push_back(CL1_CW);
		}
	}

	Vector<Vector<reg32> > CL1;
	CL1.push_back(CL1vec);
	CL1Context.insert(CL1Context.end(),CL1.begin(),CL1.end());
	libCL1File.close();

	////////////////////////////////////////////////////////////
	// Read CL2 (GCCM.bin) file
	std::ifstream libCL2File(libCL2Name.c_str());
	if(!libCL2File)
	{
		cerr<<"Can't open file RPU0_GCGM.bin'\""<<libCL2Name<<"\""<<endl;
		return -1;
	}

	// Store CL2 to CL2ContextTemp
	Vector<reg32> CL2vec;
	Vector<Vector<reg32> > CL2;
	//std::string CLlineTemp;
	unsigned int CL1Index;
	unsigned int CL2Index = 0;

	while(getline(libCL2File, CLline))
	{
		//int CL1Index;
		//unsigned int CL2Index = 0;
		std::string::size_type reg32Pos;
		std::string::size_type CL1IndexPos;
		reg32Pos = CLline.find("=0x");
		CL1IndexPos = CLline.find("Context Cnt=");
		if (CL1IndexPos!=std::string::npos)
		{
			CL2Index = static_cast<int> (CL2Context.size());
			CL2vec.push_back(CL2Index);
			CL2Index++;
			std::istringstream CL1IndexStr(CLline.substr(CL1IndexPos+12));
			CL1IndexStr >> std::dec >> CL1Index;
			RCAformerNumTemp += CL1Index;
		}
		else if (reg32Pos!=std::string::npos)
		{
			std::istringstream CL2_CWstr(CLline.substr(reg32Pos+3, 8));
			reg32 CL2_CW;
			CL2_CWstr >> std::hex >> CL2_CW;
			CL2vec.push_back(CL2_CW);
			if (CLline.find("106]=0x")!=std::string::npos)
			{
				for (std::size_t index=1; index<=64; index++)
				{
					if ( ((CL2vec.at(index)>>16) & 0x3) == 2 )
					{
						unsigned short libCMIndex = (CL2vec.at(index)>>9)&0x1f;
						assert(libCMIndex<16);
						unsigned short value = libConstMem[libCMIndex];
						int CMIndex=0;
						for (;CMIndex<16;++CMIndex)
						{
							if( (ConstMem[CMIndex]&0xffff)==0xffff ) 
							{
								ConstMem[CMIndex] = value; 
								break;
							}
							else if(ConstMem[CMIndex]==value)break;
						}
						assert(CMIndex!=16);
						CL2vec[index] = ((CL2vec.at(index)&0xffffc1ff)+(CMIndex<<9));
					}
				
				}

				CL2.push_back(CL2vec);
				CL2vec.clear();	
				CL2vec.push_back(CL2Index);
				CL2Index++;
				//CL2ContextTemp.insert(CL2ContextTemp.end(),C_CL2.begin(),C_CL2.end());
			}
		}
		//CLlineTemp = CLline;
	}	
	CL2Context.insert(CL2Context.end(),CL2.begin(),CL2.end());
	libCL2File.close();
	setRCANumbefore(RCAformerNumTemp);
	totalRCA += CL1Index;


	return curGCGMAddr;
}