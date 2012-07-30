// Filename: cl1reg.h
//
//
// 1: Original Version
// --------------------------------------------------------------
// Author: Xie Li@ MicroE. SJTU  Time: 2010/07/10
//
// Define the registor operation of CL1, Based on RPU SPEC v1.3
// ==============================================================
// Reference: RPU SPEC v1.3.doc, Configuration1.8.doc
//
//
// 2:Revised Version
// Revised by Chongyong Yin on 2010/9/25
// Reference: RPU SPEC v1.9.pdf,

#ifndef CL1REG_H
#define CL1REG_H

#include "reg.h"
#include <cassert>
#include <iostream>



/*/############################################################################
//Context Group Head ������Ϣ from RPU SPECv1.9

[2:0]    3bits  ��ʾRIDL������Ϣ����(x = 1~8)

[3]      1bit   RIDLʹ��λ��1��Ч

[7:4]    4bits  ��ʾRCA������Ϣ����(y = 1~16)

[14:8]   7bits  ��ʾcontext group�ܳ���(5~73)

[31:15]  ����

*/

class GroupHeadReg : public Reg{

public:

	int RIDLSum() const;

	void setRIDLSum(int sum);

	bool RIDLEnable() const;

	void setRIDLEnable(bool enable);

	int RCASum() const;

	void setRCASum(int sum);

	int groupLength() const;

private:

	void updateGroupLength();
};

inline int GroupHeadReg::RIDLSum() const {

	return static_cast<int>( (regst & 0x00000007) + 1 );

}

inline void GroupHeadReg::setRIDLSum(int sum) {

	assert(1 <= sum && sum <= 8);

	regst &= 0xfffffff8;
	regst |= static_cast<reg32>(sum-1);

	updateGroupLength();
}

inline bool GroupHeadReg::RIDLEnable() const {

	return (regst & 0x00000008) ? true : false;

}

inline void GroupHeadReg::setRIDLEnable(bool enable) {

	regst &= 0xfffffff7;
	regst |= (enable)? 0x00000008:0x00000000;

	updateGroupLength();
}

inline int GroupHeadReg::RCASum() const {

	return static_cast<int>( ((regst & 0x000000f0) >> 4) + 1 );

}

inline void GroupHeadReg::setRCASum(int sum) {

	assert(1 <= sum && sum <= 16);

	regst &= 0xffffff0f;
	regst |= static_cast<reg32>(sum-1) << 4;

	updateGroupLength();
}


inline int GroupHeadReg::groupLength() const {

	return static_cast<int>( (regst & 0x00007f00) >> 8 ) + 1;
}

inline void GroupHeadReg::updateGroupLength() {

	int gLength = 1 + 4 * RCASum();
	if( RIDLEnable() ) gLength += RIDLSum();

	regst &= 0xffff80ff;
	regst |= static_cast<reg32>(gLength) << 8;
}

