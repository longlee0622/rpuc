#include "rpucfg.h"

static bool rcaPortLess(
	RCAPort & left, RCAPort & right){ // uesd for sort()

	return left.dfgPort()->id() < right.dfgPort()->id();
}

static bool rcaPortEquate(
	RCAPort & left, RCAPort & right){ // uesd for unique

	return left.dfgPort()->id() == right.dfgPort()->id();
}


int interRCANodeIndex = 5000;
//////////////////////////////////////////////////////////////
int RPUConfig::ExPortTrans()
{
	List<Ptr<RCA> > & RCAS = rcaList;
	Vector<RCA*> rcas;
	for (List<Ptr<RCA> >::iterator rcaIter = RCAS.begin();rcaIter !=RCAS.end();++rcaIter)
	{
		RCA * thisRCA =rcaIter->get();
		rcas.push_back(thisRCA);
	}
	for (Vector<RCA*>::reverse_iterator rIter = rcas.rbegin();rIter !=rcas.rend();++rIter)
	{
		RCA * thisRCA = *rIter;
		if(thisRCA->seqNo() == 0) break;	//第一个RCA，不用处理
		RCA * PreRCA =  *(rIter+1);
		for (int rc = 0;rc < 2 * RC_REG_NUM;++rc)
		{
			RCANode * thisNode = (rc < RC_REG_NUM) ?
				& thisRCA->node(rc) : & thisRCA->temp(rc - RC_REG_NUM);
			DFGNode * dfgNode = thisNode->dfgNode();
			if(dfgNode == 0) continue;
			for(int k = 0;k<dfgNode->sources().size();++k)
			{
				DFGVex * thisSRC = dfgNode->sources(k);
				if (typeid(*thisSRC) != typeid(DFGNode))
				{
					bool isImm = static_cast<DFGPort*>(thisSRC)->isImmPort();
					if(!isImm)
					{
						int empty = PreRCA->findCurEmptyNode();
						assert(empty !=-1);
						DFGNode * bypNode = newBypsNode();
						bypNode->setSeqNo(interRCANodeIndex++);
						bypNode->addSource(thisSRC);
						bypNode->addTarget(dfgNode);
						dfgNode->sources(k) = bypNode;
						PreRCA->temp(empty).setDFGNode(bypNode);
					}
				}
			}
		}
		
	}
	return 0;
}
int RPUConfig::AddInterRCANode()
{
	List<Ptr<RCA> > & rcas = rcaList;
	Vector<RCA*> rcaVecs;
	for(List<Ptr<RCA> >::iterator rcaIter=rcas.begin();rcaIter !=rcas.end();++rcaIter)
	{
		RCA * thisRCA = rcaIter->get();
		rcaVecs.push_back(thisRCA);
	}
	RCA * PreRCA = 0;
	for(List<Ptr<RCA> >::iterator rcaIter=rcas.begin();rcaIter !=rcas.end();++rcaIter)
	{
		RCA * thisRCA = rcaIter->get();
		for(int rc = 0;rc <2 * RC_REG_NUM;++rc)
		{
			RCANode * thisNode = (rc < RC_REG_NUM) ?
				& thisRCA->node(rc) : & thisRCA->temp(rc - RC_REG_NUM);
			DFGNode * dfgNode = thisNode->dfgNode();
			if(dfgNode == 0) continue;
			for (int k = 0;k<dfgNode->sources().size();++k)
			{
				DFGVex * thisSrc = dfgNode->sources(k);

				if(typeid(*thisSrc) == typeid(DFGNode))
				{

					DFGNode * srcNode = static_cast<DFGNode*>(thisSrc);
					assert(srcNode != 0);

					RCA * srcRCA = srcNode->rcaNode()->rca();
					if(srcRCA != 0 && (srcRCA->seqNo() +2 == thisRCA->seqNo()))		//针对RCA2-》RCA4这种差2的情况专门处理
					{
						assert(PreRCA->seqNo()+1 == thisRCA->seqNo());
						int empty = PreRCA->findCurEmptyNode();
						assert(empty !=-1);
						DFGNode * bypNode = newBypsNode();
						bypNode->setSeqNo(interRCANodeIndex++);
						for (int c = 0;c<srcNode->targets().size();++c)
						{
							if(srcNode->targets().at(c) == dfgNode) srcNode->targets().at(c)=bypNode;
						}

						bypNode->addSource(srcNode);
						bypNode->addTarget(dfgNode);
						for (int c = 0;c<dfgNode->sources().size();++c)
						{
							if(dfgNode->sources().at(c) == srcNode) dfgNode->sources().at(c)=bypNode;
						}
						PreRCA->temp(empty).setDFGNode(bypNode);

						
					}
					if(srcRCA != 0 && (srcRCA->seqNo() +3 == thisRCA->seqNo()))		//针对RCA2-》RCA5这种差3的情况专门处理
					{
						RCA * Pre2RCA = 0;
						for (Vector<RCA*>::iterator rIter = rcaVecs.begin();rIter !=rcaVecs.end();++rIter)
						{
							if((*rIter)->seqNo() == thisRCA->seqNo()-2) 
							{
								Pre2RCA = *rIter;
								break;
							}
						}
						assert(Pre2RCA != 0);
						assert(PreRCA->seqNo()+1 == thisRCA->seqNo());
						int empty2 = Pre2RCA->findCurEmptyNode();
						assert(empty2 != -1);
						DFGNode * bypNode2 = newBypsNode();
						bypNode2->setSeqNo(interRCANodeIndex++);
						for (int c = 0;c<srcNode->targets().size();++c)
						{
							if(srcNode->targets().at(c) == dfgNode) srcNode->targets().at(c)=bypNode2;
						}
						bypNode2->addSource(srcNode);
						bypNode2->addTarget(dfgNode);
						for (int c = 0;c<dfgNode->sources().size();++c)
						{
							if(dfgNode->sources().at(c) == srcNode) dfgNode->sources().at(c)=bypNode2;
						}
						Pre2RCA->temp(empty2).setDFGNode(bypNode2);
						
						int empty = PreRCA->findCurEmptyNode();
						assert(empty !=-1);
						DFGNode * bypNode = newBypsNode();
						bypNode->setSeqNo(interRCANodeIndex++);
						for (int c = 0;c<bypNode2->targets().size();++c)
						{
							if(bypNode2->targets().at(c) == dfgNode) bypNode2->targets().at(c)=bypNode;
						}

						bypNode->addSource(bypNode2);
						bypNode->addTarget(dfgNode);
						for (int c = 0;c<dfgNode->sources().size();++c)
						{
							if(dfgNode->sources().at(c) == bypNode2) dfgNode->sources().at(c)=bypNode;
						}
						PreRCA->temp(empty).setDFGNode(bypNode);
						

					}
				}	
			}
		}
		PreRCA=rcaIter->get();
	}
	return 0;
}

