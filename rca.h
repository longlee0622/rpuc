
#ifndef RCA_H
#define RCA_H

#include "platf.h"
#include "rcavex.h"

#include <iostream>
#include <cassert>

class RCARegs;

class RCA {
	
public:

	RCA(int seqNo = -1);

	RCA(const RCA & rca);

	int seqNo() const { return seqNumber; }

	void setSeqNo(int num) { seqNumber = num; }

	//------------------------------------------------------

	const RCANode & node(int row, int col) const;

	RCANode & node(int row, int col);

	const RCANode & node(int index) const ;

	RCANode & node(int index);

	const RCANode & temp(int row, int col) const;

	RCANode & temp(int row, int col);

	const RCANode & temp(int index) const ;

	RCANode & temp(int index);

	//----------------------------------------

	RCA * sources(int seqNo) 
	
	{ return rcaSources.at(seqNo); }

	const RCA * sources(int seqNo) const 
	
	{ return rcaSources.at(seqNo); }

	Vector<RCA*> & sources() 
	
	{ return rcaSources; }

	const Vector<RCA*> & sources() const 
	
	{ return rcaSources; }

	void removeSource(RCA * rca);

	//-----------------------------------------

	RCA * targets(int seqNo) 
	
	{ return rcaTargets.at(seqNo); }

	const RCA * targets(int seqNo) const 
	
	{ return rcaTargets.at(seqNo); }

	Vector<RCA*> & targets() 
	
	{ return rcaTargets; } 

	const Vector<RCA*> & targets() const 
	
	{ return rcaTargets; }

	void removeTarget(RCA * rca);

	//-----------------------------------------

	RCAPort & inports(int seqNo) 
	
	{ return rcaInports.at(seqNo); }

	const RCAPort & inports(int seqNo) const 
	
	{ return rcaInports.at(seqNo); }

	Vector<RCAPort> & inports() 
	
	{ return rcaInports; }

	const Vector<RCAPort> & inports() const 
	
	{ return rcaInports; }

	//-----------------------------------------

	RCAPort & outports(int seqNo) 
	
	{ return rcaOutports.at(seqNo); }

	const RCAPort & outports(int seqNo) const 
	
	{ return rcaOutports.at(seqNo); }

	Vector<RCAPort> & outports() 
	
	{ return rcaOutports; }

	const Vector<RCAPort> & outports() const 
	
	{ return rcaOutports; }

	//-------------------------------------------

	// Schedule state
	int state() const { return rcaState; }

	void setState(int state) { rcaState = state; }

	//-------------------------------------------

	int loadSize() const { return rcaLoadSize; }

	void setLoadSize(int size) { rcaLoadSize = size; }

	int storeSize() const { return rcaStoreSize; }

	void setStoreSize(int size) { rcaStoreSize = size; }

	int loopNum () const { return rcaLoopNum; }

	void setLoopNum(int loop) { rcaLoopNum = loop; }

	//-------------------------------------------------------

	RCA * copy() { return new RCA(*this); }

    int getPseudoRCAFlag() const  { return PseudoRCAFlag; }

    void setPseudoRCAFlag(int flag) { PseudoRCAFlag = flag ; }

    int getPseudoRCAMode() const  { return PseudoRCAMode; }

    void setPseudoRCAMode(int mode)  { PseudoRCAMode = mode;  }

    bool getMappedFlag() const  { return MappedFlag; }

	bool getRemapFlag() const {return RemapFlag; }

	bool getTooManyFlag() const {return TooManyFlag;}
	
	void setMappedFlag(bool mode)  { MappedFlag = mode;  }
	
	void setRemapFlag(bool mode) {RemapFlag = mode; }

	void setTooManyFlag(bool mode) {TooManyFlag = mode; }

	//yin0912 begin
	List<List<int> > gettempDataBlockList()const {return tempDataBlockList;}
	void setTempDataBlockList(List<List<int> > listList) {tempDataBlockList = listList;}
	//yin0912end

	//yin0914 begin
	int rcaSSRAMInBaseAddr() const { return ssramInBaseAddr; }
	void setRCASSRAMInBaseAddr(int addr) { ssramInBaseAddr = addr; }

	int rcaSSRAMInTopAddr() const { return ssramInTopAddr; }
	void setRCASSRAMInTopAddr(int addr) { ssramInTopAddr = addr; }

	int rcaSSRAMOutBaseAddr() const { return ssramOutBaseAddr; }
	void setRCASSRAMOutBaseAddr(int addr) { ssramOutBaseAddr = addr; }