///#############################################################
/*##############################################################
//RIDL ������Ϣ��1�� from RPU Spec v1.9

(1)  �������ԴΪMBʱ��������Ϣ [0:15] λ

[0]		1bit	����Ϊ0����ʶ�������ԴΪMB

[1:5]	5bits	��ʾ�����Դ�洢��������2D��ݵ��׵�ַ��0x00~0x1F��

[6:10]	5bits	��ʾ�����Դ�洢��������2D��ݵ�ÿ�г��ȣ�1~32�������ȵ�λΪ�ֽ�

[11:15] 5bits	��ʾ�����Դ�洢��������2D��ݵ�����1~32��


(2) �������ԴΪEIʱ��������Ϣ [0:15] λ

[0]		1bit	����Ϊ1����ʶ�������ԴΪEI

[1:5]	5bits	��ʾ�����Դ�洢��������ݵ��׵�ַ��0x00~0x1F��

[6:10]	5bits	����

[11:15] 5bits	��ʾ�����Դ�洢��������ݵ�����1~32��

(3) �������Ŀ��ΪRIFʱ

[16:17]	2bits	����

[18]	1bit	����Ϊ0,��ʶ�������Ŀ��ΪRIF

[19:20]	2bits	���������ԴΪMBʱ��ʾ��ݵ�ƴ��ģʽ�����������ԴΪEIʱ������
				00:	��ʶ���Դ�е�ÿһ�����ƴ��Ϊ���Ŀ���1����ݣ���ƴ�ӣ�
				01:	ÿ2�����ƴ��Ϊ1����ݣ�������32Byte
				10:	ÿ4�����ƴ��Ϊ1����ݣ�������32Byte
				11:	ÿ8�����ƴ��Ϊ1����ݣ�������32Byte
[21:31]	����

(4) �������Ŀ��ΪRIMʱ

[16:17]	2bits	����

[18]	1bit	����Ϊ1,��ʶ�������Ŀ��ΪRIM

[19:20]	2bits	���������ԴΪMBʱ��ʾ��ݵ�ƴ��ģʽ�����������ԴΪEIʱ������
				00:	��ʶ���Դ�е�ÿһ�����ƴ��Ϊ���Ŀ���1����ݣ���ƴ�ӣ�
				01:	ÿ2�����ƴ��Ϊ1����ݣ�������32Byte
				10:	ÿ4�����ƴ��Ϊ1����ݣ�������32Byte
				11:	ÿ8�����ƴ��Ϊ1����ݣ�������32Byte

[21:25]	5bits	���������ԴΪMBʱ��ʾ�����Ŀ��洢��д���2D��ݵ��׵�ַ��0x00~0x1F��
				���������ԴΪEIʱ��ʾ�����Ŀ��洢��д����ݵ��׵�ַ��0x00~0x1F��

[26:31]	����

*/
#define SRC_MB		0
#define SRC_RB		1

#define MODE_1L		0 
#define MODE_ML		1
#define MODE_2L		2

#define RIDL_TGT_RIF	0 
#define RIDL_TGT_RIM	1

class RIDLReg: public Reg{
	
public:

	int sourceType() const; // 0: MB; 1: EI

	void setSourceType(int type);

	int inputBaseAddress() const;

	void setInputBaseAddress(int addr);

	int inputLength() const;

	void setInputLength(int length);

	int inputHeight() const;

	void setInputHeight(int height);

	int inputMode() const;

	void setInputMode(int mode);

	int inputTimes() const;

	void setInputTimes(int times);

	int RCAIndex() const;

	void setRCAIndex(int index);

	int targetType() const; // 0:RIF; 1: RIM

	void setTargetType(int type);

	int outputMode() const; 
	// Data mode: 00: one line; 01: mult-line; 
	//            10: two line; 11: reseve

	void setOutputMode(int mode);

	int outputBaseAddress() const;

	void setOutputBaseAddress(int addr);

};

inline int RIDLReg::sourceType() const{

	return static_cast<int>(regst & 0x00000001);
}

inline void RIDLReg::setSourceType(int type){

	regst &= 0xfffffffe;
	regst |= (type)? 0x00000001:0x00000000;
}

inline int RIDLReg::inputBaseAddress() const{

	//�޸� by liuxie
	//return static_cast<int>(regst & 0x0000000f);
	return static_cast<int>((regst & 0x0000003e) >> 1 ) + 1;
}

inline void RIDLReg::setInputBaseAddress(int addr){

	assert(0 <= addr && addr <= 31);

	//�޸� by liuxie
	//regst &= 0xffffffe1;
    regst &= 0xffffffc1;
	regst |= static_cast<reg32>(addr) << 1;
}

inline int RIDLReg::inputLength() const {

	return static_cast<int>( (regst & 0x000007c0) >> 6) + 1;
}

inline void RIDLReg::setInputLength(int length) {

	assert(1 <= length && length <= 32);

	regst &= 0xfffff83f;
	regst |= static_cast<reg32>(length - 1) << 6;
}

