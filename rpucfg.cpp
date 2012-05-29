
#include "rpucfg.h"

#include "optmz.h"
#include "split.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <string>


/// Basic routine for inner port
//------------------------------------------------
static const String innerPortNamePrefix = "_i_";
static const String externPortNmaePrefix = "ExternalTemp_";

static const String InnerPortName(DFGNode * node){
	std::stringstream ss;
	ss <<innerPortNamePrefix<<*node;

	return ss.str();
}

bool IsInnerPort(DFGPort * port){
	assert(port != 0);
	
	if( typeid(*port) != typeid(DFGVarPort) )	return false;
	 	
	DFGVarPort * varPort = static_cast<DFGVarPort*>(port);
	
	assert(port != 0);

	const std::size_t prefixSize = innerPortNamePrefix.size();
	if(varPort->name().size() < prefixSize)return false;

	if( 0 == varPort->name().compare(0, 
		prefixSize, innerPortNamePrefix) )return true;
	
	return false;
}

void PortStyleSwitch( DFGPort *port ){
	assert( port != 0 );

	if( typeid(*port) != typeid(DFGVarPort) )	return;
	
	DFGVarPort * varPort = static_cast<DFGVarPort*>(port);
	
	assert( port != 0 );

	const std::size_t prefixsize = externPortNmaePrefix.size();

	//2011.5.18 liuxie  ·ÀÖ¹ÖØ¸´ÃüÃû

	if( 0 == varPort->name().compare( 0, prefixsize, externPortNmaePrefix ) ) 
	{
		return;
	}
	//const String externPortNamePrefix = externPortNmaePrefix + innerPortNamePrefix;//yin0901
	const String externPortNamePrefix = externPortNmaePrefix + varPort->name();// yin0901
	varPort->setName( externPortNamePrefix );
	return;	
}

bool IsTempExternPort (DFGPort *port){
	assert( port != 0 );

	if( typeid(*port) != typeid(DFGVarPort) )	return false;
	 	
	DFGVarPort * varPort = static_cast<DFGVarPort * >(port);
	
	assert(port != 0);

	const std::size_t prefixsize = externPortNmaePrefix.size();

    if( varPort->name().size() < prefixsize) return false;
	
	if( 0 == varPort->name().compare( 0, 
		prefixsize, externPortNmaePrefix ) ) return true;
	
	return false;

}

////////////////////////////////////////////////////////////////

RCA * RPUConfig::newRCA() {

	RCA * newRCA = new RCA(rcaList.size());

	rcaList.push_back(Ptr<RCA>());
	rcaList.back().reset(newRCA);

	return newRCA;
}

/*
RCA * RPUConfig::InsertRCAInGroup()
{
	RCA * newRca = new RCA(rcaList_Group.size());

	rcaList_Group.push_back(Ptr<RCA>());
	rcaList_Group.back().reset(newRca);

	return newRca;
}
*/

DFGNode * RPUConfig::newFakeNode() {

	DFGNode * fake = new DFGNode(ADD);

	bypassNodes.push_back(Ptr<DFGNode>());
	bypassNodes.back().reset(fake);
	return fake;
}

DFGNode * RPUConfig::newBypsNode() {

	DFGNode * bypass = new DFGNode(BYP);

	bypassNodes.push_back(Ptr<DFGNode>());
	bypassNodes.back().reset(bypass);

	return bypass;
}

DFGPort * RPUConfig::getNodePort(DFGNode * node){
	using std::map;

	map<DFGNode*, DFGPort*>::iterator
		portIter = nodePortTable.find(node);

	if(portIter == nodePortTable.end())
		return insertPortOf (node);
	else return portIter->second;
}

DFGNode * RPUConfig::getNodeByps (DFGNode * node){

	std::map<DFGNode*, DFGNode*>::iterator
		bypIter = nodeBypsTable.find(node);

	if(bypIter == nodeBypsTable.end()) return node;
	else return bypIter->second;
}


DFGPort * RPUConfig::cutSource(DFGNode * node, int srcIndex){
	DFGVex * srcVex = node->sources(srcIndex);

	if(typeid(*srcVex) != typeid(DFGNode))
		return static_cast<DFGPort*>(srcVex);

	DFGNode * srcNode = static_cast<DFGNode*>(srcVex);

	Vector<DFGVex*>::iterator thisNodeIter, end;
	end = srcNode->targets().end();
	for(thisNodeIter = srcNode->targets().begin(); 
		thisNodeIter != end; ++ thisNodeIter)
		if(*thisNodeIter == node)break;

	assert(thisNodeIter != end); /// Not found this node!

	DFGPort * nodePort = getNodePort(srcNode);	


	//yin0921begin
	// Check whether has a port
	/*Vector<DFGVex*>::iterator vexIter, vexEnd;
	vexEnd = srcNode->targets().end();
	
	for(vexIter = srcNode->targets().begin(); 
		vexIter != vexEnd; ++ vexIter){

		if(typeid(**vexIter) != typeid(DFGNode))break;
	}

	if(vexIter == vexEnd)(*thisNodeIter) = nodePort; //Port not found
	else srcNode->removeTarget(node);
	*/

	(*thisNodeIter) = nodePort; //added by yin 0921


	//yin0921end


	node->sources(srcIndex) = nodePort;
	return nodePort; 
}