	int rcaSSRAMOutTopAddr() const { return ssramOutTopAddr; }
	void setRCASSRAMOutTopAddr(int addr) { ssramOutTopAddr = addr; }

	int rcaSSRAMTempOutBaseAddr() const { return ssramTempOutBaseAddr; }
	void setRCASSRAMTempOutBaseAddr(int addr) { ssramTempOutBaseAddr = addr; }

	int rcaSSRAMTempOutTopAddr() const { return ssramTempOutTopAddr; }
	void setRCASSRAMTempOutTopAddr(int addr) { ssramTempOutTopAddr = addr; }

	int getSSRAMTempInBaseAddr() const {return ssramTempInBaseAddr;}
	void setRCASSRAMTempInBaseAddr(int addr) { ssramTempInBaseAddr = addr; }

	int getSSRAMTempInTopAddr() const {return ssramTempInTopAddr;}
	void setRCASSRAMTempInTopAddr(int addr) { ssramTempInTopAddr = addr; } 

	//2011.5.18 liuxie
	int CL0GroupNumber() const {return Cl0GroupNum;}
	void setCL0GroupNumber(int CL0GroupNo) {Cl0GroupNum = CL0GroupNo;}

private:

	int seqNumber;

	Vector<RCAPort> rcaInports;

	Vector<RCAPort> rcaOutports;

	Vector<RCA*> rcaSources;

	Vector<RCA*> rcaTargets;

	RCANode nodes[RC_REG_NUM];

	RCANode tempNodes[RC_TEMP_NUM];

	// Schedule state

	int rcaState;

	// Execute parameters

	int rcaLoadSize;

	int rcaAHBLoad;

	int rcaStoreSize;

	int rcaAHBStore;

	int rcaExecuteNum;

	int rcaReloadNum;

	int rcaLoopNum;

    int PseudoRCAFlag;

    int PseudoRCAMode;

    bool MappedFlag;

	bool RemapFlag;
	
	bool TooManyFlag; 
	
	List<List<int> > tempDataBlockList;//yin0912定义该变量来存放该RCA输入tempPort的区块

	int ssramInBaseAddr;//0914存放RCA的输入端口，直接从外部输入，因为这是一块连续的区域，
	int ssramInTopAddr;//因此确定首地址和末地址即可，

	int ssramOutBaseAddr;//0914存放RCA的输出端口，直接输出到外部ssram，因为这是一块连续的区域，
	int ssramOutTopAddr;//因此确定首地址和末地址即可，

	int ssramTempOutBaseAddr;//0914存放RCA的输入端口，直接从temp out输入，因为这是一块连续的区域，
	int ssramTempOutTopAddr;//因此确定首地址和末地址即可，


	int ssramTempInBaseAddr;   //20110529 liuxie for temp data inport
	int ssramTempInTopAddr;

    //2011.5.18 liuxie
	int Cl0GroupNum;    //所在的CL0Group编号

};

//////////////////////////////////////////////////////////////

inline const RCANode & RCA::node(int row, int col) const{ 

	assert(row >=0 && row < RCA_ROW);
	assert(col >=0 && col < RCA_COL); 
	return nodes[row * RCA_COL + col]; 
}

inline RCANode & RCA::node(int row, int col){ 

	//change by liuxie 2010.11.22

	/*if( row < 0 || row >= RCA_ROW || col < 0 || col >= RCA_COL)
	{
		printf("Error: the wrong node is at Row %d and Col %d \n", row, col);
	}*/
	assert(row >=0 && row < RCA_ROW);
	assert(col >=0 && col < RCA_COL); 
	return nodes[row * RCA_COL + col]; 
}

inline const RCANode & RCA::node(int index) const {

	assert(index >=0 && index < RC_REG_NUM); 
	return nodes[index]; 
}

inline RCANode & RCA::node(int index) { 

	assert(index >=0 && index < RC_REG_NUM);
	return nodes[index]; 
}

inline const RCANode & RCA::temp(int row, int col) const{ 

	assert(row >=0 && row < RCA_ROW);
	assert(col >=0 && col < RCA_COL); 
	return tempNodes[row * RCA_COL + col]; 
}

inline RCANode & RCA::temp(int row, int col){ 

	assert(row >=0 && row < RCA_ROW);
	assert(col >=0 && col < RCA_COL); 
	return tempNodes[row * RCA_COL + col]; 
}

inline const RCANode & RCA::temp(int index) const {

	assert(index >=0 && index < RC_REG_NUM); 
	return tempNodes[index]; 
}

inline RCANode & RCA::temp(int index) { 

	assert(index >=0 && index < RC_REG_NUM); 
	return tempNodes[index]; 
}