inline int RIDLReg::inputHeight() const {

	return static_cast<int>( (regst & 0x0000f800) >> 11) + 1;
}

inline void RIDLReg::setInputHeight(int height) {

	assert(1 <= height && height <= 32);

	regst &= 0xffff07ff;
	regst |= static_cast<reg32>(height-1) << 11;
}

inline int RIDLReg::inputMode() const {

	return static_cast<int>( (regst & 0x00000006) >> 1);
}

inline void RIDLReg::setInputMode(int mode) {

	assert(0 <= mode && mode <= 3);

	regst &= 0xfffffff9;
	regst |= static_cast<reg32>(mode) << 1;
}

inline int RIDLReg::inputTimes() const {

	return static_cast<int>( (regst & 0x00000018) >> 3);
}

inline void RIDLReg::setInputTimes(int times) {

	assert(0 <= times && times <= 3);

	regst &= 0xffffffe7;
	regst |= static_cast<reg32>(times) << 3;
}

inline int RIDLReg::RCAIndex() const{

	return static_cast<int>( (regst & 0x00030000) >> 16);
}

inline void RIDLReg::setRCAIndex(int index) {

	assert(0 <= index && index <= 3);

	regst &= 0xfffcffff;
	regst |= static_cast<reg32>(index) << 16;
}

inline int RIDLReg::targetType() const {

	return static_cast<int>( (regst & 0x00040000) >> 18);
}

inline void RIDLReg::setTargetType(int type) {

	regst &= 0xfffbffff;
	regst |= (type)? 0x00040000:0x00000000;
}

inline int RIDLReg::outputMode() const {
	
	return static_cast<int>( (regst & 0x00180000) >> 19);
}

inline void RIDLReg::setOutputMode(int mode) {

	assert(0 <= mode && mode <= 3);

	regst &= 0xffe7ffff;
	regst |= static_cast<reg32>(mode) << 19;
}

inline int RIDLReg::outputBaseAddress() const {

	return static_cast<int>( (regst & 0x03e00000) >> 21);
}

inline void RIDLReg::setOutputBaseAddress(int addr) {

	assert(0 <= addr && addr <= 31);

	regst &= 0xffc1ffff;
	regst |= static_cast<reg32>(addr) << 21;
}

///############################################################################################################################################
//Core ������Ϣ���� from RPU SPECv1.9
/*
[13][6:0] 8bits  ��ʾcore������Ϣ��ѡ���ź�(0~255)��������256��RCA Core������Ϣ�ɹ�ѡ��

[12:7]    6bits  ��ʾRCA Coreѭ������

[15:14]   2bits  ��ʾѡ���������ѡ��
                 00��������0~3��������0��Ч��
                 01��������0~3��������1��Ч��
                 02��������0~3��������2��Ч��
                 03��������0~3��������3��Ч��

[21:16]   6bits  ��ʾRCA Core��const1�Ļ��ַ(0x00~0x1F)����Configuration Word�е�cons1ƫ�Ƶ�ַ�ĺ�Ϊconst1��ʵ�ʵ�ַ

[28:22]   7bits  ��const2ƫ�Ƶ�ַ��Чʱ����ʾRCA Core��const2�Ļ��ַ(0x00~0x7F)����Configuration Word�е�const2ƫ�Ƶ�ַ0��ƫ�Ƶ�ַ1�ĺ�Ϊconst2��ʵ�ʵ�ַ(0x00~0xFF)

[29]      1bit   ��const2ƫ�Ƶ�ַ��Чʱ��const2ƫ�Ƶ�ַѡ��
                 0��ѡ��const2ƫ�Ƶ�ַ0
				 1��ѡ��const2ƫ�Ƶ�ַ1

[29:22]   8bits  ��const2ƫ�Ƶ�ַ��Чʱ����ʾRCA Core��const2��ʵ�ʵ�ַ(0x00~0xFF)

[30]      1bit   CEDLʹ��λ��1��Ч
[31]      1bit   CIDLʹ��λ��1��Ч

*/

