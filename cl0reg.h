// Filename: cl0reg.h
//--------------------------------------------------------------
// Author: Xie Li@ MicroE. SJTU  Time: 2010/08/01
//
// Define the registor operation of CL0, Based on RPU SPEC v1.3
//===============================================================
// Reference: RPU SPEC v1.3.doc, Configuration1.8.doc
//
// 2:Revised Version
// Revised by Chongyong Yin on 2010/9/25
// Reference: RPU SPEC v1.9.pdf,

#ifndef CL0REG_H
#define CL0REG_H

#include "reg.h"
#include <cassert>

/*#################################################################
-----------引自SPECv1.9 包头 格式说明

[1:0]	2bits	8x8 RCA 索引
				00: 左上8x8 RCA
				01: 右上8x8 RCA
				10: 左下8x8 RCA
				11: 右下8x8 RCA

[5:2]   4bits	REDL配置信息个数(0~8)

[10:7]	4bits	REDS配置信息个数(0~8)

[12]	1bit	表示有无立即数
				0: 无立即数
				1: 有2个32bit立即数，分别为选用立即数和必用立即数

[16:13]	4bits	配置包源索引，也是中断响应源索引
				0x0: 主控ARM
				0x1~0x8: μP1~μP8
				0x9~0xF: 保留

[6]				保留
[11]			保留
*/

/*##################################################################
------引自SPECv1.9 Configuration Word（CW）格式说明

[12:0]   13bits  表示context group在GCGM中的首地址

[15:13]  3bits   表示 8x8 RCA的同步模式
                 000：单个8x8 RCA执行完当前的CW后，立即执行对应的CW FIFO中的下一个CW(如果有的话)；
				 001：单个8x8 RCA执行完当前的CW后，等待另外3个8x8 RCA也执行完同步模式为001的CW，再执行对应的CW FIFO中的下一个(如果有的话)；
				 010：单个8x8 RCA执行完当前的CW后，等待另外3个8x8 RCA也执行完同步模式为010的CW后发出中断，再执行对应的CW FIFO中的下一个(如果有的话)；
				 011：单个8x8 RCA执行完当前的CW后，等待与之绑定的8x8 RCA也执行完同步模式为011的CW，再执行对应的CW FIFO中的下一个(如果有的话)；
				      左上8x8 RCA与右上8x8 RCA互相绑定，左下8x8 RCA与右下8x8 RCA互相绑定。
                 其它：保留。

[16]     1bit    偏移地址有效性选择
                 0：const1的偏移地址有效
				 1：const2的偏移地址有效

[22:17]  6bits   当[16]=0时，表示8x8 RCA中const1的偏移地址(0x00~0x3F)，与Core Context中的cons1基地址的和为const1的实际地址

[23:17]  7bits   当[16]=1时，表示8x8 RCA中const2的偏移地址0(0x00~0x7F)，当被Core Context选择时，与Core Context中的const2基地址的和为const2的实际地址(0x00~0xFF)

[30:24]  7bits   当[16]=1时，表示8x8 RCA中const2的偏移地址1(0x00~0x7F)，当被Core Context选择时，与Core Context中的const2基地址的和为const2的实际地址(0x00~0xFF)

[31:23]  当[16]=0时，保留

[31]     当[16]=1时，保留

*/

#define SYN_MODE_0	0
#define SYN_MODE_1	1
#define SYN_MODE_2	2
#define SYN_MODE_3	3

class CL0HeadReg: public Reg{

public:

	int groupAddress() const;                     //Context Group 在GCGM中的首地址（0x00~0x7FC） [12:0] 

	void setGroupAdrress(int addr);               //设置 Context Group 在GCGM中的首地址

	int synMode() const;                          //同步模式 [15:13]

	void setSynMode(int mode);

	int constSelect() const;                      //偏移地址有效性选择  0： const1的偏移地址有效   1:const2的偏移地址有效

	void setConstSelect(int sel);

	// set offset
	int constOffset1() const;                      //表示8x8 RCA中const1的偏移地址(0x00~0x3F),（与Core Context中的const1基地址的和为const1的实际地址） [22:17]

	void setConstOffset1(int offset);

	int constOffset20() const;                     //表示8x8 RCA中const2的偏移地址(0x00~0x7F), [23:17]

	void setConstOffset20(int offset);

	int constOffset21() const;                     //表示8x8 RCA中 const2 的偏移地址1（0x00~0x7F），当被core context选择时，与core context中的const2基地址的和构成const2的实际地址

	void setConstOffset21(int offset);
};

inline int CL0HeadReg::groupAddress() const {

	return static_cast<int>(regst & 0x00001fff);    //最后13位为CL1 Context Group 的首地址
}

inline void CL0HeadReg::setGroupAdrress(int addr){

	assert(0 <= addr && addr <= 0x1fff);

	regst &= 0xffffe000;                     //先把低13位清零
	regst |= static_cast<reg32>(addr);       //给低13位赋值
}

inline int CL0HeadReg::synMode() const{

	return static_cast<int>((regst & 0x0000e000) >> 13);  // 13~15位为同步模式
}

