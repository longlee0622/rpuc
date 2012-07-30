// File name: cl1cfg.h
//
// Author: Xie Li@ SJTU.MicroE, Time: 2010/07/31
// ==============================================
// This file defines class generating the CL1 context.
//

#ifndef CL1CFG_H
#define CL1CFG_H

#include "cl1.h"

class RCA;
class RCAPort;

class RPUConfig;

class CL1Config{

public:

	CL1Config();

    Vector<CL1Block> genCL1Block(RPUConfig & config, Vector<RCA*> & rcas);

	int PreGenCL1(Vector<RCA*> & rcas,int DFGInBase);
	
	const Vector<reg32> & genRegs(const Vector<CL1Block> & blocks);

	int RIDLLength() const { return CL1RIDLLength; }

	int CoreLength() const { return CL1CoreLength; }

	int totalLength() const { return CL1TotalLength; }

	//////////////////////////////////////////////////////////////




	int getSSRAMBaseAddrIn() const { return SSRAMInBaseAddr; }
	void setSSRAMBaseAddrIn(int addr){SSRAMInBaseAddr =addr;}

	int getSSRAMTopAddrIn() const { return SSRAMInTopAddr; }
	void setSSRAMTopAddrIn(int addr){SSRAMInTopAddr =addr;}

	int getSSRAMBaseAddrOut() const { return SSRAMOutBaseAddr; }
	void setSSRAMBaseAddrOut(int addr){SSRAMOutBaseAddr =addr;}

	int getSSRAMTopAddrOut() const { return SSRAMOutTopAddr; }
	void setSSRAMTopAddrOut(int addr){SSRAMOutTopAddr =addr;}

	int getSSRAMTempBaseAddrOut() const { return SSRAMTempOutBaseAddr; }
	void setSSRAMTempBaseAddrOut(int addr){SSRAMTempOutBaseAddr =addr;}

	int getSSRAMTempTopAddrOut() const { return SSRAMTempOutTopAddr; }
	void setSSRAMTempTopAddrOut(int addr){SSRAMTempOutTopAddr =addr;}

	int getSSRAMTempInBaseAddr() const { return SSRAMTempInBaseAddr; }
	void setSSRAMTempInBaseAddr(int addr){SSRAMTempInBaseAddr =addr;}

	int getSSRAMTempTopAddrIn() const { return SSRAMTempInTopAddr; }
	void setSSRAMTempTopAddrIn(int addr){SSRAMTempInTopAddr =addr;}

    Vector<RCAPort *> getTempPortInRIM() const { return tempPortInRIM ; }

	int GCGMAddress() const { return GCGMAddr; }

	void setGCGMAddress(int addr) { GCGMAddr = addr; }
    
    const Vector<CL1Block> insertPseudoRCA(RPUConfig & config, Vector<RCA *> & CL1RCA, Vector<RCA *> & recordTempPseduRCA);
	//伪RCA(PseudoRCA)的输入和输出一样，节点既是输入节点又是输出接点，这样做的好处是：保证输入顺序和输出顺序的一致性

	//2011.5.18 liuxie
	/////////////////////////////////start////////////////////////////
	void setAreaTempCounter(int num, int i) {tempAreaCounter[i] += num;}

	void setAreaExternCounter(int num, int i) {outAreaCounter[i] += num;}

	void insertTempPort(RCAPort * port) { tempPortInRIM.push_back(port);}

	void insertOutPort(RCAPort * port) {outPortInRIM.push_back(port);}

	int RPUBlockBefore() {return RPUBlockMappedBefore;}

	void setRPUBlockBefoe(int num) {RPUBlockMappedBefore = num;}

	
	//Vector<int> outAreaCounter;

	//Vector<int> tempAreaCounter;
	
	/////////////////////////////////end//////////////////////////////
   
private:

	int remainRCA(const Vector<RCA*> & rcas);

	Vector<RCA*> readyRCA(const Vector<RCA*> & rcas);

	void updateRCAState(const Vector<RCA*> & rcas, int & ScheduleFalseFlag,int & remainRCANumVar);

	Vector<CL1Block> mapRCA(Vector<RCA*> rcas,Vector<RCA*> &tmpGrpRCA,Vector<RCA*> &RCAS,RPUConfig &config);

	const Vector<CL1Block> PreMapRCA(Vector<RCA*> rcas,Vector<RCA*> &tmpGrpRCA,Vector<RCA*> &RCAS,RPUConfig &config,int &REDLCnt);
	
	void freeRIMSpace(const Vector<RCA*> rcas);

	int sumPseudoRCA();


private:

	int CL1RIDLLength;

	int CL1CoreLength;

	int CL1TotalLength;

	Vector<reg32> Regs;

	CL1RIM RIM;

	Vector<RCAPort*> outPortInRIM;   //outPortInRIM中的out port节点是按照RCA执行过程的中的先后顺序读入；
	                                 //并且也是按照这个顺序读出生成伪RCA（PseudoRCA）
	Vector<int> outAreaCounter;

	Vector<RCAPort*> tempPortInRIM;

	Vector<int> tempAreaCounter;

	int SSRAMInBaseAddr ;      //REDL1取数据的首地址
	int SSRAMOutBaseAddr;      //REDL2取数据的首地址

	int SSRAMInTopAddr;     
	int SSRAMOutTopAddr;

	int SSRAMTempOutBaseAddr;
	int SSRAMTempOutTopAddr;

	int SSRAMTempInBaseAddr;
	int SSRAMTempInTopAddr;

	int GCGMAddr;

	int RPUBlockMappedBefore;  //当前RPU已经映射的RCA数量
};


//-----------------------------------------------
//2011.5.11 liuxie
/*
#define MAX_OUT_AREA_SUM    6
#define MAX_TEMP_AREA_SUM    2
#define MAX_AREA_SUM    8
#define MAX_OUT_PORT_IN_RIM  24*32
#define MAX_TEPM_PORT_IN_RIM  8*32
*/
#define MAX_OUT_AREA_SUM    2
#define MAX_TEMP_AREA_SUM    2
#define MAX_AREA_SUM    4
#define MAX_OUT_PORT_IN_RIM  16*32
#define MAX_TEPM_PORT_IN_RIM  16*32



//2011.5.27 liuxie
//修改了SSRAM的各个区域的首地址
//  SSRAMInBaseAddr = 0;
//  SSRAMTempOutBaseAddr = 0x300;
//  SSRAMOutBaseAddr = 0x800;
inline CL1Config::CL1Config(): 
		CL1RIDLLength(0), 
		CL1CoreLength(0), 
		CL1TotalLength(0),
		outAreaCounter(MAX_OUT_AREA_SUM, 0),
		tempAreaCounter(MAX_TEMP_AREA_SUM, 0),
		SSRAMInBaseAddr(0), 
		SSRAMOutBaseAddr(816),  //0x800 
		SSRAMInTopAddr(192),     
	    SSRAMOutTopAddr(1024),   //0x800
	    SSRAMTempOutBaseAddr(192),  //0x300
	    SSRAMTempOutTopAddr(816),   //0x300
		SSRAMTempInBaseAddr(192),
		SSRAMTempInTopAddr(816),
		GCGMAddr(0)
		{
        outPortInRIM.reserve(MAX_OUT_PORT_IN_RIM);
        tempPortInRIM.reserve(MAX_TEPM_PORT_IN_RIM);
        }

#endif

