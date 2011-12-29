
#include "cl1.h"
#include <cassert>

CL1RIM::CL1RIM(){

	for(int i =0; i < RIM_HEIGHT; ++ i){
		memoryRecord[i] = -1;
		freeOutSpace[i] = OUT_REGION_WIDTH; 
	}

}

CL1Data CL1RIM::allocate(int RCAIndex, int tempNum, int outNum)
{
	assert(RCAIndex >= 0);

	/* Search the best consecutive free space */   //找到连续的空余空间
	int bestHeight = -1;
	int bestBaseAddr = -1;
	int bestMinOutLength = 0;
	//2011.5.11 liuxie
	//int minWastSpace =1024;     // MAX space of CL1RIM
	int minWastSpace = 512;       // MAX space of CL1RIM

	int minTempHeight = tempNum / TEMP_REGION_WIDTH 
		+ (tempNum% TEMP_REGION_WIDTH? 1: 0);

	if(minTempHeight == 0)minTempHeight = 1;

	//outport在RIM中的放置策略
	for(int height = minTempHeight; height <RIM_HEIGHT; ++ height)
	{
		const int outSize = outNum/height + (outNum%height? 1: 0);

		int baseAddr = 0;      //从SSRAM的首地址开始找是否有合适的地址暂存数据

		for(int i =0; i < RIM_HEIGHT; ++i)
		{
			/* find the appropriate baseAddr */
			if(memoryRecord[i] != -1 || freeOutSpace[i] < outSize )
			{  
				baseAddr = i + 1;         //不满足，换下一行
			} 

			/* based on one baseAddr locate enough momery block */
			//当某一行满足时候，则将i++,这样找到满足条件的连续行空间来存储这块数据
			if( i - baseAddr + 1 == height )
			{
				/* Find the line with minimal free space */
				int minOutLength = freeOutSpace[baseAddr];

				for(int j = 0; j <height; ++ j)
					if(freeOutSpace[baseAddr + j] < minOutLength)
						minOutLength = freeOutSpace[baseAddr + j];
				
				/* Calculate the waste space */
				int wastSpace = 0;

				for(int j =0; j <height; ++ j)
				{

					int curAddr = baseAddr + j;
					wastSpace += freeOutSpace[curAddr] - minOutLength;
				}

				const int wastThreshold = 8; // This variable used to make the data 
											 // more compact in horizontal direction.

				if(wastSpace + wastThreshold < minWastSpace)
				{
					bestBaseAddr = baseAddr;
					bestHeight = height;
					bestMinOutLength = minOutLength;

					minWastSpace = wastSpace;
				}

				++ baseAddr;
			}		
			
		}

	}

	if(bestBaseAddr != -1)    //当RIM中不为空时
	{ // Allocate CL1RIM space

		for(int j =0; j <bestHeight; ++ j)
		{
			
			int curAddr = bestBaseAddr + j;
			memoryRecord[curAddr] = RCAIndex;
			//freeOutSpace[curAddr] = bestMinOutLength - outNum/bestHeight - 1;
			freeOutSpace[curAddr] = bestMinOutLength - outNum/bestHeight;
			
		}
		
		for(int j =0; j <outNum % bestHeight; ++ j)
		{
			/* FIXME : it may should get revised */
			-- freeOutSpace[bestBaseAddr + j];
		}
	}


	/* Set Allocated space */
	CL1Data data;

	data.setBaseAddress(bestBaseAddr);
	data.setHeight(bestHeight);
	//2011.5.11 liuxie
	//data.setLength(TEMP_REGION_WIDTH + bestMinOutLength);
	data.setLength(TEMP_REGION_WIDTH + bestMinOutLength);
	data.setOffset(0);

	return data;
}


CL1Data CL1RIM::getRCACIDLData(int RCAIndex) const{

	assert(RCAIndex > 0);

	/* Find the base address */
	int baseAddr;
	for(baseAddr =0; baseAddr <RIM_HEIGHT; ++ baseAddr)
		if(memoryRecord[baseAddr] == RCAIndex)break;

	assert(baseAddr <RIM_HEIGHT);  // Must can be found!

	// Find the end address
	int endAddr;
	for(endAddr = baseAddr + 1; endAddr <RIM_HEIGHT; ++ endAddr)
		if(memoryRecord[endAddr] != RCAIndex)break;

	int height = endAddr - baseAddr;

	CL1Data CIDLData;

	CIDLData.setBaseAddress(baseAddr);
	CIDLData.setLength(TEMP_REGION_WIDTH);
	CIDLData.setWidth(TEMP_REGION_WIDTH);
	CIDLData.setHeight(height);

	return CIDLData;
}

int CL1RIM::getBaseAddress(int RCAIndex) const {

	for(int i =0; i <RIM_HEIGHT; ++ i)
		if(memoryRecord[i] == RCAIndex)return i;

	return -1;
}

int CL1RIM::getEndAddress(int RCAIndex) const  {

	for(int i =RIM_HEIGHT -1; i >=0; -- i)
		if(memoryRecord[i] == RCAIndex)return i+1;

	return -1;
}

int CL1RIM::getOutBaseAddress() const {

	for(int i =0; i <RIM_HEIGHT; ++ i)
		if(freeOutSpace[i] < OUT_REGION_WIDTH)return i;

	return -1;
}

int CL1RIM::getOutEndAddress() const {

	for(int i =RIM_HEIGHT -1; i >=0; -- i)
		if(freeOutSpace[i] < OUT_REGION_WIDTH)return i+1;

	return -1;
}

int CL1RIM::getOutOffset() const {

	int offset = OUT_REGION_WIDTH;
	for(int i = RIM_HEIGHT -1; i >=0; -- i)
		if( freeOutSpace[i] < offset) offset = freeOutSpace[i];

	return offset;
}

Vector<int> CL1RIM::free(int RCAIndex){
	
	Vector<int> FreeLineInRIM;
	FreeLineInRIM.reserve(RIM_HEIGHT);
	
	int baseAddr;
	for(baseAddr =0; baseAddr <RIM_HEIGHT; ++baseAddr)
		if(memoryRecord[baseAddr] == RCAIndex)break;

	for(; baseAddr <RIM_HEIGHT; ++baseAddr){

		if(memoryRecord[baseAddr] == RCAIndex)
			{
			memoryRecord[baseAddr] = -1;
			FreeLineInRIM.push_back(baseAddr);
			}
		else break;
	}

	return FreeLineInRIM;
}

void CL1RIM::clear() {

	for(int i =0; i < RIM_HEIGHT; ++ i){
		memoryRecord[i] = -1;
		freeOutSpace[i] = OUT_REGION_WIDTH; 
	}
}

void CL1RIM::copy(CL1RIM RIM) {
	
	for(int i =0; i < RIM_HEIGHT; ++ i){
		memoryRecord[i] = RIM.memoryRecord[i];
		freeOutSpace[i] = freeOutSpace[i]; 
	}
}