inline void CL0HeadReg::setSynMode(int mode) {

	assert(0 <= mode && mode <= 7);

	regst &= 0xffff1fff;                        //将第14~16位清零
	regst |= static_cast<reg32>(mode) << 13;    //给第14~16位赋值
}

inline int CL0HeadReg::constSelect() const{

	return static_cast<int>((regst & 0x00010000) >> 16);    //第17位为const memory偏移地址有效性选择
}

inline void CL0HeadReg::setConstSelect(int sel) {

	assert(0 <= sel && sel <= 1);

	//2012.5.7 longlee 之前代码居然有笔误！！！
	//regst &= 0x0000e000;
	regst &= 0xfffeffff;
	regst |= static_cast<reg32>(sel) << 16;
}

inline int CL0HeadReg::constOffset1() const {

	return static_cast<int>((regst & 0x007e0000) >> 17);
}

inline void CL0HeadReg::setConstOffset1(int offset){

	assert(0 <= offset && offset <= 0x3f);

	//修改 by liuxie
	//regst &= 0x00810000;
	//2012.5.7 longlee 之前代码有笔误
	regst &= 0xff81ffff;
	regst |= static_cast<reg32>(offset) << 17;
}

inline int CL0HeadReg::constOffset20() const {

	return static_cast<int>((regst & 0x00fe0000) >> 17);
}

inline void CL0HeadReg::setConstOffset20(int offset){

	assert(0 <= offset && offset <= 0x7f);

	regst &= 0xff01ffff;
	regst |= static_cast<reg32>(offset) << 17;
}

inline int CL0HeadReg::constOffset21() const {

	return static_cast<int>((regst & 0x7f000000) >> 24);
}

inline void CL0HeadReg::setConstOffset21(int offset){

	assert(0 <= offset && offset <= 0x7f);

	regst &= 0x80ffffff;
	regst |= static_cast<reg32>(offset) << 24;
}

/*###################### Configuration Word (CW)中断模式 ####################
// CW 配置信息 from RPU spec v1.9
[12:0]   保留
[15:13]  表示8x8 RCA的同步模式
         100：CW无效，仅同步模式有效，直接发出中断；
		 其它：保留
[31:16]  保留
*/

class CL0HeadRegInt: public Reg{

public:

	int synModeInt() const;

	void setSynModeInt(int mode);	
};

inline int CL0HeadRegInt::synModeInt() const{

	return static_cast<int>((regst & 0x0000e000) >> 13);
}

inline void CL0HeadRegInt::setSynModeInt(int mode){

	assert(0 <= mode && mode <= 7);

	regst &= 0xffff1fff;
	regst |= static_cast<reg32>(mode) << 13;
}


////////////////////////////////////////////////////////////

#define REDL_MODE_1L	0
#define REDL_MODE_2L	1
#define REDL_MODE_4L	2
#define REDL_MODE_8L	3

//---------------------------------------------------------
/*#########################################################
//REDL 配置信息 word1 from RPU Specv1.9

[0:23]	24bits	表示从数据源存储器读出的2D数据的首地址，（0x000000~0xFFFFFFFF）,可寻址16MByte 空间

[31:24] 保留

*/
class REDLReg0: public Reg{

public:

	int SSRAMAddress() const;

	void setSSRAMAddress(int addr);
};

inline int REDLReg0::SSRAMAddress() const {

	return static_cast<int>(regst & 0x00ffffff);
}

inline void REDLReg0::setSSRAMAddress(int addr) {

	assert(0 <= addr && addr <= 0xffffff);

	regst &= 0xff000000;
	regst |= static_cast<reg32>(addr);
}

/*#####################################################################
//REDL 配置信息word0 from Spec V1.9

[0:4]	 5bits  表示从数据源存储器读出的2D数据的每行长度(1~32)，长度单位为字节

[5:9]	 5bits  表示向数据目标存储器写入的2D数据的行数(1~32)

[10:20]	 11bits 表示向数据目标存储器写入的2D数据的行间跳转长度(0~2047)，长度单位为字节

[22:21]	 2bits  表示数据拼接模式
				00： 标识数据源中的每1行数据拼接为数据目标中的1行数据（不拼接）
				01： 标识数据源中的每2行数据拼接为数据目标中的1行数据（须保证拼接所得的1行数据不大于32Byte）
				10： 标识数据源中的每4行数据拼接为数据目标中的1行数据（须保证拼接所得的1行数据不大于32Byte）
				11： 标识数据源中的每8行数据拼接为数据目标中的1行数据（须保证拼接所得的1行数据不大于32Byte）

[23:31]   保留
*/

class REDLReg1: public Reg{

public:

	int SSRAMLength() const;

	void setSSRAMLength(int length);

	int SSRAMHeight() const;

	void setSSRAMHeight(int height);

	int SSRAMJump() const;

	void setSSRAMJump(int jump);

	int mode() const;

	void setMode(int mode);
};

inline int REDLReg1::SSRAMLength() const {

	return static_cast<int>(regst & 0x0000001f);
}

