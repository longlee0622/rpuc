/*
log:
	2011.4.20
	changed by liuxie
    SSRAM把数据传输到RIF中无操作，在数据从RIF到RC Array中需要进行选择操作
    （该方案暂时被注释掉，如有需要可以使用）
*/
#include "cl2reg.h"
#include "rpucfg.h"
#include "dfgraph.h"

extern bool IsInnerPort(DFGPort * port);
extern bool IsTempExternPort (DFGPort *port);
extern int totalRCA;

int RPUConfig::genCL2Context( Vector<RCA *> CL1RCA )
{
	
	//List<Ptr<RCA> >::iterator rcaIter = rcaList.begin();
	Vector<RCA *>::iterator rcaIter = CL1RCA.begin();
	for(; rcaIter != CL1RCA.end(); ++ rcaIter)
	{
		//RCA * thisRCA = rcaIter->get();
		RCA * thisRCA = *rcaIter;
		CL2Reg CL2Contex;
		
		//2011.5.11 liuxie
		/* RCA Array control information */
        //InputAGrain == 0  8 bit
		//            == 1  16bit
		//InputBGrain == 0  8 bit
		//            == 1  16bit
		//OutputGrain == 0  8 bit
		//            == 1  16bit
		//InputTempGrain == 0 8 bit
		//               == 1 16bit
		//InputGrainMode == 0 输入粒度由N,P,Q配置
		//               == 1 输入数据都为4 bit 

	    /*int inputTimes = 0, outputTimes = 0, outputBeginCycle = 0, outputBegin = 0
	    , LoopInterval = 0, ConstOut0Flag = 0, LoopTimes = 0
	    , ConstOut1Flag = 0, OutputGrain = 0, RIFSign = 0, InputAGrain = 0
	    , InputBGrain = 0, InputTempGrain = 0, InputGrainMode = 0;*/	
        
        int inputTimes = 0, outputTimes = 0, outputBeginCycle = 0, outputBegin = 0
	    , LoopInterval = 0, ConstOut0Flag = 0, LoopTimes = 0
	    , ConstOut1Flag = 0, OutputGrain = 0, RIFSign = 0, InputAGrain = 1
	    , InputBGrain = 1, InputTempGrain = 1, InputGrainMode = 0;
		// Set RC and Temp inter-connection context
		/////////////////////////////////////////////////////////////

		for(int i =0; i <RC_REG_NUM; ++ i) 
		{

			RCANode & curRCANode = thisRCA->node(i);
			RCANode & curRCATemp = thisRCA->temp(i);

			DFGNode * thisNode = curRCANode.dfgNode();
			DFGNode * thisTemp = curRCATemp.dfgNode();

			// Set RC context
			//==================================================
			

			if(thisNode == 0)
			{ // This RC not used

				CL2Contex.setInputA(i, 0, SRC_TYPE_FIFO);
				CL2Contex.setInputB(i, 0, SRC_TYPE_FIFO);
				CL2Contex.setRCOutEnable(i, false);
			} 
			else
			{

				// (1) Set A, B input
				//--------------------------------------------

				DFGVex * srcA = 0;
				DFGVex * srcB = 0;
				
				switch(thisNode->sourceSize())
				{
					case 2: srcB = thisNode->sources(1);
					case 1: srcA = thisNode->sources(0);
							break;
					default: assert(0);
				}
				if ((thisNode->sourceSize() == 1) && (thisNode->opecode() == ADD))
					thisNode->setOpecode(BYP);

				assert(srcA != 0);

				// (i) Set inputA

				bool isNode = (typeid(*srcA) == typeid(DFGNode));

				if(isNode) 
				{
					RCANode * srcRCANode = static_cast<DFGNode*>(srcA)->rcaNode();
			
					// Input -- RC: 0-7; Temp 8-15;
					CL2Contex.setInputA(i, srcRCANode->col(), SRC_TYPE_LAST);
				} 
				else 
				{ // It's a port
					int inputA;

					//2011.4.20 chenged by liuxie
					//因为这里改变了DFG图输入数据的顺序
					Vector<RCAPort> & rcaInport = thisRCA->inports(); 

					int inSize = static_cast<int>(rcaInport.size());
					for(inputA =0; inputA <inSize; ++ inputA)
					{

						if(rcaInport[inputA].dfgPort() == srcA)
							break;
					}

					assert(inputA != static_cast<int>(rcaInport.size())); // Must can be found

					//2011.4.27 changed by liuxie
					////////////////////////start///////////////////////////////////////////////
					int DFGportASeqNum = rcaInport[inputA].dfgPort()->SSRAMAddress() - DFGInBaseAddr;

					//int PortAinRIFNumber = DFGportASeqNum/32+((DFGportASeqNum%32)?1:0); 

                   
					//RCAPort & srcAPort = rcaInport[DFGportSeqNum];
             
					RCAPort & srcAPort = rcaInport[inputA];

					
					/*CL2Contex.setInputA(
						i, 
						srcAPort.RIFRow() * FIFO_WIDTH 
							+ srcAPort.RIFCol(), 
						SRC_TYPE_FIFO
					);*/
                    
					//2011.5.27  liuxie 
					if(DFGportASeqNum>=0 && DFGportASeqNum != 0xffffffff && (!(IsTempExternPort(rcaInport[inputA].dfgPort()))) && (DFGportASeqNum< 192))//表明该port是一个来自外部SSRAM的port，即DFG的输入port
					{	
						CL2Contex.setInputA(
						i, 
						DFGportASeqNum/2,  //当是out port时候，则要减去它的起始地址 0x800(2048) 
						SRC_TYPE_FIFO
						);
					}
					else if(IsTempExternPort(rcaInport[inputA].dfgPort()) && !(rcaInport[inputA].IsInSameGroup()))
					{
						
						int Index = rcaInport[inputA].RIFRow() * FIFO_WIDTH_DATA + rcaInport[inputA].RIFCol();
						CL2Contex.setInputA(
						i, 
						Index,  //当是TempExternPort时候，则要减去它的起始地址0x300（768）
						SRC_TYPE_FIFO
						);
					}
					else
					{
						//2011.6.9 liuxie
						CL2Contex.setInputA(i, srcAPort.RIFRow() * FIFO_WIDTH_DATA +srcAPort.RIFCol(), SRC_TYPE_FIFO);
					}

                    /////////////////////////end///////////////////////////////////////
					
					/* Get the max RIFRow for ControlReg*/
					inputTimes = (inputTimes>(( (CL2Contex.inputA(i)) & 0xe0 )>>5) 
								  ? inputTimes
								  : (( (CL2Contex.inputA(i)) & 0xe0 )>>5) );
					LoopInterval = i/8;
				}

				//(ii) Set inputB

				if(srcB != 0)
				{ // It must be a bypass node

					bool isNode = (typeid(*srcB) == typeid(DFGNode));

					if(isNode) 
					{

						RCANode * srcRCANode = static_cast<DFGNode*>(srcB)->rcaNode();

						// Input -- RC: 0-7; Temp 8-15;
						CL2Contex.setInputB(i, srcRCANode->col(), SRC_TYPE_LAST);
					} 
					else 
					{ // It's a port
						int inputB;

						Vector<RCAPort> & rcaInport = thisRCA->inports();
						int inSize = static_cast<int>(rcaInport.size());
						for(inputB =0; inputB <inSize; ++ inputB)
						{

							if(rcaInport[inputB].dfgPort() == srcB)break;
						}

						assert(inputB != static_cast<int>(rcaInport.size())); // Must can be found

						RCAPort & srcBPort = rcaInport[inputB];
                        
						//2011.4.27 changed by liuxie
						///////////////////////////start/////////////////////////////////////////////
						int DFGportBSeqNum = rcaInport[inputB].dfgPort()->SSRAMAddress() - DFGInBaseAddr;
					    //int PortBinRIFNumber = DFGportBSeqNum/32+((DFGportBSeqNum%32)?1:0);
						//int PortBinRIFRow = DFGportBSeqNum/FIFO_WIDTH+((DFGportBSeqNum%FIFO_WIDTH)?1:0);
						//int PortBinRIFCol = DFGportBSeqNum%FIFO_WIDTH;

						/*CL2Contex.setInputB(
							i, 
							srcBPort.RIFRow() * FIFO_WIDTH 
								+ srcBPort.RIFCol(), 
							SRC_TYPE_FIFO
						);*/
                        
						if(DFGportBSeqNum>=0 && DFGportBSeqNum!= 0xffffffff && (!(IsTempExternPort(rcaInport[inputB].dfgPort()))) && (DFGportBSeqNum< 192))
						{
							CL2Contex.setInputB(
								i, 
								DFGportBSeqNum/2,  //当是out port时候，则要减去它的的起始地址0x800(2048)
								SRC_TYPE_FIFO
							);
						}
						else if(IsTempExternPort(rcaInport[inputB].dfgPort()) && !(rcaInport[inputB].IsInSameGroup()))
						{
							
							int Index = rcaInport[inputB].RIFRow() * FIFO_WIDTH_DATA + rcaInport[inputB].RIFCol();
							CL2Contex.setInputB(
							i, 
							Index,  //当是TempExternPort时候，则要减去它的起始地址0x300（768）
							SRC_TYPE_FIFO
							);
						}
						else
						{
							//2011.6.9 liuxie
							CL2Contex.setInputB(
							i, 
							srcBPort.RIFRow() * FIFO_WIDTH_DATA 
								+ srcBPort.RIFCol(), 
							SRC_TYPE_FIFO
						);
						}

                      //////////////////////////end////////////////////////////////////////////////////
					
					/* Get the max RIFRow */
					inputTimes = (inputTimes > (( (CL2Contex.inputB(i)) & 0xe0 )>>5)
								  ? inputTimes
								  :  (((CL2Contex.inputB(i)) & 0xe0 )>>5) );
					LoopInterval = i/8;
		
					}
				}

				// (2) Set RC output
				//-----------------------------------------------------------------

				DFGPort * nodeOutport = 0;

				for(int t =0; t <thisNode->targetSize(); ++t)
				{

					DFGVex * thisTgt = thisNode->targets(t);

					bool isNode = (typeid(*thisTgt) == typeid(DFGNode) );

					if(!isNode)
					{

						nodeOutport = static_cast<DFGPort*>(thisTgt);
						break;
					}
				}

				if(nodeOutport != 0)
				{

					int outIndex = 0;
					const Vector<RCAPort> & rcaOutput = thisRCA->outports();
					Vector<RCAPort>::const_iterator outIter = rcaOutput.begin();

					for(; outIter != rcaOutput.end(); ++ outIter, ++outIndex)
					{

						if(outIter->dfgPort() == nodeOutport)break;
					}

					assert(outIter != rcaOutput.end()); // Must can be found

					RCAPort tgtRCAPort = rcaOutput[outIndex];

					//2011.6.9 liuxie
					CL2Contex.setRCOut(i, 
						tgtRCAPort.ROFRow() * FIFO_WIDTH_DATA + tgtRCAPort.ROFCol()
					);

					CL2Contex.setRCOutEnable(i, true);
					outputTimes = (outputTimes > (tgtRCAPort.ROFRow())
											? outputTimes
											: (tgtRCAPort.ROFRow()) ) ;
					if (outputBegin == 0)
					{
						outputBeginCycle = i/8;
						outputBegin = 1;
					}
				}

				// (3) Set opecode
				//-----------------------------------------------------------------

				CL2Contex.setRCOpecode(i, thisNode->opecode());
			} // End set RC context

			// Set Temp context
			//=====================================================================

			if(thisTemp == 0)
			{

				CL2Contex.setInputTemp(i, 0, SRC_TYPE_FIFO);
				CL2Contex.setTempOutEnable(i, false);

			} 
			else 
			{
				// (1) Set temp input

				assert(thisTemp->sources().size() == 1); // It must be a bypass
				DFGVex * srcTemp = thisTemp->sources(0);


				bool isNode = (typeid(*srcTemp) == typeid(DFGNode));

				if(isNode) 
				{

					RCANode * srcRCANode = static_cast<DFGNode*>(srcTemp)->rcaNode();

					// Input -- RC: 0-7; Temp 8-15;
					CL2Contex.setInputTemp(i, srcRCANode->col(), SRC_TYPE_LAST);
				} 
				else 
				{ // It's a port
					int inputTemp;

					Vector<RCAPort> & rcaInport = thisRCA->inports();

					int inSize = static_cast<int>(rcaInport.size());

					for(inputTemp =0; inputTemp <inSize; ++ inputTemp)
					{
						if(rcaInport[inputTemp].dfgPort() == srcTemp)break;
					}

					assert(inputTemp != static_cast<int>(rcaInport.size())); // Must can be foundi

					RCAPort & srcTempPort = rcaInport[inputTemp];

					//2011.6.9 liuxie
					CL2Contex.setInputTemp(
						i, 
						srcTempPort.RIFRow() * FIFO_WIDTH_DATA 
							+ srcTempPort.RIFCol(), 
						SRC_TYPE_FIFO
					);
				
					/* Get the max RIFRow*/
					inputTimes = (inputTimes>(( (CL2Contex.inputTemp(i)) & 0xe0 )>>5)
							 ? inputTimes
							 : (( (CL2Contex.inputTemp(i)) & 0xe0 )>>5) );
				
					LoopInterval = i/8;
					if (outputBegin == 0)
					{
						outputBeginCycle = i/8;
						outputBegin = 1;
					}
				}

				// (2) Set Temp output
				//-----------------------------------------------------------------

				DFGPort * tempOutport = 0;

				for(int t =0; t <thisTemp->targetSize(); ++t){

					DFGVex * thisTgt = thisTemp->targets(t);

					bool isNode = (typeid(*thisTgt) == typeid(DFGNode) );

					if(!isNode){

						tempOutport = static_cast<DFGPort*>(thisTgt);
						break;
					}
				}

				if(tempOutport != 0){

					int outIndex = 0;
					const Vector<RCAPort> rcaOutput = thisRCA->outports();
					Vector<RCAPort>::const_iterator outIter = rcaOutput.begin();

					for(; outIter != rcaOutput.end(); ++ outIter, ++outIndex){

						if(outIter->dfgPort() == tempOutport)break;
					}

					assert(outIter != rcaOutput.end()); // Must can be found

					RCAPort tgtRCAPort = rcaOutput[outIndex];

					CL2Contex.setTempOut(i, 
						tgtRCAPort.ROFRow() * FIFO_WIDTH_DATA + tgtRCAPort.ROFCol()
					);

					CL2Contex.setTempOutEnable(i, true);

					outputTimes = (outputTimes > (tgtRCAPort.ROFRow())
											? outputTimes
											: (tgtRCAPort.ROFRow()) );
					if (outputBegin == 0)
						{
						outputBeginCycle = i/8;
						outputBegin = 1;
						}
				}
		
			} // End set Temp context

		}

		
		
		//2011.5.31  liuxie 
		int MinStep = 1000;
		int MaxStep = 0;
		RCANode nodetemp; 
		int outputBeginCycleTemp = 0;
		int nodenum = 0;
		for(; nodenum < RC_REG_NUM ; nodenum++)
		{
			nodetemp = thisRCA->node(nodenum);
			if(nodetemp.dfgNode() == 0)
				continue;
			if(nodetemp.dfgNode()->step() == -1)
				continue;
			if( nodetemp.dfgNode()->step() < MinStep)
				MinStep = nodetemp.dfgNode()->step();
			if( MaxStep <  nodetemp.dfgNode()->step())
				MaxStep = nodetemp.dfgNode()->step();
		}

        outputBeginCycleTemp = MaxStep - MinStep;
		
		if(outputBeginCycle <= 0 ) 
			outputBeginCycle = 1;

		/*for(nodenum = 0; nodenum < RC_REG_NUM; nodenum++)
		{
			nodetemp = thisRCA->temp(nodenum);
			if(nodetemp.dfgNode() == 0)
				continue;
			if(nodetemp.dfgNode()->step() == -1)
				continue;
			if( nodetemp.dfgNode()->step() < MinStep2)
				MinStep2 = nodetemp.dfgNode()->step();
			if( MaxStep2 <  nodetemp.dfgNode()->step())
				MaxStep2 = nodetemp.dfgNode()->step();
		}
		outputBeginCyclesTemp2 = MaxStep2 - MinStep2;

		if( outputBeginCyclesTemp1 >= outputBeginCyclesTemp2)
			outputBeginCycle = outputBeginCyclesTemp1;
		else
			outputBeginCycle = outputBeginCyclesTemp2;

		*/




		// Set control context
		///////////////////////////////////////////////////////////////////////

		/* Generate the controlReg */
		{
			/* set input times ;
			   input_times = total rows of input data in RIF during a cycle ; */
			
			//2011.6.13 liuxie
			////////////////////////////////start/////////////////////////////////////////////
			//if (thisRCA->getPseudoRCAFlag())
			//	CL2Contex.CL2ControlReg.setInputTimes((thisRCA->getPseudoRCAMode()?3:7));
			//else
			int temp1=0;   
			int temp2=0;   			
			int tempRow1=0;   //for extern port in
			int tempRow2=0;   //for extern temp port in
			int tempRow3=0;   //for inner port in
			int tempRow=0;
			int tempRIFBaseRow = 8;
			int tempRIFTopRow  = 0;

			if( thisRCA->rcaSSRAMInTopAddr() >= 0 && thisRCA->rcaSSRAMInBaseAddr() >= 0)
			{
				if(thisRCA->rcaSSRAMInTopAddr() >= thisRCA->rcaSSRAMInBaseAddr())
				{
					temp1 = thisRCA->rcaSSRAMInTopAddr() - DFGInBaseAddr;
					temp1 = temp1 / 2;
					tempRow1 = temp1/FIFO_WIDTH_DATA + ((temp1 % FIFO_WIDTH_DATA)?1:0);
				}
			}
			if(thisRCA->getSSRAMTempInTopAddr() >=0 && thisRCA->getSSRAMTempInBaseAddr() >= 0)
			{
				if(thisRCA->getSSRAMTempInTopAddr()>= thisRCA->getSSRAMTempInBaseAddr())
				{
					temp2 = thisRCA->getSSRAMTempInTopAddr() - thisRCA->getSSRAMTempInBaseAddr();
					temp2 = temp2 / 2;
					tempRow2 = temp2/FIFO_WIDTH_DATA;
					//tempRow2 = temp2/FIFO_WIDTH_DATA + ((temp2 % FIFO_WIDTH_DATA)?1:0);
				}
			}

			Vector<RCAPort> & rcaInport = thisRCA->inports();
			Vector<RCAPort>::iterator portIter;

			for(portIter = rcaInport.begin();portIter != rcaInport.end(); ++ portIter) 
			{
				if( IsInnerPort( portIter->dfgPort() ) || ((IsTempExternPort(portIter->dfgPort())) && portIter->IsInSameGroup()))
				{
					if( tempRIFBaseRow > portIter->RIFRow())
						tempRIFBaseRow = portIter->RIFRow();
					if( tempRIFTopRow < portIter->RIFRow())
						tempRIFTopRow = portIter->RIFRow();
				}
			}
			if(tempRIFTopRow >= tempRIFBaseRow)
				tempRow3 = tempRIFTopRow - tempRIFBaseRow + 1;

			if(thisRCA->getPseudoRCAFlag() != 1)
				tempRow= tempRow1+tempRow2+tempRow3 - 1;
			else
				tempRow = 7;
			////////////////////////////////end/////////////////////////////////////////////////////
			
			//CL2Contex.CL2ControlReg.setInputTimes(inputTimes);
			CL2Contex.CL2ControlReg.setInputTimes(tempRow);
			
			/* set loop interval ;
			   loopinterval = max level of input data in DFG ; */
  		    CL2Contex.CL2ControlReg.setLoopInterval(LoopInterval);
			
			/* set output times ;
			   output_times =  total rows of output data during a cycle ; */
            CL2Contex.CL2ControlReg.setOutputTimes(outputTimes);
			
			/* set output begin time ;
			   output_begin = level of first output data in DFG */
			CL2Contex.CL2ControlReg.setOutputCycle(outputBeginCycle);
			
			/* set the loop times */
		    //LoopTimes = thisRCA.loopNum();
			CL2Contex.CL2ControlReg.setLoop(LoopTimes);
			
			/* set data grain */
			CL2Contex.CL2ControlReg.setInputAGran(InputAGrain);
			CL2Contex.CL2ControlReg.setInputBGran(InputBGrain);
			CL2Contex.CL2ControlReg.setInputTempGran(InputTempGrain);
			CL2Contex.CL2ControlReg.setInputGranMode(InputGrainMode);
			
			//2011.5.11 changed by liuxie
			//OutputGrain = 0x00;     	// default : 8bit 
										//            运算        输出
			                            //  00:        8           8
			                            //  01:        8           16
			                            //  10:        16          8
			                            //  11:        16          16
			OutputGrain = 3;

			CL2Contex.CL2ControlReg.setOutputGran(OutputGrain);
			
			/* set RIMSignMode */
			CL2Contex.CL2ControlReg.setRIFSignMode(RIFSign);

			/* set constgroup controlling information */
			/* FIXME: Constant Group is still unused */
			//CL2Contex.CL2ControlReg.setOutCG0Flag();  
			//CL2Contex.CL2ControlReg.setOutCG1Flag();
		}
		///////////////////////////////////////////////////////////////////////

		Vector<reg32> context;
		context.reserve(CL2_CONTEXT_LEN );

		//20110719 liuxie begin
		//pushback the rcaindex at the beginning of each CL2 context for creatCL2.cpp
		context.push_back(thisRCA->seqNo() + totalRCA);
		//end

		for(int i =0; i <IN_OP_REG_SUM; ++i)
			context.push_back(CL2Contex.inputReg(i).reg());

		for(int i =0; i <OUT_REG_SUM; ++i)
			context.push_back(CL2Contex.outputReg(i).reg());

		for(int i =0; i <OUT_EN_SUM; ++i)
			context.push_back(CL2Contex.outEnableReg(i).reg());

		context.push_back(CL2Contex.controlReg().reg());

		for(int i =0; i <IN_HBIT_SUM; ++i)
			context.push_back(CL2Contex.inputHBitReg(i).reg());

		CL2Context.push_back(context);
	}

	return 0;
}

