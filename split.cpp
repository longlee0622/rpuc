/************************************************
 *			File name: split.cpp
 *-----------------------------------------------
 * Description:
 *
 * Author: L. Xie @ MicroE. SJTU
 * Date : 2010/01/06
 *************************************************
 */

#include "split.h"

#include "platf.h"
#include "rpucfg.h"

#include <algorithm>
#include <fstream>

///////////////////////////////////////////////////////

static bool WeightLargerFirst(
	DFGNode * left, DFGNode * right){

		assert(left != 0 && right != 0);

		return (left->weight() == right->weight())?
			left->step() < right->step() 
			: left->weight() > right->weight();
}


/**********************************************************************
 * Algorithm
 * --------------------------------------------------------------------
 * Step 1: if no DFG node need to be placed, end program; 
 *         else create a new RCA.
 * Step 2: Select a valid RC in the RCA, if can't, go to step 1.
 * Step 3: Find a DFG node could be put into the RC selected, 
 *         if can't, go to step 2.
 *
 *        Placement condition:
 *        --------------------------------------------------------
 *        |A: all sources has been placed
 *        |And
 *        |    B: all sources and this node in different RCA
 *        |    Or
 *        |      ^B 
 *        |       And
 *        |       C: RCA row of source all shoule smaller than this node
 *        |       And
 *        |       D: there are enough blank cells for all sources to 
 *        |         insert bypass node if any	
 *        --------------------------------------------------------
 *        canPlaced = A( B + ^BCD) = A(B + CD) = ^(^A + ^B(^C+^D));
 *
 ************************************************************************
 */

/*int SplitMethod::splitDFG( RPUConfig & config ) const {

	DFGraph & graph = config.graph();

	// (1) Static sort node list
	List<List<DFGNode*> > topOrder(graph.sort());
	List<List<DFGNode*> >::iterator stepIter;
	List<DFGNode*>::iterator widthIter;

	List<DFGNode*> allDFGNode;

	if(topOrder.empty())return 0;

	stepIter = topOrder.end();
	for(-- stepIter; stepIter != topOrder.begin();-- stepIter)
	{

		for(widthIter = stepIter->begin(); 
			widthIter != stepIter->end(); ++widthIter)
		{

				DFGNode * thisNode = *widthIter;
				Vector<DFGVex*>::iterator srcIter;

				for(srcIter = thisNode->sources().begin();
					srcIter != thisNode->sources().end(); ++ srcIter)
				{

						DFGVex * srcVex = *srcIter;
						if(typeid(*srcVex) != typeid(DFGNode)) continue;

						DFGNode * srcNode = static_cast<DFGNode*>(srcVex);

						assert(srcNode != 0);

						srcNode->setWeight(srcNode->weight() + thisNode->weight());

				}

		}

		stepIter->sort(WeightLargerFirst);
		allDFGNode.merge(*stepIter, WeightLargerFirst);
	}

	stepIter->sort(WeightLargerFirst);
	allDFGNode.merge(*stepIter, WeightLargerFirst);


	// (2) Fill the RCA one by one

	while(!allDFGNode.empty())
	{	


		RCA * thisRCA = config.newRCA();

		int nodeConuter = 0;

		Vector<int> curColIndex(RCA_ROW, 0), curTempIndex(RCA_ROW, 0);


		// Fill this RCA as possible

		List<DFGNode*>::iterator nodeIter = allDFGNode.begin();
		List<DFGNode*>::iterator endIter = allDFGNode.end();

		for(int curRow = 0;curRow < RCA_ROW; ++ curRow){

			if(allDFGNode.empty())break;

			for(int curCol = curColIndex[curRow]; curCol < RCA_COL; ++ curCol){

				if(allDFGNode.empty())break;

				nodeIter = allDFGNode.begin();
				endIter = allDFGNode.end();	

				while(nodeIter != endIter){

					//change by liuxie 2010.11.22 for debug
					//if( curRow==3 && curCol==8 )
					//{
					//	printf("The bad place at split.cpp line 137!\n");
					//}
					RCANode & curRCANode = thisRCA->node(curRow, curCol);

					if(nodeIter == allDFGNode.end())nodeIter = allDFGNode.begin();
					if(nodeIter == endIter)break;

					DFGNode * thisDFGNode = *nodeIter;

					Vector<DFGVex*>::iterator srcIter, srcEnd;

					// Placement condition:
					//-------------------------------------------------
					// A: all sources has been placed
					// And
					//    B: all sources and this node in different RCA
					//    Or
					//      ^B 
					//		And
					//		C: RCA row of all source shoule smaller 
					//			than this node
					//		And
					//		D: there are enough blank cells for all 
					//			sources to insert bypass node if any	
					//--------------------------------------------------
					// canPlaced = A( B + ^BCD) = A(B + CD) = ^(^A + ^B(^C+^D));

					bool canPlaced = true;
					srcIter = thisDFGNode->sources().begin();
					srcEnd = thisDFGNode->sources().end();

					//当该节点的父节点还没有被放置的时候，该节点不能被映射
					for(; srcIter != srcEnd; ++ srcIter){

						if(typeid(**srcIter) != typeid(DFGNode))continue;

						DFGNode * srcNode = static_cast<DFGNode*>(*srcIter);

						assert(srcNode != 0);

						if( ! srcNode->placed() ){

							canPlaced = false; 
							break;
						}

						RCANode * srcRCANode = srcNode->rcaNode();
						RCA * srcRCA = (srcRCANode)? srcRCANode->rca() : 0;

						if(srcRCA == thisRCA){
							// Check row of this source whether smaller than this node
							if(srcRCANode->row() >= curRow){
								canPlaced = false; 
								break;
							}

							// Check whether has enough blank cell for bypass node
							DFGNode * lastBypsNode = config.getNodeByps(srcNode);
							RCANode * lastBypsRCANode = lastBypsNode->rcaNode();

							assert(lastBypsRCANode != 0);

							for(int i = lastBypsRCANode->row() + 1; i <= curRow; ++ i){

								bool RCANodeFull = curColIndex[i] > RCA_COL -1;
								bool RCATempFull = curTempIndex[i] > RCA_COL -1;

								if(RCANodeFull && RCATempFull){ // No empty blank cell in this row
									canPlaced = false; 
									break;
								}
							}
						} 
					}

					if( !canPlaced)
					{

						if(nodeIter == allDFGNode.end())
							nodeIter = allDFGNode.begin();
						else ++ nodeIter;
					} 
					else { // Place this node into RCA cell

						// Dynamic sort node list
						Vector<DFGVex*>::iterator tgtIter;

						for(tgtIter = thisDFGNode->targets().begin();
							tgtIter != thisDFGNode->targets().end(); ++ tgtIter){

								DFGVex * tgtVex = *tgtIter;
								if(typeid(*tgtVex) != typeid(DFGNode)) continue;

								DFGNode * tgtNode = static_cast<DFGNode*>(tgtVex);
								assert(tgtNode != 0);

								// Find the source node in DFG node list.
								List<DFGNode*>::iterator iter = nodeIter;   //nodeIter == allDFGNode.begin()
                                
                                //change by liuxie 2010.11.23 for debug
								//DFGVex * tempVex = *iter;
								
								for(++iter; iter != endIter; ++ iter){      //endIter == allDFGNode.end()
									
									if(*iter == tgtNode)break;
								}
								//change by liuxie 2010.11.23 for debug
								/*if(iter == endIter)
								{
									
									DFGNode * temp_node =  static_cast<DFGNode*>(tempVex);
									printf("Error: the wrong node is at No. %d \n", temp_node->seqNo());
									printf("Error: the wrong node is at No. %d \n", thisDFGNode->seqNo());
									printf("Error: the wrong node is at No. %d \n", tgtNode->seqNo());
								}*/

