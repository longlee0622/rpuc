/************************************************
 *			File name: dfgraph.cpp
 *-----------------------------------------------
 * Description:
 *
 * Author: L. Xie @ MicroE. SJTU
 * Date : 2009/12/21
 *************************************************
 */



#include <cctype>	// for isdigit() & isspace()
#include <typeinfo>
#include <cassert>
#include <iostream>
#include <fstream>

#include <map>

#include "dfgraph.h"
#include "platf.h"
#include "optmz.h"

#define SRC_BASE_ADDR	000000
#define DST_BASE_ADDR	100000
#define DFG_BASE_ADDR	200000

#define ID_NULL		-1
#define ID_SRC      0
#define ID_DST      1
#define ID_NODE     2

extern int ConstMem[];


// Copy construction function
DFGraph::DFGraph(const DFGraph & graph)
	: dfgName(graph.dfgName),
		dfgWidth(graph.dfgWidth),
		dfgDepth(graph.dfgDepth),
		dfgStepWidth(graph.dfgStepWidth),
		inports(graph.inports), 
		outports(graph.outports),
		nodes(graph.nodes){

	// Update adjacent vertex of node
	Vector<Ptr<DFGNode> >::iterator nodeIter;

	for(nodeIter = nodes.begin(); nodeIter != nodes.end(); ++nodeIter){
		DFGNode * thisNode = nodeIter->get();

		// Reset source
		Vector<DFGVex*>::iterator adjIter;
		for(adjIter = thisNode->sources().begin();
			adjIter != thisNode->sources().end(); ++ adjIter){

				if(typeid(**adjIter) == typeid(DFGNode)){

					DFGNode * srcNode = static_cast<DFGNode*>(*adjIter);
					*adjIter = nodes[srcNode->seqNo()].get();
				} else {

					DFGPort * srcPort = static_cast<DFGPort*>(*adjIter);
					*adjIter = inports[srcPort->seqNo()].get();
				}
		}
		
		// Reset targets
		for(adjIter = thisNode->targets().begin();
			adjIter != thisNode->targets().end(); ++ adjIter){

				if(typeid(**adjIter) == typeid(DFGNode)){

					DFGNode * tgtNode = static_cast<DFGNode*>(*adjIter);
					*adjIter = nodes[tgtNode->seqNo()].get();
				} else {

					DFGPort * tgtPort = static_cast<DFGPort*>(*adjIter);
					*adjIter = outports[tgtPort->seqNo()].get();
				}
		}
	}
}


// Judge the type of a address
static int addressType(int addr){
	if(addr >= DFG_BASE_ADDR)return ID_NODE;
	else if(addr >= DST_BASE_ADDR) return ID_DST;
	else if(addr >= SRC_BASE_ADDR) return ID_SRC;
	else return ID_NULL;
}

///Class: DFGraph
//------------------------------------------------------

