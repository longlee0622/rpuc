/************************************************
 *			File name: rpucfg.h
 *-----------------------------------------------
 * Description:
 *
 * Author: L. Xie @ MicroE. SJTU
 * Date : 2009/11/13
 *************************************************
 */

#ifndef REMUSCFG_H
#define REMUSCFG_H


#include "rca.h"
#include "cl1cfg.h"
#include "dfgraph.h"

#include <iostream>
#include <map>

class OpCL2timizeMethod;
class SplitMethod;
class CL1Block;

//2011.5.11 liuxie
/*#define MAX_CL1_RCA_NUM		8
#define MAX_CL1BLOCK_NUM		16 // 
#define MAX_CL0_OUTREGION_SIZE  (24*32)  //upper limit of momery in RIM OutRegion
*/
#define MAX_CL1_RCA_NUM		12     
#define MAX_CL1BLOCK_NUM	16       //由于out region只有512Bytes，所以一个Group最多也就只有四个伪RCA，即四个REDS
#define MAX_CL0_OUTREGION_SIZE  (16*32)  //upper limit of momery in RIM OutRegion

//////////////////////////////////////////////////////////

class RPUConfig{

public:

	RPUConfig( void );

	RPUConfig(const DFGraph & graph): dfgGraph(graph) {}

	DFGraph & graph() { return dfgGraph; }

	List<Ptr<RCA> > & rcas() { return rcaList; }

	Vector<RCA*> & allRCAs() { return allRCAVec; }//yin0901

	void addRCAToAllRCAVec(RCA* rca){allRCAVec.push_back(rca);}//yin0901

	RCA * newRCA();       //insert a new RCA in the whole list

   // RCA * InsertRCAInGroup();  //insert a new RCA in one CL0RCA list
	//----------------------------------------------------

	// The functions below used by the spliting algorithm 
	DFGNode * newBypsNode();

	DFGNode * newFakeNode();

	DFGPort * getNodePort (DFGNode * node);

	DFGNode * getNodeByps (DFGNode * node);

	DFGPort * cutSource(DFGNode * node, int srcIndex);

	DFGPort * insertPortOf (DFGNode * node);

	DFGNode * insertBypassBetween(
		DFGNode * srcNode, DFGNode * tgtNode
	);

	void display(std::ostream & out = std::cout) const;

	//-------------------------------------------------

	// This funnction use to optimizing and splitiing a DFG, 
	// and map the subgraph into a series of RCA.
	int mapDFGraph(
		const OptimizeMethod & optmize, const SplitMethod & split
	);
	
	// This function used to generate the CL0, CL1, CL2 context
	int genContext();

	// -------------------------------------------------

	// The functions below create header file including context
	// and macro definition of function interface.

	//2011.6.19 liuxie for PortInterface file
	//int createHeadFile(const String & fileName);

	int createInterface(const String & fileName);

	int createPatchFile(const String & fileName);

	int createCL0File(const String & fileName);

	int createCL1File(const String & fileName);

	int createCL2File(const String & fileName);

	// Create file for debug
	int printNodeInfo(const String & fileName);

	
//20110716 liuxie	
	Vector<reg32> CL0ContextCopy() {return CL0Context;}

	void pasteCL0Context(Vector<reg32> CL0ContextTemp) { CL0Context = CL0ContextTemp;}

	Vector<Vector<reg32> > CL1ContextCopy() {return CL1Context;}

	void pasteCL1Context(Vector<Vector<reg32> > CL1ContextTemp) {CL1Context = CL1ContextTemp;}

	Vector<Vector<reg32> > CL2ContextCopy() {return CL2Context;}

	void pasteCL2Context(Vector<Vector<reg32> > CL2ContextTemp) {CL2Context = CL2ContextTemp;} 

	int onRPUNumber(){return onRPUNum;}

	void setRPUNum(int RPUnumber){onRPUNum = RPUnumber;}

	int onRCANumber(){return onRCANum;}

	void setRCANum(int RCAnumber){onRCANum = RCAnumber;}

	const char * locateDFGname(int q) {return locateGroupDFGList[q];} 
	
	void setlocateDFGroupList(const char * curDFGList, int q) {locateGroupDFGList[q] = curDFGList;}

	int RCANumBefore() { return RCANumberBefore;}

