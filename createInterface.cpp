////////////////////////////////////////////////////////////
//				       ESLCompiler
//==========================================================
//
// Author: TangSZ   Time: 2012
// Department: MicroE of Shanghai Jiao Tong University
//----------------------------------------------------------
// This file generates a data interface file for ESL project,
// which needs a formal_map.h file as input and then generate  
// an interface file used in ESL platform.
//
////////////////////////////////////////////////////////////
#include "rpucfg.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>


int createInterface(const String & fileName, RPUConfig &config){
	
	using std::cerr;
	using std::endl;
	using std::cout;

	//const char * underline = "__"; 
	//const char * functionNode = "_NODE";
	//std::string::size_type fileNameBegin = fileName.find(underline)+2;
	//std::string::size_type fileNameEnd = fileName.find(functionNode);
	//std::string::size_type fileNameSubLength = fileNameEnd - fileNameBegin;

	//String mapNameTemp = fileName.substr(fileNameBegin, fileNameSubLength) + "_formal_map.h";

	String mapNameTemp = fileName.substr(0) + "_formal_map.h";
	const char * mapFileName = mapNameTemp.c_str();
	std::ifstream mapFile(mapFileName);
	if(!mapFile)
	{
		cerr<<"Can't open file data.txt'\""<<mapFileName<<"\""<<endl;
		return -1;
	}

	//int mapLoopTime = 0;
	std::string line;
	//std::string tempLoopData;
	//getline(mapFile, line);
	//std::istringstream stream(line);
	//while (stream>>tempLoopData)
	//{
	//	mapLoopTime++;
	//}

	//mapLoopTime = mapLoopTime - 2;
	//config.setLoopTime(mapLoopTime);


	const int loopTime = config.getLoopTime();

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
				

	//do {
	//	if (line == "") continue;
	//	std::istringstream stream(line);
	//	for (int i=0; i<(loopTime+2); i++)
	//	{
	//		stream>>tempScalar[i];
	//	}

	//	if (strncmp(tempScalar[0].data(), equal,7) == 0)
	//	{
	//		for (int j=0; j<loopTime; j++)
	//		{
	//			In[inNum][j] = tempScalar[j+2];
	//		}
	//		inNum++;
	//	}
	//	else if (strncmp(tempScalar[loopTime+1].data(), equal,7) == 0)
	//	{
	//		for (int j=0; j<loopTime; j++)
	//		{
	//			Out[outNum][j] = tempScalar[j];
	//		}
	//		outNum++;
	//	}
	//	else std::cout<<"Incorrect!"<<std::endl;
	//}
	//while (getline(mapFile, line));
	while (getline(mapFile, line)) {
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
	//while (getline(mapFile, line));

	const String SSRAM_interface =config.graph().name()+"_interface.h";


	std::ofstream SSRAMinterfaceFile(SSRAM_interface.c_str());
	assert(SSRAMinterfaceFile);

	String portName;
	int  portSSRAM;
	DFGPort * currInport;
	DFGPort * currOutport;
	//DFGVarPort * varPort;
	
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
	strcpy(tempUperString, graphName.c_str());
	//int graphNamelength,i;
	int graphNamelength;
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
					SSRAMinterfaceFile<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<") = (short)"<<portName<<";\\\n";
					scalarInNum++;
				}
				else
				{
					portName = portName.substr(portName.find(".")+1);
					portSSRAM = (*currInport).SSRAMAddress() + currLoopTime * ssramInSize;
					SSRAMinterfaceFile<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<") = (short)"<<portName<<";\\\n";
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
				SSRAMinterfaceFile<<portName<<" = "<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<");"<<"\\\n";
				scalarOutNum++;
			}
			else
			{
				portName = portName.substr(portName.find(".")+1);
				portSSRAM = (*currOutport).SSRAMAddress() + currLoopTime * ssramOutSize;
				SSRAMinterfaceFile<<portName<<" = "<<"*(RP16)( AHB0_S2_EMI_SSRAM + 0x"<<std::hex<<portSSRAM<<");"<<"\\\n";
			}

		}
	}

	SSRAMinterfaceFile<<std::endl;

	SSRAMinterfaceFile.close();

	return 0;
}