int DFGraph::parse(std::istream & dfgFile){

	// Read DFG name
	String dfgNode;
	dfgFile>>dfgNode;

	// Read Step
	//----------------------------------------------
	int numStep;

	dfgFile>>numStep;
	dfgStepWidth.resize(numStep, 0);

	dfgWidth = 0;
	for(int i=0; i<numStep; ++i)
	{
		int indexStep, numStepNode;

		dfgFile>>indexStep>>numStepNode;
		assert(i == indexStep);

		dfgStepWidth.at(indexStep) = numStepNode;
		if(dfgWidth < numStepNode)dfgWidth = numStepNode;
	}

	// Read Source
	//-----------------------------------------------
	int numSource;
	dfgFile>>numSource;
	inports.resize(numSource);

	Vector<Vector<int> > srcTargets(numSource); // used later
	int j=0,k=-10;
	for(int i=0; i<numSource; ++i)
	{
		int indexSrc, immdt;
		String varName;

		// Read number
		dfgFile>>indexSrc;

		assert(i == indexSrc);	

		// Find the first not-space charactor 
		char firstChar;
		while( isspace(firstChar = dfgFile.get()) );
		dfgFile.unget();

		if(isdigit(firstChar)){ // OK, it's an immediate

			dfgFile>>immdt;
			inports[i].reset(new DFGImmPort(immdt));
			inports[i]->setPortType(true);
			inports[i]->setSeqNo(k--);			//2012.5.7 longlee 立即数从-10递减编号
			
			int exist=0;
			int m=0;
			for (;ConstMem[m]!=-1;++m)
			{
				if(ConstMem[m]==immdt)
				{
					exist=1;//说明CM中已经有了这个立即数
					break;
				}
			}

			if(!exist)
			{
				if(m<=15) ConstMem[m]=immdt;
				else 
				{
					std::cerr<<"too many immdt inputs(>16),no more room for it!!"<<std::endl;
					exit(0);
				}
			}

		} else { // It's a variable 

			dfgFile>>varName;
			inports[i].reset(new DFGVarPort(varName));
			inports[i]->setSeqNo(j++);			//2012.5.7 longlee 外部输入从0开始递增编号
		}

		while(dfgFile.get() != '\n'); // ignore the rest part
	}

	assert(-10 -k+ j==numSource);		//2012.5.7 longlee 确认immdt 和ext输入的个数符合实际情况
	// Read DFG
	//------------------------------------------------
	int numNodes;
	dfgFile>>numNodes;

	nodes.resize(numNodes);

	Vector<int> stepWidth(numNodes, 0);
	Vector<int> src1(numNodes), src2(numNodes);
	Vector<Vector<int> > tgtNo(numNodes);

	for(int i=0; i <numNodes; ++i) { // Create vertex

		int indexNode, step, opd, numTarget;

		dfgFile>>indexNode>>step>>opd>>src1[i]>>src2[i]>>numTarget;

		assert(i == indexNode - DFG_BASE_ADDR);
		assert(step >= 0 && step <numStep);

		nodes[i].reset(new DFGNode);
		nodes[i]->setSeqNo(i);
		nodes[i]->setStep(step);
		nodes[i]->setWidth(stepWidth[step] ++);

		int opecode;
        
		switch(opd){
			case 6: opecode = ADD;     break;
			case 7: opecode = SUB;     break;
			case 9: opecode = LMT;     break;
			case 10: opecode = HMT;	   break;
			case 20: opecode = LSR;    break;
			case 21: opecode = LSL;    break;
			case 22: opecode = BYP;    break;
			case 28: opecode = EQU;    break;                     //28:  ==
			case 29: opecode = NEQU;   break;                     //29:  !=
			//change by liuxie 2010.11.23
			case 30: opecode = RGNT;   break;                     //30:  <
			//case 31: opecode = GNT;    break;                     //31:  >
			case 31: opecode = RGTE;   break;                     //32:  <=
			case 34: opecode = EQU;    break;                     //34:  ==
			case 36: opecode = GTE;    break;                     //36:  >=
			case 37: opecode = GNT;    break;                     //37:  >
			case 38: opecode = RGTE;   break;                     //38:  <=
			default:
				std::cerr<<"Unkonw opecode \""<<opd<<"\"!\n";
				return -1; // opecode is not supported!
		}

		nodes[i]->setOpecode(opecode);

		tgtNo[i].resize(numTarget);
		for(int j =0; j <numTarget; ++j)dfgFile>>tgtNo[i][j];
	
	}

	// Read destination
	//------------------------------------------------
	int numDest;
	dfgFile>>numDest;
	outports.resize(numDest);

	Vector<int> srcOpd(numDest);
	std::map<int, int> tgtID2SeqNo;

	for(int i =0; i <numDest; ++i) // Create output ports
	{;
		int DestID;
		String varName;

		dfgFile>>DestID>>varName>>srcOpd[i];
		
		outports[i].reset(new DFGVarPort(varName));
		outports[i]->setSeqNo(i);

		tgtID2SeqNo[DestID] = i;
	}

	// Read DFG name
	dfgFile>>dfgName;
	dfgName += "_"+dfgNode;

	// Connect ports and vertexs;
	//--------------------------------------------------

	//inports/nodes ---> nodes
	for(int i =0; i <numNodes; ++i)
	{
		
		//Set sources
		int seqNo;

		// Source1
		if(addressType(src1[i]) == ID_SRC){ // It's a port
	
			seqNo = src1[i]-SRC_BASE_ADDR;
			assert(seqNo >= 0 && seqNo < numSource);
			nodes[i]->sources().push_back(inports[seqNo].get());

		} else { // It's a DFG node

			assert(addressType(src1[i]) == ID_NODE);

			seqNo = src1[i]-DFG_BASE_ADDR;
			assert(seqNo >= 0 && seqNo < numNodes);

			nodes[i]->sources().push_back(nodes[seqNo].get());
		}

		// Source2
		if(addressType(src2[i]) == ID_SRC){ // It's a port
	
			seqNo = src2[i]-SRC_BASE_ADDR;
			assert(seqNo >= 0 && seqNo < numSource);
			nodes[i]->sources().push_back(inports[seqNo].get());

		} else if(addressType(src2[i]) == ID_NULL){
			//BYP节点的输入只有一个，后续判断中以此为依据；
		}else{ // It's a DFG node

			assert(addressType(src2[i]) == ID_NODE);

			seqNo = src2[i]-DFG_BASE_ADDR;
			assert(seqNo >= 0 && seqNo < numNodes);

			nodes[i]->sources().push_back(nodes[seqNo].get());
		}

		// Targets
		for(std::size_t j =0; j <tgtNo[i].size(); ++j){

			if(addressType(tgtNo[i][j]) == ID_DST){ // It is a port

				seqNo = tgtID2SeqNo[tgtNo[i][j]];
				assert(seqNo >= 0 && seqNo < numDest);

				nodes[i]->targets().push_back(outports[seqNo].get());
	
			//	nodes[i]->setOutport(outports[seqNo].get());
			} else { // It is a node

				assert(addressType(tgtNo[i][j]) == ID_NODE);

				seqNo = tgtNo[i][j] - DFG_BASE_ADDR;
				assert(seqNo >= 0 && seqNo < numNodes);

				nodes[i]->targets().push_back(nodes[seqNo].get());
			}
		}
	}

	if(!dfgFile.good())return -1;

	return 0;
}