	void setRCANumbefore(int num) { RCANumberBefore = num;}

	int curGCGMBaseAddr() {return GCGMAddrRecord;}

	void setcurGCGMBaseAddr(int num) {GCGMAddrRecord = num;} 

	int DFGInBaseAddress() {return DFGInBaseAddr;}

	void setDFGInBaseAddress(int num) {DFGInBaseAddr = num;}

	int GroupRCANumber(){return GroupRCANum;}

	void setGroupRCANumber(int num){GroupRCANum = num;}

	int RPUGroupNumber(){return RPUGroupNum;}

	void setRPUGroupNum(int num) {RPUGroupNum = num;}

	List<Ptr<DFGPort> > & getInnerPortList() {return innerDFGPorts;}


	// ----------------------------------------------------

private:

	// This function used to get the source 
	// and target information of RCA after splitting
	int connectRCA();

	int genCL0Context( CL1Config & cl1config,Vector<RCA *> &CL1RCATemp,Vector<RCA *> &recordPseudoRCA); // GenerateCL0 context

	int genGroupContext( ); // Generate Groups context

	int genCL2Context( Vector<RCA *> CL1RCA ); // GenerateCL2 context

private:

	DFGraph dfgGraph; // DFG structrue of this configuration

	Vector<RCA* > allRCAVec;//splitted RCA + pseudo RCA vector 0901

	List<Ptr<RCA> > rcaList; // RCA list after splitting.		

	List<Ptr<CL1Block> > blockList; // CL1Block list after CL1 mapping

	List<Ptr<DFGPort> > innerDFGPorts; // Internal port after splitting

	List<Ptr<DFGNode> > bypassNodes; // Bypass node in DFG

	// Record the RCA port attach to a DFG node
	std::map<DFGNode*, DFGPort*> nodePortTable;

	// Record the last bypass node of a DFG node
	std::map<DFGNode*, DFGNode*> nodeBypsTable;

	//CL1Config cl1config;
    Vector<int>DFGSSRAMInBase;

	Vector<int>DFGSSRAMOutBase;
	
	Vector<int> GroupSSRAMExternInBase;

    Vector<int> GroupSSRAMExternInTop;

    Vector<int> GroupSSRAMExternOutBase;

    Vector<int> GroupSSRAMExternOutTop;

    Vector<int> GroupSSRAMTempInBase;

    Vector<int> GroupSSRAMTempInTop;

    Vector<int> GroupSSRAMTempOutBase;

    Vector<int> GroupSSRAMTempOutTop;

    Vector<int> GroupGCGMAddress;

    Vector<int> GroupREDLExternNum;

    Vector<int> GroupREDLTempNum;
    
	// Store context
    Vector<Vector<reg32> > CLOGroup;
    
	Vector<reg32> CL0Context;

	Vector<Vector<reg32> > CL1Context;

	Vector<Vector<reg32> > CL2Context;

	int onRPUNum; //当前config所在的RPU编号

	int onRCANum; //当前config所在的RCA编号

	int RCANumberBefore;  //在改组RPU下已经映射了多少个RCA

	const char *locateGroupDFGList[4];

	int GCGMAddrRecord;

	int DFGInBaseAddr;  //当前DFG图所在RCA从外部输入的最低数据地址

	int GroupRCANum;    //当前Group中的RCA编号

	int RPUGroupNum;    //当前组的组号
};


/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
#define MAX_GROUP_NUM 20


inline RPUConfig::RPUConfig( void ) :
      GroupRCANum(0),
	  RCANumberBefore(0),
	  onRPUNum(0),
	  onRCANum(0),
	  GroupSSRAMExternInBase(MAX_GROUP_NUM,0),
      GroupSSRAMExternInTop(MAX_GROUP_NUM,0),
      GroupSSRAMExternOutBase(MAX_GROUP_NUM,0),
      GroupSSRAMExternOutTop(MAX_GROUP_NUM,0),
      GroupSSRAMTempInBase(MAX_GROUP_NUM,0),
      GroupSSRAMTempInTop(MAX_GROUP_NUM,0),
      GroupSSRAMTempOutBase(MAX_GROUP_NUM,0),
      GroupSSRAMTempOutTop(MAX_GROUP_NUM,0),
      GroupGCGMAddress(MAX_GROUP_NUM,0)
        {
        }
    

#endif