inline RCA::RCA(int seqNo)
	: seqNumber(seqNo),
	rcaInports(),
	rcaOutports(),
	rcaSources(),
	rcaTargets(),
	rcaState(0),
	rcaLoadSize(1), 
	rcaAHBLoad(0), 
	rcaStoreSize(1), 
	rcaAHBStore(0), 
	rcaExecuteNum(0),
	rcaReloadNum(0),
	rcaLoopNum(1), 
	PseudoRCAFlag(0),
	PseudoRCAMode(0),
	RemapFlag(false),
	MappedFlag(false),
	TooManyFlag(false){

	for(int r =0; r <RCA_ROW; ++r)
		for(int c =0; c <RCA_COL; ++c){
			int curIndex = r * RCA_COL + c;

			nodes[curIndex].setRow(r);
			nodes[curIndex].setCol(c);

			nodes[curIndex].setRCA(this);

			tempNodes[curIndex].setRow(r);
			tempNodes[curIndex].setCol(RCA_COL + c);

			tempNodes[curIndex].setRCA(this);
		}
}

inline RCA::RCA(const RCA & rca)
	: seqNumber(rca.seqNumber),
	rcaInports(rca.rcaInports),
	rcaOutports(rca.rcaOutports),
	rcaSources(rca.rcaSources),
	rcaTargets(rca.rcaTargets),
	rcaState(rca.rcaState),
	rcaLoadSize(rca.rcaLoadSize), 
	rcaAHBLoad(rca.rcaAHBLoad), 
	rcaStoreSize(rca.rcaStoreSize), 
	rcaAHBStore(rca.rcaAHBStore), 
	rcaExecuteNum(rca.rcaExecuteNum),
	rcaReloadNum(rca.rcaReloadNum),
	rcaLoopNum(rca.rcaLoopNum),
	PseudoRCAFlag(rca.PseudoRCAFlag),
	PseudoRCAMode(rca.PseudoRCAMode),
	tempDataBlockList(rca.tempDataBlockList),
	ssramInBaseAddr(rca.ssramInBaseAddr),
	ssramInTopAddr(rca.ssramInTopAddr),
	ssramOutBaseAddr(rca.ssramOutBaseAddr),
	ssramOutTopAddr(rca.ssramOutTopAddr),
	ssramTempOutBaseAddr(rca.ssramTempOutBaseAddr),
	ssramTempOutTopAddr(rca.ssramTempOutTopAddr),
	RemapFlag(rca.RemapFlag),
	MappedFlag(rca.MappedFlag),
	TooManyFlag(rca.TooManyFlag){

	for(int i =0; i <RC_REG_NUM; ++i){
		nodes[i] = rca.nodes[i];
		nodes[i].setRCA(this);

		tempNodes[i] = rca.tempNodes[i];
		tempNodes[i].setRCA(this);
	}

}

inline void RCA::removeSource(RCA * rca){
	Vector<RCA*>::iterator iter, end;

	end = rcaSources.end();
	for(iter = rcaSources.begin(); iter != end; ++ iter)
		if(*iter == rca)break;

	rcaSources.erase(iter);
}

inline void RCA::removeTarget(RCA * rca){
	Vector<RCA*>::iterator iter, end;

	end = rcaTargets.end();
	for(iter = rcaTargets.begin(); iter != end; ++ iter)
		if(*iter == rca)break;

	rcaTargets.erase(iter);
}

///---------------------------------------------------

inline std::ostream & 
operator <<(std::ostream & out, const RCA & rca){
	//Print RCA inport
	Vector<RCAPort>::const_iterator portIter;

	out<<"In("<<rca.inports().size()<<"):\n";
	for(portIter = rca.inports().begin(); 
		portIter != rca.inports().end(); ++ portIter)
		out<<'\t'<<*portIter->dfgPort()<<"\n";


	for(int r =0; r <RCA_ROW; ++ r){
		for(int c =0; c <RCA_COL; ++ c){

			//chage by liuxie 2010.11.22 for debug
			if( r==3 && c==8 )
			{
				printf("The bad place at rca.h line 403!\n");
			}
			const RCANode & curNode = rca.node(r, c);
			DFGNode * thisNode = curNode.dfgNode();

			thisNode ? out<<*thisNode<<"("<<thisNode->weight()<<") " : out<<"X ";
		}

		out<<"\n";
	}

	out<<"Out("<<rca.outports().size()<<"):\n";
	for(portIter = rca.outports().begin(); 
		portIter != rca.outports().end(); ++ portIter)
		out<<'\t'<<*portIter->dfgPort()<<"\n";

	return out;
}

#endif