/*								assert(iter != endIter); // Must can be found

								List<DFGNode*>::iterator frontIter = iter;;
								
								for(-- frontIter; (*iter)->weight() == 
									(*frontIter)->weight(); iter = frontIter -- ){ // Exchange these two nodes

										DFGNode * swap = *iter;
										*iter = *frontIter;
										*frontIter = swap;
								}

						}

						// Put DFG node into RCA
						assert(curRCANode.dfgNode() == 0);

						curRCANode.setDFGNode(thisDFGNode);
						
						++ curColIndex[curRow];

						nodeIter = allDFGNode.erase(nodeIter);

						nodeConuter ++;

						// Set bypass node
						//--------------------------------------
						//	Source node: A	Taret node: B, C
						//	Bypass node of A: A1, A2, A3
						//
						//	A  X  X  X  X
						//	A1 X  X  X  X
						//	A2 X  X  X  X
						//	A3 C  X  X  X
						//	B  X  X  X  X
						//--------------------------------------
						// In this pattern, lastBypsNode = A3, and
						// node B need attached to A3, while C to A2
						//

						for(srcIter = thisDFGNode->sources().begin(); 
							srcIter != srcEnd; ++ srcIter){

							if(typeid(**srcIter) != typeid(DFGNode))continue; // It's a port

							DFGNode * srcNode = static_cast<DFGNode*>(*srcIter);

							assert(srcNode != 0);

							DFGNode * firstBypsNode = config.getNodeByps(srcNode);

							if(firstBypsNode->rcaNode()->rca() == thisRCA) { 
								// In the same RCA, need to insert bypass node

								DFGNode * lastBypsNode = 
									config.insertBypassBetween(srcNode, thisDFGNode);

								int curBypNodeRow = curRow -1;
								
								while( lastBypsNode != firstBypsNode ){

									assert(curBypNodeRow >= 0);

									bool curRCATempFull = curTempIndex[curBypNodeRow] > RCA_COL -1;

									//chage by liuxie 2010.11.22 for debug
									/*if( curRCATempFull && curBypNodeRow==3 && curColIndex[curBypNodeRow]++ == 8 )
									{
										printf("The bad place at split.cpp line 304!\n");
									}*/

