
#include "cl1.h"
#include "platf.h"
#include <cassert>

CL1RIM::CL1RIM(){

	for(int i =0; i < RIM_HEIGHT; ++ i){
		memoryRecord[i] = -1;
		freeOutSpace[i] = OUT_REGION_WIDTH; 
	}

}

CL1Data CL1RIM::allocate(int RCAIndex, int tempNum, int outNum, bool remapFlag, int RIMoutMode,int loop)
{
	assert(RCAIndex >= 0);
	if(tempNum == 0) tempNum = outNum;
	int x = tempNum/RIM_WIDTH_DATA;
	int y = (tempNum%RIM_WIDTH_DATA)?1:0;
	int bestHeight = x + y;
	//int bestHeight = tempNum/RIM_WIDTH_DATA + (tempNum%RIM_WIDTH_DATA)?1:0;
	for (int i = 0; i < RIM_HEIGHT; ++i)
	{
		if(memoryRecord[i] != -1 || freeOutSpace[i] != 8) continue;
		int bestBaseAddr = i;
		bool fit = true;
		for (int j = bestBaseAddr; j < bestBaseAddr + bestHeight;++j)
		{
			if (memoryRecord[i] != -1 || freeOutSpace[i] != 8) fit = false;
		}
		if (fit)
		{
			for (int k = bestBaseAddr; k < bestBaseAddr + bestHeight; ++k)
			{
				memoryRecord[k] = RCAIndex;
				freeOutSpace[k] = 0;
			}
			CL1Data data;
				
			data.setBaseAddress(bestBaseAddr);
			data.setHeight(bestHeight * loop);
			data.setLength((tempNum>=16)?16:tempNum);
			data.setOffset(0);

			return data;
		}
	}
	assert(0);
	CL1Data data_exception;
	return data_exception;
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

Vector<int> CL1RIM::free(int RCAIndex,int RIMOutMode){
	
	Vector<int> FreeLineInRIM;
	FreeLineInRIM.reserve(RIM_HEIGHT);
	
	int baseAddr;
	for(baseAddr =0; baseAddr <RIM_HEIGHT; ++baseAddr)
		if(memoryRecord[baseAddr] == RCAIndex)break;

	for(; baseAddr <RIM_HEIGHT; ++baseAddr){

		if(memoryRecord[baseAddr] == RCAIndex)
			{
			memoryRecord[baseAddr] = -1;
			//freeOutSpace[baseAddr] = -1;
			//2012.5.24 longlee 如果本RCA全行输出，freeOutSpace应该复位
			if(RIMOutMode == 0)	//内部节点全行输出,全行释放
				freeOutSpace[baseAddr] = 8;
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
		freeOutSpace[i] = RIM.freeOutSpace[i]; 
	}
}