inline void REDLReg1::setSSRAMLength(int length){

	assert(0 <= length && length <= 0x1f);         //   0 <=length<= 31 

	regst &= 0xffffffe0;                          
	regst |= static_cast<reg32>(length);
}

inline int REDLReg1::SSRAMHeight() const {

	return static_cast<int>((regst & 0x000003e0) >> 5);
}

inline void REDLReg1::setSSRAMHeight(int height){

	assert(0 <= height && height <= 0x1f);

	regst &= 0xfffffc1f;
	regst |= static_cast<reg32>(height) << 5;
}

inline int REDLReg1::SSRAMJump() const {

	return static_cast<int>((regst & 0x001ffc00) >> 10);
}

inline void REDLReg1::setSSRAMJump(int jump){

	assert(0 <= jump && jump <= 0x7ff);

	regst &= 0xffe003ff;
	regst |= static_cast<reg32>(jump) << 10;
}

inline int REDLReg1::mode() const {

	return static_cast<int>((regst & 0x00600000) >> 21);
}

inline void REDLReg1::setMode(int mode){

	assert(0 <= mode && mode <= 3);

	regst &= 0xff9fffff;
	regst |= static_cast<reg32>(mode) << 21;
}

class REDLReg {

public:

	REDLReg0 & word0() { return REDLWord0; }

	REDLReg1 & word1() { return REDLWord1; }

private:

	REDLReg0 REDLWord0;
	REDLReg1 REDLWord1;
};

/////////////////////////////////////////////////////////
/*#######################################################
//REDS 配置信息 word(1) from SPEC v1.9

[0:23]	24bits	表示向数据目标存储器写入的2D数据的首地址（0x000000~0xFFFFFFFF）,可寻址16MByte空间
[24:31] 保留

*/

class REDSReg0: public Reg{

public:

	int SSRAMAddress() const;

	void setSSRAMAddress(int addr);
};

inline int REDSReg0::SSRAMAddress() const {

	return static_cast<int>(regst & 0x00ffffff);
}

inline void REDSReg0::setSSRAMAddress(int addr) {

	assert(0 <= addr && addr <= 0xffffff);

	regst &= 0xff000000;
	regst |= static_cast<reg32>(addr);
}

/*######################################################################
//REDS 配置信息 word0（1）from RPU Spec V1.9

[4:0]    5bits  表示从数据源存储器读出的2D数据的每行长度(1~32)，长度单位为字节

[9:5]    5bits  表示向数据目标存储器写入的2D数据的行数(1~32)

[20:10]  11bits 表示向数据目标存储器写入的2D数据的行间跳转长度(0~2047)，长度单位为字节

[21]     1bit   设为0，表示数据拼接模式传输

[25:22]  4bits  表示数据拼接的行数(1~16)

[31:26]  保留

*/



#define REDS_MODE_ML	0
#define REDS_MODE_DL	1

class REDSReg1: public Reg{

public:

	int SSRAMLength() const;

	void setSSRAMLength(int length);

	int SSRAMHeight() const;

	void setSSRAMHeight(int height);

	int SSRAMJump() const;

	void setSSRAMJump(int jump);

	int mode() const;

	void setMode(int mode);

	int FIFOLineNum() const;

	void setFIFOLineNum(int num);
};

inline int REDSReg1::SSRAMLength() const {

	return static_cast<int>(regst & 0x0000001f);
}

inline void REDSReg1::setSSRAMLength(int length){

	assert(0 <= length && length <= 0x1f);

	regst &= 0xffffffe0;
	regst |= static_cast<reg32>(length);
}

inline int REDSReg1::SSRAMHeight() const {

	return static_cast<int>((regst & 0x000003e0) >> 5);
}

inline void REDSReg1::setSSRAMHeight(int height){

	assert(0 <= height && height <= 0x1f);

	regst &= 0xfffffc1f;
	regst |= static_cast<reg32>(height) << 5;
}

inline int REDSReg1::SSRAMJump() const {

	return static_cast<int>((regst & 0x001ffc00) >> 10);
}

inline void REDSReg1::setSSRAMJump(int jump){

	assert(0 <= jump && jump <= 0x7ff);

	regst &= 0xffe003ff;
	regst |= static_cast<reg32>(jump) << 10;
}

inline int REDSReg1::mode() const {

	return static_cast<int>((regst & 0x00200000) >> 21);
}

inline void REDSReg1::setMode(int mode){

	assert(0 <= mode && mode <= 1);

	regst &= 0xffdfffff;
	regst |= static_cast<reg32>(mode) << 21;
}

inline int REDSReg1::FIFOLineNum() const{

	return static_cast<int>((regst & 0x03c00000) >> 22);
}

inline void REDSReg1::setFIFOLineNum(int num){

	assert(0 <= num && num <= 0xf);

	regst &= 0xfc3fffff;
	regst |= static_cast<reg32>(num) << 22;
}

//--------------------------------------------

class REDSReg {

public:

	REDSReg0 & word0() { return REDSWord0; }

	REDSReg1 & word1() { return REDSWord1; }

private:

	REDSReg0 REDSWord0;
	REDSReg1 REDSWord1;
};

#endif