DFGPort * RPUConfig::insertPortOf (DFGNode * node){

	Vector<DFGVex*>::iterator iter, end;
	end = node->targets().end();

	DFGPort * port = new DFGVarPort(InnerPortName(node));

	innerDFGPorts.push_back(Ptr<DFGPort>());
	innerDFGPorts.back().reset(port);

	nodePortTable.insert(
		std::map<DFGNode*, DFGPort*>::value_type(node, port)
	);

	return port;
}

///----------------------------------------------------
DFGNode * RPUConfig::insertBypassBetween(
		DFGNode * srcNode, DFGNode * tgtNode
	){

	// Check the relation
	Vector<DFGVex*>::iterator srcIter, tgtIter;

	for(tgtIter = srcNode->targets().begin();
		tgtIter != srcNode->targets().end(); ++ tgtIter){

			if(*tgtIter == tgtNode)break;
	}

	assert(tgtIter != srcNode->targets().end());

	for(srcIter = tgtNode->sources().begin();
		srcIter != tgtNode->sources().end(); ++ srcIter){

			if(*srcIter == srcNode)break;
	}

	assert(srcIter != tgtNode->sources().end());

	if(tgtNode->rcaNode()->row() 
		- srcNode->rcaNode()->row() == 1)return srcNode;

	// Create bypass node

	std::map<DFGNode*, DFGNode*>::iterator
		bypIter = nodeBypsTable.find(srcNode);

	DFGNode * frontNode = 0;

	if(bypIter == nodeBypsTable.end())frontNode = srcNode;
	else frontNode = bypIter->second;

	assert(tgtNode->rcaNode() != 0);
	assert(frontNode->rcaNode() != 0);

	int distance = 
		tgtNode->rcaNode()->row() - frontNode->rcaNode()->row();

	srcNode->removeTarget(*tgtIter);

	if(distance > 1){

		while(distance != 1){
			DFGNode * bypNode = new DFGNode(BYP);

			bypassNodes.push_back(Ptr<DFGNode>());
			bypassNodes.back().reset(bypNode);

			bypNode->setSeqNo( - frontNode->seqNo());
			// Minus means this node is a bypass node

			bypNode->setStep(frontNode->step() + 1);
			
			bypNode->sources().push_back(frontNode);
			frontNode->targets().push_back(bypNode);

			frontNode = bypNode;

			-- distance;
		}	

		nodeBypsTable[srcNode] = frontNode;
	} else {

		while(distance != 1){
			frontNode = static_cast<DFGNode*>(
				frontNode->sources().front()
			);

			++ distance;
		}

	}

	*srcIter = frontNode;
	frontNode->targets().push_back(tgtNode);

	return frontNode;
	
}


