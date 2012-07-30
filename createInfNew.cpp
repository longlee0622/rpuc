#include "rpucfg.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <algorithm>

int RPUConfig::createPatchFile(const String &fileName)
{

	int zz;

	std::ofstream patchFile(fileName.c_str());

	assert(patchFile);

	const String & graphName = dfgGraph.name();

	String GRAPHNAME = graphName;
	std::transform(GRAPHNAME.begin(), GRAPHNAME.end(), 
			GRAPHNAME.begin(), ::toupper);

	//upper graphName
	char tempUperString[256];
	strcpy(tempUperString,graphName.c_str());
	int graphNamelength,i;
	graphNamelength=strlen(tempUperString);
	for(i=0;i<graphNamelength;i++)
	{
		tempUperString[i]=toupper(tempUperString[i]);
	}
	//upper end

	char templiststring[256];
	int listlength;

	//		"#include \"RPU"<<onRPUNum<<"_CWI.h\"\n\n\n";

	for(zz = 0; zz < GroupRCANum; zz++)
	{
		patchFile<<
			"#include \""<<locateGroupDFGList[zz]<<"_interface.h\"\n";
	}

	patchFile<<
		"\n\n\n"
//		"#ifndef RPU_"<<onRPUNum<<"_GROUP_"<<RPUGroupNum<<"_FLAG\n\n\n"
//		"#define RPU_"<<onRPUNum<<"_GROUP_"<<RPUGroupNum<<"_FLAG\n\n\n"
		"#define "<<tempUperString<<"_FUNC \\\n";


	for(zz = 0; zz < GroupRCANum; zz++)
	{
		//upper locateGroupDFGList
		strcpy(templiststring,locateGroupDFGList[zz]);
		listlength = strlen(templiststring);
		for(i=0;i<listlength;i++)
		{
			templiststring[i]=toupper(templiststring[i]);
		}
		//upper end

		patchFile<<
			"\t\t\t"<<templiststring<<"_DIN\\\n";
	}
	
	patchFile<<"\t\t\t"<<"GROUP"<<RPUGroupNum<<"_CWI\\\n";
	//patchFile<<"\t\t\t"<<"while(!RPU0_done){ }\\\n";

	for(zz = 0; zz < GroupRCANum; zz++)
	{
		//upper locateGroupDFGList
		strcpy(templiststring,locateGroupDFGList[zz]);
		listlength = strlen(templiststring);
		for(i=0;i<listlength;i++)
		{
			templiststring[i]=toupper(templiststring[i]);
		}
		//upper end

		if(zz!=GroupRCANum-1){
			patchFile<<
				"\t\t\t"<<templiststring<<"_DOUT\\\n";
		}
		else{
			patchFile<<"\t\t\t"<<templiststring<<"_DOUT\n";
		}
	}

	patchFile<<std::endl;
	patchFile.close();

	/*
	patchFile<<
		"#else\n\n"
		"#define "<<tempUperString<<"_FUNC \\\n\n"
		"#endif\n\n";

	patchFile<<std::endl;
	*/
	
	return 0;
}
