class RCACoreReg: public Reg{

public:

	int coreIndex() const;

	void setCoreIndex(int index);

	int coreLoop() const;

	void setCoreLoop(int loop);

	int selectImmIndex() const;

	void setSelectImmIndex(int index);

	int const1Address() const;

	void setConst1Address(int addr);

	int const2Address() const;

	void setConst2Address(int addr);

	bool CEDLEnable() const;

	void setCEDLEnable(bool enable);

	bool CIDLEnable() const;

	void setCIDLEnable(bool enable);
};

inline int RCACoreReg::coreIndex() const{

	//[13][6:0]  8bits
	return static_cast<int>( ((regst & 0x00002000)>>6)|(regst & 0x0000007f) );
}

inline void RCACoreReg::setCoreIndex(int index) {

	assert(0 <= index && index <= 0xff);

	regst &= 0xffffdf80;   //initialize
	//20110720 liuxie
	regst |= static_cast<reg32>( ((index>>7)<<13) | (index & 0x0000007f) );
}

inline int RCACoreReg::coreLoop() const {

	return static_cast<int>((regst & 0x00001f80) >> 7) + 1;
}

inline void RCACoreReg::setCoreLoop(int loop) {

	assert(1 <= loop && loop <= 0x40);

	regst &= 0xffffe07f;
	regst |= static_cast<reg32>(loop-1) << 7;
}

inline int RCACoreReg::selectImmIndex() const {

	return static_cast<int>((regst & 0x0000c000) >> 14);
}

inline	void RCACoreReg::setSelectImmIndex(int index) {

	assert(0 <= index && index <= 3);

	regst &= 0xffff3fff;
	regst |= static_cast<reg32>(index) << 14;
}

inline int RCACoreReg::const1Address() const{

	return static_cast<int>((regst & 0x003f0000) >> 16);
}

inline void RCACoreReg::setConst1Address(int addr) {

	assert(0 <= addr && addr <= 0x3f);

	regst &= 0xffc0ffff;
	regst |= static_cast<reg32>(addr) << 16;
}

inline int RCACoreReg::const2Address() const{

	return static_cast<int>((regst & 0x3fc00000) >> 22);
}

inline void RCACoreReg::setConst2Address(int addr) {

	assert(0 <= addr && addr <= 0xff);

	regst &= 0xc03fffff;
	regst |= static_cast<reg32>(addr) << 22;
}

inline bool RCACoreReg::CEDLEnable() const {

	return (regst & 0x40000000)? true : false;
}

inline void RCACoreReg::setCEDLEnable(bool enable) {

	regst &= 0xbfffffff;
	regst |= enable? 0x40000000 : 0x00000000;
}

inline bool RCACoreReg::CIDLEnable() const {

	return (regst & 0x80000000)? true : false;
}

inline void RCACoreReg::setCIDLEnable(bool enable) {

	regst &= 0x7fffffff;
	regst |= enable? 0x80000000 : 0x00000000;
}

///#############################################################

#define CEDL_TGT_RIF	0
#define CEDL_TGT_RIM	1

class CEDLReg: public Reg{

public:

	int targetType() const;

	void setTargetType(int type);

	int dataSum() const;

	void setDataSum(int sum);

	int targetAddress() const;

	void setTargetAddress(int addr);
};

inline int CEDLReg::targetType() const {

	return static_cast<int>(regst & 0x00000001);
}

inline void CEDLReg::setTargetType(int type) {

	regst &= 0xfffffffe;
	regst |= type? 0x00000001: 0x00000000;
}

inline int CEDLReg::dataSum() const {

	return static_cast<int>((regst & 0x0000003e) >> 1) + 1;
}