//-----------------------------------------------------------------
static inline bool hasSorted(const DFGNode & node){
	return node.step() != -1 && node.width() != -1;
}

static void findNextStep(
	const List<DFGNode*> & frontStep, List<DFGNode*> & nextStep, int stepNo
){

	List<DFGNode*> tgtNode;
	List<DFGNode*>::const_iterator nodeIter;
	for(nodeIter = frontStep.begin(); 
		nodeIter != frontStep.end(); ++ nodeIter){

		Vector<DFGVex*>::const_iterator tgtIter;
		for(tgtIter = (*nodeIter)->targets().begin();
			tgtIter != (*nodeIter)->targets().end(); ++tgtIter){

			if(typeid(**tgtIter)==typeid(DFGNode) )
				tgtNode.push_back(static_cast<DFGNode*>(*tgtIter));
		}
	}

	tgtNode.sort();
	tgtNode.unique();

	nextStep.clear();

	int stepWidth = 0;
	List<DFGNode*>::iterator iter;
	for(iter = tgtNode.begin(); iter != tgtNode.end(); ++ iter){

		bool needSorted = true;
		DFGNode* thisNode = *iter;

		// Check this source whether is sorted node or a port.
		Vector<DFGVex*>::iterator srcIter;
		for(srcIter = thisNode->sources().begin(); 
			srcIter != thisNode->sources().end(); ++ srcIter){

				DFGNode * srcNode = dynamic_cast<DFGNode*>(*srcIter);
				if( srcNode && 
					(! hasSorted(*srcNode) || srcNode->step() == stepNo) ){

					needSorted = false;
					break;
				}
		}

		if(needSorted){
			thisNode->setStep(stepNo);
			thisNode->setWidth(stepWidth ++);
			nextStep.push_back(thisNode);
		}
	}
}

