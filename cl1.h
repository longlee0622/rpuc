
#ifndef CL1_H
#define CL1_H

#include "cl1reg.h"
//#include <iosfwd>

class CL1Data{

public:

	CL1Data(): dataBaseAddr(0), dataLength(0), 
		dataWidth(0), dataHeight(0), dataOffset(0) {}

	int source() const { return dataSource; }

	void setSource(int src) { dataSource = src; }

	int target() const { return dataTarget; }

	void setTarget(int tgt) { dataTarget = tgt; }

	int inputMode() const { return dataInputMode; }

	void setOutputMode(int mode) { dataOutputMode = mode; }

	int outputMode() const { return dataOutputMode; }

	void setInputMode(int mode) { dataInputMode = mode; }

	int baseAddress() const { return dataBaseAddr; }

	void setBaseAddress(int addr) { dataBaseAddr = addr; }

	int length() const { return dataLength; }

	void setLength( int length) { dataLength = length; }

	int width() const { return dataWidth; }

	void setWidth( int width) { dataWidth = width; }

	int height() const { return dataHeight; }

	void setHeight( int height ) { dataHeight = height; }

	int offset() const { return dataOffset; }

	void setOffset(int offset) { dataOffset = offset; }

private:
	
	int dataSource;
	int dataTarget;
	int dataInputMode;
	int dataOutputMode;
	int dataBaseAddr;
	int dataLength;
	int dataWidth;
	int dataHeight;
	int dataOffset;
	
};

///////////////////////////////////////////////////////////////

class CL1Block{

public:

	CL1Block();

	const CL1Data & RIDLData() const { return blockRIDLData; }

	CL1Data & RIDLData() { return blockRIDLData; }

	void setRIDLData( const CL1Data & data ) { blockRIDLData = data; }

	const CL1Data & CEDLData() const { return blockCEDLData; }

	CL1Data & CEDLData() { return blockCEDLData; }

	void setCEDLData( const CL1Data & data ) { blockCEDLData = data; }

	const CL1Data & CIDLData() const { return blockCIDLData; }

	CL1Data & CIDLData() { return blockCIDLData; }

	void setCIDLData( const CL1Data & data ) { blockCIDLData = data; }

	const CL1Data & CDSData() const { return blockCDSData; }

	CL1Data & CDSData() { return blockCDSData; }

	void setCDSData( const CL1Data & data ) { blockCDSData = data; }

	int RCAIndex() const { return blockRCAIndex; }

	void setRCAIndex( int index) { blockRCAIndex = index; }

	int RCACoreLoop() const { return blockRCACoreLoop; }

	void setRCACoreLoop( int loop) { blockRCACoreLoop = loop; }

	int const1Address () const { return blockConst1Addr; }

	void setConst1Address ( int addr) { blockConst1Addr = addr; }

	int const2Address () const { return blockConst2Addr; }

	void setConst2Address ( int addr) { blockConst2Addr = addr; }

	bool RIDLEnable() const { return blockRIDLEnable; }

	void setRIDLEnable( bool enable) { blockRIDLEnable = enable; }

	bool CEDLEnable() const { return blockCEDLEnable; }

	void setCEDLEnable( bool enable) { blockCEDLEnable = enable; }

	bool CIDLEnable() const { return blockCIDLEnable; }

	void setCIDLEnable( bool enable) { blockCIDLEnable = enable; }
	void setRemapFlag( bool enable) { remapFlag = enable; }

	bool getRemapFlag() const {return remapFlag;}
	void setTMFlag( bool enable) { TMFlag = enable; }
	bool getTMFlag() const {return TMFlag;}
	int BlockBefore() const {return RPUBlockBefore;}

	void setBlockBefore(int num){RPUBlockBefore = num;}

private:

	CL1Data blockRIDLData;
	CL1Data blockCEDLData, blockCIDLData, blockCDSData;

	int blockRCAIndex, blockRCACoreLoop;
	int blockConst1Addr, blockConst2Addr;

	bool blockRIDLEnable, blockCEDLEnable, blockCIDLEnable;
	bool remapFlag;
	bool TMFlag;
	int RPUBlockBefore;
};

inline CL1Block::CL1Block():
		blockRIDLData(), 
		blockCEDLData(), 
		blockCIDLData(), 
		blockCDSData(),
		blockRCAIndex(0), 
		blockRCACoreLoop(0),
		blockConst1Addr(0),
		blockConst2Addr(0),
		remapFlag(false),
		TMFlag(false),
		blockRIDLEnable(false), 
		blockCEDLEnable(false), 
		blockCIDLEnable(false) {}

/////////////////////////////////////////////////////////////

// RIM:
//===========================================
//	Temp Region: 8 bytes per line;
//  Out Region: 24 bytes per line;
//
//	1: * * * * + + + + + + + + + + + +
//	2: * * * * + + + + + + + + + + + +
//	3: * * * * + + + + + + + + + + + +
//	4: * T * * + + + + + + + + + + + +
//	5: * E * * + + + O U T + + + + + +
//	6: * M * * + + R E G I O N + + + +
//	7: * P * * + + + + + + + + + + + +
//	8: * * * * + + + + + + + + + + + +
//	9: * R * * + + + + + + + + + + + +
// 10: * E * * + + + + + + + + + + + +
// 11: * G * * + + + + + + + + + + + +
// 12: * I * * + + + + + + + + + + + +
// 13: * O * * + + + + + + + + + + + +
// 14: * N * * + + + + + + + + + + + +
// 15: * * * * + + + + + + + + + + + +
// 16: * * * * + + + + + + + + + + + +
//
//============================================

#define RIM_WIDTH	32 // Bytes
#define RIM_HEIGHT	32

//20110510 liuxie for expand from 8bit to 16bit
//#define TEMP_REGION_WIDTH	8
//#define OUT_REGION_WIDTH	24	// CIDL need to choose 
                                // input mode MODE_IN_D4

#define TEMP_REGION_WIDTH	8  // 2bytes a data
#define OUT_REGION_WIDTH	8  // 2bytes a data

#define TEMP_REGION_WIDTH_BYTE 16
#define OUT_REGION_WIDTH_BYTE  16

#define ROF_WIDTH_DATA  16
#define ROF_WIDTH       32

class CL1RIM{

public:

	CL1RIM();

	CL1Data allocate(int RCAIndex, int tempNum, int outNum);

	CL1Data getRCACIDLData(int RCAIndex) const;

	int getBaseAddress(int RCAIndex) const;

	int getEndAddress(int RCAIndex) const;

	int getOutBaseAddress() const;

	int getOutEndAddress() const;

	int getOutOffset() const;

	int getFreeOutSpace(int addr) const

	{ return freeOutSpace[addr]; }

	Vector<int> free(int RCAIndex);//void free(int RCAIndex);

	void clear();
	void copy(CL1RIM RIM);

private:

	int memoryRecord[RIM_HEIGHT];   //Record RCAIndex of each row of RIM
	int freeOutSpace[RIM_HEIGHT];   //available Bytes in each row of RIM_OUT_SPACE
};


#endif