inline void CEDLReg::setDataSum(int sum) {

	assert(1 <= sum && sum <= 32);

	regst &= 0xffffffc1;
	regst |= static_cast<reg32>(sum-1) << 1;
}

inline int CEDLReg::targetAddress() const {

	return static_cast<int>((regst & 0x000007c0) >> 6);
}

inline void CEDLReg::setTargetAddress(int addr) {

	assert(0 <= addr && addr <= 0x1f);

	regst &= 0xfffff83f;
	regst |= static_cast<reg32>(addr) << 6;
}

///########################################################

#define MODE_IN_2D		0 
#define MODE_IN_T2D		1 
#define MODE_IN_M2D		2 
#define MODE_IN_V2D		3

#define MODE_OUT_1L		0
#define MODE_OUT_2L		1
#define MODE_OUT_4L		2
#define MODE_OUT_8L		3

class CIDLReg: public Reg{

public:

	int inputMode() const;

	void setInputMode(int mode);

	int input1BaseAddress() const;

	void setInput1BaseAddress(int addr);

	int input1Length() const;

	void setInput1Length(int length);

	int inputHeight() const;

	void setInputHeight(int height);

	int input2BaseAddress() const;

	void setInput2BaseAddress(int addr);

	int input2Length() const;

	void setInput2Length(int length);

	int outputMode() const;

	void setOutputMode(int mode);

	int dataBlockSum() const;

	void setDataBlockSum(int sum);

	int inputOffset() const;

	void setInputOffset(int offset);
};

inline int CIDLReg::inputMode() const {

	return static_cast<int>(regst & 0x00000003);
}

inline void CIDLReg::setInputMode(int mode) {

	assert(0 <= mode && mode <= 0x3);

	regst &= 0xfffffffc;
	regst |= static_cast<reg32>(mode);
}

inline int CIDLReg::input1BaseAddress() const {

	return static_cast<int>((regst & 0x0000007c) >> 2);
}

inline void CIDLReg::setInput1BaseAddress(int addr) {

	std::cout<<" Input Base Address is :  "<<addr<<std::endl ;

	assert(0 <= addr && addr <= 0x1f);

	regst &= 0xffffff83;
	regst |= static_cast<reg32>(addr) << 2;
}

inline int CIDLReg::input1Length() const {

	return static_cast<int>((regst & 0x00000f80) >> 7) + 1;
}

inline void CIDLReg::setInput1Length(int length) {

	assert(1 <= length && length <= 32);

	regst &= 0xfffff07f;
	regst |= static_cast<reg32>(length-1) << 7;
}

inline int CIDLReg::inputHeight() const {

	return static_cast<int>((regst & 0x0001f000) >> 12) + 1;
}

inline void CIDLReg::setInputHeight(int height) {

	assert(1 <= height && height <= 32);

	regst &= 0xfffe0fff;
	regst |= static_cast<reg32>(height-1) << 12;
}

inline int CIDLReg::input2BaseAddress() const {

	return static_cast<int>((regst & 0x003e0000) >> 17);
}

inline void CIDLReg::setInput2BaseAddress(int addr) {

	assert(0 <= addr && addr <= 0x1f);

	regst &= 0xffc1ffff;
	regst |= static_cast<reg32>(addr) << 17;
}

inline int CIDLReg::input2Length() const {

	return static_cast<int>((regst & 0x07c00000) >> 22) + 1;
}

inline void CIDLReg::setInput2Length(int length) {

	assert(1 <= length && length <= 32);

	regst &= 0xf83fffff;
	regst |= static_cast<reg32>(length-1) << 22;
}

inline int CIDLReg::outputMode() const {

	return static_cast<int>((regst & 0x18000000) >> 27);
}

inline void CIDLReg::setOutputMode(int mode) {

	assert(0 <= mode && mode <= 0x3);

	regst &= 0xe7ffffff;
	regst |= static_cast<reg32>(mode) << 27;
}