List<List<DFGNode*> > DFGraph::sort(){
	List<List<DFGNode*> > nodeLocation;

	///(1) Find the nodes in first step which inputs 
	///    are only ports
	nodeLocation.clear();
	nodeLocation.push_back(List<DFGNode*>()); // first step

	List<List<DFGNode*> >::iterator 
		curStepIter = nodeLocation.begin();

	int step = 0;
	int firstStepWidth = 0;
	Vector<Ptr<DFGNode> >::iterator nodeIter;

	for(nodeIter = nodes.begin(); 
		nodeIter != nodes.end(); ++ nodeIter){

		bool hasNodeIn = false;
		Vector<DFGVex*>::iterator inIter;
		for(inIter = (*nodeIter)->sources().begin();
			inIter != (*nodeIter)->sources().end(); ++ inIter){

				if(typeid(**inIter) == typeid(DFGNode)){
					hasNodeIn = true; 
					break;
				}
		}

		DFGNode * node = (*nodeIter).get();

		if(!hasNodeIn){ // first step	
			node->setStep(step);
			node->setWidth(firstStepWidth ++);
			curStepIter->push_back(node);
		} else { // others
			node->setStep(-1);
			node->setWidth(-1);
		}
	}

	dfgStepWidth.clear();
	dfgStepWidth.push_back(curStepIter->size());

	/// (2) Find nodes in next Step	
	List<DFGNode*> nextStepNodes;
	findNextStep(*curStepIter, nextStepNodes, step + 1);

	dfgWidth = 0; // Set DFG Width
	while(!nextStepNodes.empty()){
		++ step;

		const int size = nextStepNodes.size();

		if(dfgWidth < size)dfgWidth = size;
		nodeLocation.push_back(nextStepNodes);
		dfgStepWidth.push_back(size);

		findNextStep(*(++curStepIter), nextStepNodes, step + 1);	
	}

	/// Set Depth
	dfgDepth = nodeLocation.size();

	return nodeLocation;
}

int DFGraph::optmize(OptimizeMethod * method){

	method->optimizeDFG(*this);

	return 0;
}


void DFGraph::display(std::ostream & out){
	using std::size_t;

	List<List<DFGNode*> > nodeLocation(DFGraph::sort());

	out<<"DFG name: "<<dfgName;
	out<<"\n------------------------------------------\n";

	// Print inputs
	//----------------------------------------------------
	out<<"In ("<<inports.size()<<"):\n";

	const size_t inputSize = inports.size();
	for(size_t i =0; i < inputSize; ++i)out<<"\t"<<*inports[i]<<"\n";
	out.seekp(-2, std::ios::cur);
	out<<std::endl;

	// Print nodes
	//----------------------------------------------------
	const size_t stepSize = nodeLocation.size();

	List<List<DFGNode*> >::iterator stepIter = nodeLocation.begin();
	for(size_t step =0; step <stepSize; ++ step, ++ stepIter){
		out<<"Step"<<step + 1<<" ("<<dfgStepWidth[step]<<"):\n";

		List<DFGNode*>::iterator nodeIter;
		for(nodeIter = stepIter->begin(); 
			nodeIter != stepIter->end(); ++ nodeIter){

			DFGNode * thisNode = *nodeIter;

			out<<"\t"<<*thisNode<<"("; // Print seqNo

			// Print node inports
			for(size_t i =0; i < thisNode->sources().size(); ++i)
				out<<* thisNode->sources(i)<<", ";

			out.seekp(-2, std::ios::cur);
			out<<"\t["<<(*nodeIter)->opecode()<<"]=>\t";

			// Print node outports
			for(size_t i =0; i < thisNode->targets().size(); ++i)
				out<<* thisNode->targets(i)<<", ";

			out.seekp(-2, std::ios::cur);
			out<<")\n";
		}
	}

	// Print outputs
	//----------------------------------------------------
	out<<"Out ("<<outports.size()<<"):\n";

	const size_t outputSize = outports.size();
	for(size_t i =0; i < outputSize; ++i)out<<"\t"<<*outports[i]<<"\n";
	out.seekp(-2, std::ios::cur);
	out<<"\n";

	out<<"------------------------------------------"<<std::endl;
}

DFGPort * DFGraph::newInport(const String & name){

	DFGPort * newPort = new DFGVarPort(name);
	newPort->setSeqNo(inports.size());

	inports.push_back(Ptr<DFGPort>());
	inports.back().reset(newPort);

	return newPort;
}

DFGPort * DFGraph::newOutport(const String & name){

	DFGPort * newPort = new DFGVarPort(name);
	newPort->setSeqNo(outports.size());

	outports.push_back(Ptr<DFGPort>());
	outports.back().reset(newPort);

	return newPort;
}

DFGNode * DFGraph::newNode(int opecode){

	DFGNode * newNode = new DFGNode(opecode);
	newNode->setSeqNo(nodes.size());

	nodes.push_back(Ptr<DFGNode>());
	nodes.back().reset(newNode);

	return newNode;
}