int RPUConfig::connectRCA()
{

	List<Ptr<RCA> > & rcas = rcaList;
	const int numRCA = rcas.size();

	Vector<List<RCA*> > rcaSrcList(numRCA), rcaTgtList(numRCA);
	
	int i =0;
	List<Ptr<RCA> >::iterator rcaIter;

	for(rcaIter =rcas.begin();rcaIter != rcas.end(); ++ rcaIter, ++ i)
	{

		RCA * thisRCA = rcaIter->get();
		
		/// Split nodes between different RCAs
		//---------------------------------
		for(int rc =0; rc <2*RC_REG_NUM; ++ rc)
		{

			RCANode * thisNode = (rc < RC_REG_NUM) ?
				& thisRCA->node(rc) : & thisRCA->temp(rc - RC_REG_NUM);
			DFGNode * dfgNode = thisNode->dfgNode();

			if(dfgNode == 0)continue;
	
			const int srcSize = dfgNode->sourceSize();
			for(int src =0; src < srcSize; ++ src)
			{

				DFGVex * thisSrc = dfgNode->sources(src);

				if(typeid(*thisSrc) == typeid(DFGNode))
				{

					DFGNode * srcNode = static_cast<DFGNode*>(thisSrc);
					assert(srcNode != 0);

					RCA * srcRCA = srcNode->rcaNode()->rca();
					if(srcRCA != 0 && srcRCA != thisRCA)
					{

						rcaSrcList[thisRCA->seqNo()].push_back(srcRCA);
						rcaTgtList[srcRCA->seqNo()].push_back(thisRCA);

						cutSource(dfgNode, src);
					}
				}	
			}

		}
	}

	for(rcaIter =rcas.begin(), i =0; rcaIter != rcas.end(); ++ rcaIter, ++ i)
	{

		List<RCAPort> inport, outport;
		RCA * thisRCA = rcaIter->get();

		/// collect the port
		//---------------------------------
		for(int rc =0; rc <RC_REG_NUM; ++ rc)
		{

			RCANode * thisNode = & thisRCA->node(rc);
			RCANode * thisTemp= & thisRCA->temp(rc);

			DFGNode * dfgNode = thisNode->dfgNode();
			DFGNode * dfgByps = thisTemp->dfgNode();

			//if(dfgNode == 0)continue;
	
			if(dfgNode != 0)
			{/// Source
				const int srcSize = dfgNode->sourceSize();
				for(int src =0; src < srcSize; ++ src)
				{

					DFGVex * thisSrc = dfgNode->sources(src);

					if(typeid(*thisSrc) != typeid(DFGNode))
						inport.push_back( 
							RCAPort(static_cast<DFGPort*>(thisSrc)) 
						);
				}

				/// Node Target
				int tgtSize = dfgNode->targetSize();
				for(int tgt =0; tgt < tgtSize; ++ tgt)
				{

					DFGVex * nodeTgt = dfgNode->targets(tgt);

					if(typeid(*nodeTgt) != typeid(DFGNode))
						outport.push_back( 
							RCAPort(static_cast<DFGPort*>(nodeTgt)) 
						);
				}
			}

			
			if(dfgByps != 0)
			{

				//Temp Source
				int srcSizeTemp = dfgByps->sourceSize();
				for (int src =0; src < srcSizeTemp; ++ src)
				{
					DFGVex * tempSrc = dfgByps->sources(src);
					if(typeid(*tempSrc) != typeid(DFGNode))
						inport.push_back(
							RCAPort(static_cast<DFGPort*>(tempSrc))
							);
				}
				// Temp Target
				int tgtSize = dfgByps->targetSize();
				for(int tgt =0; tgt < tgtSize; ++ tgt)
				{

					DFGVex * tempTgt = dfgByps->targets(tgt);

					if(typeid(*tempTgt) != typeid(DFGNode))
						outport.push_back( 
							RCAPort(static_cast<DFGPort*>(tempTgt)) 
						);
				}
			}

		}

		/// Copy to RCA
		//------------------------------------------

		// RCA
		List<RCA*> & rcaIn = rcaSrcList[i];
		rcaIn.sort();
		rcaIn.unique();

		thisRCA->sources().resize(rcaIn.size());
		std::copy(rcaIn.begin(), rcaIn.end(), 
			thisRCA->sources().begin());
		

		List<RCA*> & rcaOut = rcaTgtList[i];
		rcaOut.sort();
		rcaOut.unique();

		thisRCA->targets().resize(rcaOut.size());
		std::copy(rcaOut.begin(), rcaOut.end(), 
			thisRCA->targets().begin());	

		// Ports

		// Inport
		inport.sort(rcaPortLess);
		inport.unique(rcaPortEquate);

		thisRCA->inports().resize(inport.size());
		std::copy(inport.begin(), inport.end(), 
			thisRCA->inports().begin());
		
		// Outport
		outport.sort(rcaPortLess);
		outport.unique(rcaPortEquate);

		thisRCA->outports().resize(outport.size());
		std::copy(outport.begin(), outport.end(), 
			thisRCA->outports().begin());
	}

	return 0;
}