inline int CIDLReg::dataBlockSum() const {

	return static_cast<int>((regst & 0x00060000) >> 17) + 2;
}

inline void CIDLReg::setDataBlockSum(int sum) {

	assert(2 <= sum && sum <= 5);

	regst &= 0xffff9fff;
	regst |= static_cast<reg32>(sum - 2) << 17;
}

inline int CIDLReg::inputOffset() const {

	return static_cast<int>((regst & 0x003e0000) >> 17);
}

inline void CIDLReg::setInputOffset(int offset) {

	assert(0 <= offset && offset <= 31);

	regst &= 0xffc1ffff;
	regst |= static_cast<reg32>(offset) << 17;
}

///########################################################

#define CDS_TGT_RIF		0
#define CDS_TGT_RIM		1
#define CDS_TGT_ESDF	2 
#define CDS_TGT_MB		3
#define CDS_TGT_RB		4

//#########################################################
//CDS ������Ϣ���� from SPEC v1.9
/*CDS ������Ϣ(1)
  
[2:0]    000: ��ʶ�������Ŀ��ΪRIF
		 010: ��ʶ�������Ŀ��ΪESDF

[7:3]    ��ʾ������ݸ���(1~32)��ÿ����ݵĴ�СΪ32B
[31:8]	 ����

CDS ������Ϣ(2)

[2:0]	001:	��ʶ�������Ŀ��ΪRIM
		011:	��ʶ�������Ŀ��ΪMB
		100:	��ʶ�������Ŀ��ΪEI

[7:3]	��ʾ������ݸ���(1~32)��ÿ����ݴ�СΪ32B
[12:8]	��ʾ�����Ŀ��洢��д����ݵ��׵�ַ(0x00~0x1F)
[17:13]	��ʾ�����Ч��ݳ���(1~32)
[22:18]	��ʾ�����Ч���ƫ�Ƶ�ַ(0x00~0x1F)
[31:23]	����

CDS ������Ϣ(3)   ��ȡ����Ҫ�޸ı�ɾ��
//�ϰ汾��CDS ������Ϣ(3)���£�

[2:0]	100:��ʶ�������Ŀ��ΪRB
[31:3]	����

*/


class CDSReg: public Reg{

public:

	int targetType() const;

	void setTargetType(int type);

	int dataSum() const;

	void setDataSum(int sum);

	int RIMAddress() const;

	void setRIMAddress(int addr);

	int RIMLength() const;

	void setRIMLength(int length);

	int RIMOffset() const;

	void setRIMOffset(int offset);

	int MBAddress() const;

	void setMBAddress(int addr);

	int MBLength() const;

	void setMBLength(int length);

	int MBOffset() const;

	void setMBOffset(int offset);

	int RBAddress() const;

	void setRBAddress(int addr);

	int RBOffset() const;

	void setRBOffset(int offset);

	int RBDataSize() const;

	void setRBDataSize(int size);
};

inline int CDSReg::targetType() const {

	return static_cast<int>(regst & 0x00000007);
}

inline void CDSReg::setTargetType(int type) {

	assert(0 <= type && type <= 7);
    
	regst &= 0xfffffff8;
	regst |= static_cast<reg32>(type);
}

inline int CDSReg::dataSum() const {

	return static_cast<int>((regst & 0x000000f8) >> 3 ) + 1;
}

inline void CDSReg::setDataSum(int sum) {

	assert(1 <= sum && sum <= 32);

	regst &= 0xffffff07;
	regst |= static_cast<reg32>(sum - 1) << 3;
}

inline int CDSReg::RIMAddress() const {

	return static_cast<int>((regst & 0x00001f00) >> 8 );
}

inline void CDSReg::setRIMAddress(int addr) {

	assert(0 <= addr && addr <= 31);

	regst &= 0xffffe0ff;
	regst |= static_cast<reg32>(addr) << 8;
}