/*									RCANode & curRCABypNode = (curRCATempFull) ?
										thisRCA->node(curBypNodeRow, curColIndex[curBypNodeRow] ++) :
										thisRCA->temp(curBypNodeRow, curTempIndex[curBypNodeRow] ++);

									assert(curRCABypNode.dfgNode() == 0); // It's an empty RC or temp registor

									curRCABypNode.setDFGNode(lastBypsNode);
									
									lastBypsNode = static_cast<DFGNode*>(lastBypsNode->sources().front());

									-- curBypNodeRow;
								}
							}
						}

						break;
					}
				}

			} // End column searching
		} // End row searching

	}

	return 0;
}
*/
int SplitMethod::splitDFG( RPUConfig & config ) const {

	DFGraph & graph = config.graph();
	List<Ptr<RCA> > & rcas = config.rcas();

	// (1) // Static sort node list
	List<List<DFGNode*> > topOrder(graph.sort());
	List<List<DFGNode*> >::iterator stepIter;
	List<DFGNode*>::iterator widthIter;

	List<DFGNode*> allDFGNode;

	if(topOrder.empty())return 0;

	stepIter = topOrder.end();
	for(-- stepIter; stepIter != topOrder.begin();-- stepIter){

		for(widthIter = stepIter->begin(); 
			widthIter != stepIter->end(); ++widthIter){

				DFGNode * thisNode = *widthIter;
				Vector<DFGVex*>::iterator srcIter;

				for(srcIter = thisNode->sources().begin();
					srcIter != thisNode->sources().end(); ++ srcIter){

						DFGVex * srcVex = *srcIter;
						if(typeid(*srcVex) != typeid(DFGNode)) continue;

						DFGNode * srcNode = static_cast<DFGNode*>(srcVex);

						assert(srcNode != 0);

						srcNode->setWeight(srcNode->weight() + thisNode->weight());

				}

		}

		stepIter->sort(WeightLargerFirst);
		allDFGNode.merge(*stepIter, WeightLargerFirst);
	}
   //以上代码得到allDFGNode的链表
	stepIter->sort(WeightLargerFirst);
	allDFGNode.merge(*stepIter, WeightLargerFirst);

	// (2) Fill the RCA one by one

	int rcaSeqNo = 0;

	
	/********************************************需要替代的部分begin****************************************************************/
	std::ofstream outfile("RCAInfo.txt");
	while(!allDFGNode.empty())
	{	

		rcas.push_back(Ptr<RCA>());

		RCA * thisRCA = new RCA(rcaSeqNo++);
		rcas.back().reset(thisRCA);
		outfile<<"RCA No."<<thisRCA->seqNo()<<"\n"<<std::endl;

		Vector<int> curColIndex(RCA_ROW, 0);

		// Fill this RCA as possible

		List<DFGNode*>::iterator nodeIter = allDFGNode.begin();
		List<DFGNode*>::iterator endIter = allDFGNode.end();
		
		for(int curRow = 0;curRow < RCA_ROW; ++ curRow){

			if(allDFGNode.empty())break;

			for(int curCol = curColIndex[curRow]; curCol < RCA_COL; ++ curCol){

				if(allDFGNode.empty())break;

				// Set Start and End
		//		if(nodeIter == endIter){ // Find from the begining

					nodeIter = allDFGNode.begin();
					endIter = allDFGNode.end();	
		/*		} else { // Find from N2

					endIter = nodeIter;

					if(endIter != allDFGNode.begin()) -- endIter;
					else endIter = allDFGNode.end();		
				}*/

				while(nodeIter != endIter)
				{

					RCANode & curRCANode = thisRCA->node(curRow, curCol);

					if(nodeIter == allDFGNode.end())nodeIter = allDFGNode.begin();
					if(nodeIter == endIter)break;

					DFGNode * curDFGNode = *nodeIter;

					Vector<DFGVex*>::iterator srcIter, srcEnd;

					// Placement condition:
					//-------------------------------------------------
					// A: all sources has been placed
					// And
					//    B: all sources and this node in different RCA
					//    Or
					//      ^B 
					//		And
					//		C: RCA row of all source shoule smaller 
					//			than this node
					//		And
					//		D: there are enough blank cells for all 
					//			sources to insert bypass node if any	
					//--------------------------------------------------
					// canPlaced = A( B + ^BCD) = A(B + CD) = ^(^A + ^B(^C+^D));

					bool canPlaced = true;
					srcIter = curDFGNode->sources().begin();
					srcEnd = curDFGNode->sources().end();

					for(; srcIter != srcEnd; ++ srcIter)
					{

						if(typeid(**srcIter) != typeid(DFGNode))continue;

						DFGNode * srcNode = static_cast<DFGNode*>(*srcIter);

						assert(srcNode != 0);

						if( ! srcNode->placed() ){

							canPlaced = false; 
							break;
						}

						RCANode * srcRCANode = srcNode->rcaNode();
						RCA * srcRCA = (srcRCANode)? srcRCANode->rca() : 0;

						if(srcRCA == thisRCA){
							// Check row of this source whether smaller than this node
							if(srcRCANode->row() >= curRow){
								canPlaced = false; 
								break;
							}

							// Check whether has enough blank cell for bypass node
							DFGNode * lastBypsNode = config.getNodeByps(srcNode);
							RCANode * lastBypsRCANode = lastBypsNode->rcaNode();

							assert(lastBypsRCANode != 0);

							for(int i = lastBypsRCANode->row() + 1; i <= curRow; ++ i){

								if(curColIndex[i] >= RCA_COL -1){ // No empty blank cell in this row
									canPlaced = false; 
									break;
								}
							}
						} 
					}

					if(!canPlaced)
					{

						if(nodeIter == allDFGNode.end())
							nodeIter = allDFGNode.begin();
						else ++ nodeIter;
					} 
					else 
					{    // Place this node into RCA cell		

						 // Dynamic sort node list
						Vector<DFGVex*>::iterator tgtIter;

						for(tgtIter = curDFGNode->targets().begin();
							tgtIter != curDFGNode->targets().end(); ++ tgtIter)
						{

								DFGVex * tgtVex = *tgtIter;
								if(typeid(*tgtVex) != typeid(DFGNode)) continue;

								DFGNode * tgtNode = static_cast<DFGNode*>(tgtVex);
								assert(tgtNode != 0);

								// Find the target node in DFG node list.
								List<DFGNode*>::iterator iter = nodeIter;
								for(++iter; iter != endIter; ++ iter)
								{
									
									if(*iter == tgtNode)break;
								}
								
								assert(iter != endIter); // Must can be found

								List<DFGNode*>::iterator frontIter = iter;;
								
								for(-- frontIter; (*iter)->weight() == 
									(*frontIter)->weight(); iter = frontIter -- )
								{ // Swap these two nodes

										DFGNode * swap = *iter;
										*iter = *frontIter;
										*frontIter = swap;
								}

						}

						// Put DFG node into RCA
						assert(curRCANode.dfgNode() == 0);

						curRCANode.setDFGNode(curDFGNode);
						curDFGNode->setPlaced(true);
						outfile<<curDFGNode->seqNo()<<"\t";

						++ curColIndex[curRow];

						nodeIter = allDFGNode.erase(nodeIter);

						// Set bypass node
						//--------------------------------------
						//	Source node: A	Taret node: B, C
						//	Bypass node of A: A1, A2, A3
						//
						//	A  X  X  X  X
						//	A1 X  X  X  X
						//	A2 X  X  X  X
						//	A3 C  X  X  X
						//	B  X  X  X  X
						//--------------------------------------
						// In this pattern, lastBypsNode = A3, and
						// node B need attached to A3, while C to A2
						//

						for(srcIter = curDFGNode->sources().begin(); 
							srcIter != srcEnd; ++ srcIter){

							if(typeid(**srcIter) != typeid(DFGNode))continue;

							DFGNode * srcNode = static_cast<DFGNode*>(*srcIter);

							assert(srcNode != 0);

							DFGNode * firstBypsNode = config.getNodeByps(srcNode);

							if(firstBypsNode->rcaNode()->rca() == thisRCA) {

								DFGNode * lastBypsNode = 
									config.insertBypassBetween(srcNode, curDFGNode);

								int curBypNodeRow = curRow -1;
								
								while( lastBypsNode != firstBypsNode){

									assert(curBypNodeRow >= 0);

									RCANode & curRCABypNode =
										thisRCA->node(curBypNodeRow, curColIndex[curBypNodeRow] ++);

									assert(curRCABypNode.dfgNode() == 0);

									curRCABypNode.setDFGNode(lastBypsNode);
									lastBypsNode->setPlaced(true);

									lastBypsNode = static_cast<DFGNode*>(lastBypsNode->sources().front());

									-- curBypNodeRow;
								}
							}
						}

						break;
					}
				}

			}
			outfile<<"\n"<<std::endl;
		}

	}

	return 0;
}

