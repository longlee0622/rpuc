

#ifndef DFGNODE_H
#define DFGNODE_H

#include "dfgvex.h"
#include "dfgport.h"

#include <iosfwd>
#include <vector>

class RCANode;

class DFGNode: public DFGVex{

public:

	DFGNode()
		: nodeWidth(-1), 
		nodeStep(-1),
		nodeWeight(1),
		isPlaced(false), 
		nodeRC(0){}

	DFGNode(int opecode)
		: nodeWidth(-1), 
		nodeStep(-1),
		nodeOpecode(opecode),
		nodeWeight(1),
		isPlaced(false), 
		nodeRC(0){}

	//-------------------------------------

	int seqNo () const { return nodeSeqNo; }

	void setSeqNo (int seqNo) { nodeSeqNo = seqNo; }

	int width() const { return nodeWidth; }

	void setWidth(int width) { nodeWidth = width; }

	int step() const { return nodeStep; }

	void setStep(int step) { nodeStep = step; }

	int opecode() const { return nodeOpecode; }

	void setOpecode(int code) { nodeOpecode = code; }

	double weight() const { return nodeWeight; }

	void setWeight(double weight) { nodeWeight = weight; }

	//----------------------------------------

	int sourceSize() const 
	
	{ return nodeSource.size(); }

	void addSource(DFGVex * source) 
	
	{ nodeSource.push_back(source); }

	DFGVex * const & sources(int seqNo) const 
	
	{ return nodeSource.at(seqNo); }

	DFGVex * & sources(int seqNo) 
	
	{ return nodeSource.at(seqNo); }

	const Vector<DFGVex*> & sources() const 
	
	{ return nodeSource; }

	Vector<DFGVex*> & sources() 
	
	{ return nodeSource; }

	void removeSource(DFGVex * source);

	//-----------------------------------------

	int targetSize() const 
	
	{ return nodeTarget.size(); }

	void addTarget(DFGVex * target) 
	
	{ nodeTarget.push_back(target); }

	DFGVex * const & targets(int seqNo) const 
	
	{ return nodeTarget.at(seqNo); }

	DFGVex * & targets(int seqNo) 
	
	{ return nodeTarget.at(seqNo); }

	const Vector<DFGVex*> & targets() const 
	
	{ return nodeTarget; } 

	Vector<DFGVex*> & targets() { return nodeTarget; }

	void removeTarget(DFGVex * target);

	//------------------------------------------

	RCANode * rcaNode() const { return nodeRC; }

	void setRCANode(RCANode * node) { nodeRC = node; }

	bool placed() const { return isPlaced; }

	void setPlaced(bool placed) { isPlaced = placed; }

	DFGVex * copy() const { return new DFGNode(*this); }

	void print(std::ostream & out) const

	{ (nodeSeqNo < 0)? out<<"B"<<-nodeSeqNo : out<<"V"<<nodeSeqNo; }

private:
	
	int nodeSeqNo;
	int nodeWidth;
	int nodeStep;
	int nodeOpecode;
	double nodeWeight;

	bool isPlaced;

	RCANode * nodeRC;

	Vector<DFGVex*> nodeSource; 
	Vector<DFGVex*> nodeTarget;
};

inline void DFGNode::removeSource(DFGVex * source){

	Vector<DFGVex*>::iterator iter, end;

	end = nodeSource.end();
	for(iter = nodeSource.begin(); iter != end; ++ iter)
		if(*iter == source)break;

	nodeSource.erase(iter);
}

inline void DFGNode::removeTarget(DFGVex * target){

	Vector<DFGVex*>::iterator iter, end;

	end = nodeTarget.end();
	for(iter = nodeTarget.begin(); iter != end; ++ iter)
		if(*iter == target)break;

	nodeTarget.erase(iter);
}

//////////////////////////////////////////////////////////////////////////////


#endif