inline int CDSReg::RIMLength() const {

	return static_cast<int>((regst & 0x0003e000) >> 13 ) + 1;
}

inline void CDSReg::setRIMLength(int length) {

	assert(1 <= length && length <= 32);

	regst &= 0xfffc1fff;
	regst |= static_cast<reg32>(length - 1) << 13;
}

inline int CDSReg::RIMOffset() const {

	return static_cast<int>((regst & 0x007c0000) >> 18 );
}

inline void CDSReg::setRIMOffset(int offset) {

	assert(0 <= offset && offset <= 31);

	regst &= 0xff83ffff;
	regst |= static_cast<reg32>(offset) << 18;
}

inline int CDSReg::MBAddress() const {  return RIMAddress(); }

inline void CDSReg::setMBAddress(int addr) { setRIMAddress(addr); }

inline int CDSReg::MBLength() const { return RIMLength(); }

inline void CDSReg::setMBLength(int length) { setRIMLength(length); }

inline int CDSReg::MBOffset() const {return RIMOffset(); }

inline void CDSReg::setMBOffset(int offset) { setRIMOffset(offset); }

inline int CDSReg::RBAddress() const {

	return static_cast<int>((regst & 0x0001ff00) >> 8 );
}

inline void CDSReg::setRBAddress(int addr) {

	assert(0 <= addr && addr <= 0x1ff);

	regst &= 0xfffe00ff;
	regst |= static_cast<reg32>(addr) << 8;
}

inline int CDSReg::RBOffset() const {

	return static_cast<int>((regst & 0x000e0000) >> 17 );
}

inline void CDSReg::setRBOffset(int offset) {

	assert(0 <= offset && offset <= 7);

	regst &= 0xff1fffff;
	regst |= static_cast<reg32>(offset) << 17;
}

inline int CDSReg::RBDataSize() const{

	return static_cast<int>( (regst & 0x00300000) >> 20 );
}

inline void CDSReg::setRBDataSize(int size)  {

	assert(0 <= size && size <= 3);

	regst &= 0xffcfffff;
	regst |= static_cast<reg32>(size) << 20;
}

///#######################################################

#define CL1_MAX_RIDL_CONTEXT	8
#define CL1_MAX_RCA_CONTEXT		16

class CL1Regs {

public:

	GroupHeadReg & groupContext() { return CL1GroupReg; }

	RIDLReg & RIDLContext(int index) { 

		assert(0 <= index && index <= CL1_MAX_RIDL_CONTEXT-1);
		return CL1RIDLReg[index]; 
	}

	RCACoreReg & RCContext(int index) {

		assert(0 <= index && index <= CL1_MAX_RCA_CONTEXT-1);
		return CL1RCACoreReg[index];
	}

	CEDLReg & CEDLContext(int index) {

		assert(0 <= index && index <= CL1_MAX_RCA_CONTEXT-1);
		return CL1CEDLReg[index];
	}

	CIDLReg & CIDLContext(int index) {

		assert(0 <= index && index <= CL1_MAX_RCA_CONTEXT-1);
		return CL1CIDLReg[index];
	}

	CDSReg & CDSContext(int index) {

		assert(0 <= index && index <= CL1_MAX_RCA_CONTEXT-1);
		return CL1CDSReg[index];
	}

private:

	GroupHeadReg CL1GroupReg;

	RIDLReg CL1RIDLReg[CL1_MAX_RIDL_CONTEXT];

	RCACoreReg CL1RCACoreReg[CL1_MAX_RCA_CONTEXT];
	CEDLReg CL1CEDLReg[CL1_MAX_RCA_CONTEXT];
	CIDLReg CL1CIDLReg[CL1_MAX_RCA_CONTEXT];
	CDSReg CL1CDSReg[CL1_MAX_RCA_CONTEXT];
};

#endif

#define CL1_MAX_REDL_CONTEXT	8
#define CL1_MAX_REDS_CONTEXT    8
