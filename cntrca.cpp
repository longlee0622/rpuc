
#include "rpucfg.h"



static bool rcaPortLess(
	RCAPort & left, RCAPort & right){ // uesd for sort()

	return left.dfgPort()->id() < right.dfgPort()->id();
}

static bool rcaPortEquate(
	RCAPort & left, RCAPort & right){ // uesd for unique

	return left.dfgPort()->id() == right.dfgPort()->id();
}


//////////////////////////////////////////////////////////////

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

			if(dfgNode == 0)continue;
	
			/// Source
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

			// Temp Target
			if(dfgByps != 0)
			{

				tgtSize = dfgByps->targetSize();
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