int RPUConfig::mapDFGraph(
		const OptimizeMethod & optmize, 
		const SplitMethod & split
	){

	int error = 0;
	//error |= optmize.optimizeDFG(dfgGraph);
	error |= split.splitDFG(*this);

	if(error)return error;

	error |= AddInterRCANode();

	error |= connectRCA();


#ifndef NDEBUG

	// Check the splition  result
	//-----------------------------------------------------
	// Rule 1: Step between adjecent RC has dependence 
	//         of date must be 1;
	// Rule 2: No cycle relation of data dependence exists 
	//         between any two RCA. 

	bool pass = true;
	int errorSum = 0;

	List<Ptr<RCA> >::iterator rcaIter = rcaList.begin();
	List<Ptr<RCA> >::iterator rcaEnd = rcaList.end();

	for(; rcaIter != rcaEnd; ++ rcaIter){

		RCA * thisRCA = rcaIter->get();

		for(int r =0; r <RCA_ROW; ++ r)
			for(int c =0; c <RCA_COL; ++c){

				//chage by liuxie 2010.11.22 for debug
				/*if( r==3 && c==8 )
				{
					printf("The bad place at rpucfg.cpp line 341!\n");
				}*/
				RCANode & curRCANode = thisRCA->node(r, c);
				DFGNode * thisDFGNode = curRCANode.dfgNode();

				if(thisDFGNode == 0)continue;

				const int thisSeqNo = thisRCA->seqNo();

				Vector<DFGVex*>::iterator tgtIter, tgtEnd;
				tgtIter = thisDFGNode->targets().begin();
				tgtEnd = thisDFGNode->targets().end();

				for(; tgtIter != tgtEnd; ++ tgtIter){

					DFGVex * thisTgtVex = *tgtIter;

					if(typeid(*thisTgtVex) != typeid(DFGNode))continue;

					DFGNode * thisTgt = static_cast<DFGNode*>(thisTgtVex);

					assert(thisTgt != 0);

					if(thisTgt->rcaNode()->row() != (r+1)%RCA_ROW){

						pass = false;
						++ errorSum;

						std::cerr<<dfgGraph.name()<<" in RCA"<<thisSeqNo
							<<" error "<<errorSum<<": The step distance between Node"
							<<thisDFGNode->seqNo()<<" and Node"<<thisTgt->seqNo()
							<<" is not equal to 1!"<<std::endl;
					}
				}

				Vector<RCA*>::iterator tgtRCAIter, tgtRCAEnd;
				tgtRCAIter = thisRCA->targets().begin();
				tgtRCAEnd = thisRCA->targets().end();

				for(; tgtRCAEnd != tgtRCAEnd; ++ tgtRCAEnd){

					RCA* tgtRCA = (*tgtRCAEnd);

					const int tgtSeqNo = tgtRCA->seqNo();
					
					if( tgtSeqNo <= thisSeqNo ){

						pass = false;
						++ errorSum;

						std::cerr<<dfgGraph.name()<<"error "<<errorSum
							<<": RCA"<<tgtSeqNo<<" need output data of RCA"<<thisSeqNo
							<<" but RCA"<<thisSeqNo<<" is executed after RCA"<<tgtSeqNo
							<<std::endl;

						Vector<RCA*>::iterator iter, end;
						iter = tgtRCA->targets().begin();
						end = tgtRCA->targets().end();

						for(; iter != end; ++iter){

							if((*iter) == thisRCA){

								++ errorSum;

								std::cerr<<dfgGraph.name()<<"error "<<errorSum
									<<": Cycle relation of data dependence exists between RCA"
									<<tgtSeqNo<<" and RCA"<<thisSeqNo<<std::endl;
							}
						}
					}
				}

			}
	}

	if(!pass)error = -1;

#endif

	return error;
}


////////////////////////////////////////////////////////////

int RPUConfig::genContext() {

	genGroupContext();
	return 0;
}

/////////////////////////////////////////////////////////////

#define DFG_BASE_ADDR	200000

int RPUConfig::printNodeInfo(const String & fileName){

	std::ofstream debugFile(fileName.c_str());

	const int nodeSize = dfgGraph.nodeSize(); 
	for(int i =0; i < nodeSize; ++i){

		DFGNode & curNode = dfgGraph.node(i);
		RCANode * thisRCANode = curNode.rcaNode();

		assert(thisRCANode != 0);

		RCA * rca = thisRCANode->rca();

		DFGPort * tgtPort = 0;
		// Get target
		for(int t =0; t <curNode.targetSize(); ++ t){

			DFGVex * thisTgt = curNode.targets(t);
			if(typeid(*thisTgt) != typeid(DFGNode)){

				tgtPort = static_cast<DFGVarPort*>(thisTgt);
				assert(tgtPort != 0);

				break;
			}
		}

		int addr = -1;
		if(tgtPort != 0){

			Vector<RCAPort>::iterator portIter;
			for(portIter = rca->outports().begin(), addr = 0; 
				portIter != rca->outports().end(); ++ portIter, ++addr){
					if(portIter->dfgPort() == tgtPort)break;
			}

			assert(portIter != rca->outports().end());
		} 

		debugFile<<DFG_BASE_ADDR + curNode.seqNo()
			     <<": RCA"<<rca->seqNo()
				 <<" RC"<<thisRCANode->row()*RCA_COL+thisRCANode->col()
				 <<" Out["<<addr<<"]\n";
	}

	return 0;
}

//void RPUConfig::display(std::ostream & out) const {

//	out<<"\nRCA Total: "<<rcaList.size()
//		<<"\n###################################################\n";

//	int seqNo = 0;
//	List<Ptr<RCA> >::const_iterator rcaIter;
//	for(rcaIter = rcaList.begin(); 
//		rcaIter != rcaList.end(); ++ rcaIter){

//			out<<"RCA"<<seqNo++<<":\n"
//					 <<"---------------------------------------------------\n";
//			out<<**rcaIter<<std::endl;
//	}
//}

void RPUConfig::display(std::ostream & out) const {

	out<<"\nRCA Total: "<<allRCAVec.size()
		<<"\n###################################################\n";

	int seqNo = 0;
	Vector<RCA* >::const_iterator rcaIter;
	for(rcaIter = allRCAVec.begin(); 
		rcaIter != allRCAVec.end(); ++ rcaIter){

			out<<"RCA"<<seqNo++<<":\n"
					 <<"---------------------------------------------------\n";
			out<<**rcaIter<<std::endl;
	}
}

