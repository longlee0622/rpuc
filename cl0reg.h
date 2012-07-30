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
-----------����SPECv1.9 ��ͷ ��ʽ˵��

[1:0]	2bits	8x8 RCA ����
				00: ����8x8 RCA
				01: ����8x8 RCA
				10: ����8x8 RCA
				11: ����8x8 RCA

[5:2]   4bits	REDL������Ϣ����(0~8)

[10:7]	4bits	REDS������Ϣ����(0~8)

[12]	1bit	��ʾ����������
				0: ��������
				1: ��2��32bit���������ֱ�Ϊѡ���������ͱ���������

[16:13]	4bits	���ð�Դ������Ҳ���ж���ӦԴ����
				0x0: ����ARM
				0x1~0x8: ��P1~��P8
				0x9~0xF: ����

[6]				����
[11]			����
*/

/*##################################################################
------����SPECv1.9 Configuration Word��CW����ʽ˵��

[12:0]   13bits  ��ʾcontext group��GCGM�е��׵�ַ

[15:13]  3bits   ��ʾ 8x8 RCA��ͬ��ģʽ
                 000������8x8 RCAִ���굱ǰ��CW������ִ�ж�Ӧ��CW FIFO�е���һ��CW(����еĻ�)��
				 001������8x8 RCAִ���굱ǰ��CW�󣬵ȴ�����3��8x8 RCAҲִ����ͬ��ģʽΪ001��CW����ִ�ж�Ӧ��CW FIFO�е���һ��(����еĻ�)��
				 010������8x8 RCAִ���굱ǰ��CW�󣬵ȴ�����3��8x8 RCAҲִ����ͬ��ģʽΪ010��CW�󷢳��жϣ���ִ�ж�Ӧ��CW FIFO�е���һ��(����еĻ�)��
				 011������8x8 RCAִ���굱ǰ��CW�󣬵ȴ���֮�󶨵�8x8 RCAҲִ����ͬ��ģʽΪ011��CW����ִ�ж�Ӧ��CW FIFO�е���һ��(����еĻ�)��
				      ����8x8 RCA������8x8 RCA����󶨣�����8x8 RCA������8x8 RCA����󶨡�
                 ������������

[16]     1bit    ƫ�Ƶ�ַ��Ч��ѡ��
                 0��const1��ƫ�Ƶ�ַ��Ч
				 1��const2��ƫ�Ƶ�ַ��Ч

[22:17]  6bits   ��[16]=0ʱ����ʾ8x8 RCA��const1��ƫ�Ƶ�ַ(0x00~0x3F)����Core Context�е�cons1����ַ�ĺ�Ϊconst1��ʵ�ʵ�ַ

[23:17]  7bits   ��[16]=1ʱ����ʾ8x8 RCA��const2��ƫ�Ƶ�ַ0(0x00~0x7F)������Core Contextѡ��ʱ����Core Context�е�const2����ַ�ĺ�Ϊconst2��ʵ�ʵ�ַ(0x00~0xFF)

[30:24]  7bits   ��[16]=1ʱ����ʾ8x8 RCA��const2��ƫ�Ƶ�ַ1(0x00~0x7F)������Core Contextѡ��ʱ����Core Context�е�const2����ַ�ĺ�Ϊconst2��ʵ�ʵ�ַ(0x00~0xFF)

[31:23]  ��[16]=0ʱ������

[31]     ��[16]=1ʱ������

*/

#define SYN_MODE_0	0
#define SYN_MODE_1	1
#define SYN_MODE_2	2
#define SYN_MODE_3	3

class CL0HeadReg: public Reg{

public:

	int groupAddress() const;                     //Context Group ��GCGM�е��׵�ַ��0x00~0x7FC�� [12:0] 

	void setGroupAdrress(int addr);               //���� Context Group ��GCGM�е��׵�ַ

	int synMode() const;                          //ͬ��ģʽ [15:13]

	void setSynMode(int mode);

	int constSelect() const;                      //ƫ�Ƶ�ַ��Ч��ѡ��  0�� const1��ƫ�Ƶ�ַ��Ч   1:const2��ƫ�Ƶ�ַ��Ч

	void setConstSelect(int sel);

	// set offset
	int constOffset1() const;                      //��ʾ8x8 RCA��const1��ƫ�Ƶ�ַ(0x00~0x3F),����Core Context�е�const1����ַ�ĺ�Ϊconst1��ʵ�ʵ�ַ�� [22:17]

	void setConstOffset1(int offset);

	int constOffset20() const;                     //��ʾ8x8 RCA��const2��ƫ�Ƶ�ַ(0x00~0x7F), [23:17]

	void setConstOffset20(int offset);

	int constOffset21() const;                     //��ʾ8x8 RCA�� const2 ��ƫ�Ƶ�ַ1��0x00~0x7F��������core contextѡ��ʱ����core context�е�const2����ַ�ĺ͹���const2��ʵ�ʵ�ַ

	void setConstOffset21(int offset);
};

inline int CL0HeadReg::groupAddress() const {

	return static_cast<int>(regst & 0x00001fff);    //���13λΪCL1 Context Group ���׵�ַ
}

inline void CL0HeadReg::setGroupAdrress(int addr){

	assert(0 <= addr && addr <= 0x1fff);

	regst &= 0xffffe000;                     //�Ȱѵ�13λ����
	regst |= static_cast<reg32>(addr);       //����13λ��ֵ
}

inline int CL0HeadReg::synMode() const{

	return static_cast<int>((regst & 0x0000e000) >> 13);  // 13~15λΪͬ��ģʽ
}

inline void CL0HeadReg::setSynMode(int mode) {

	assert(0 <= mode && mode <= 7);

	regst &= 0xffff1fff;                        //����14~16λ����
	regst |= static_cast<reg32>(mode) << 13;    //����14~16λ��ֵ
}

inline int CL0HeadReg::constSelect() const{

	return static_cast<int>((regst & 0x00010000) >> 16);    //��17λΪconst memoryƫ�Ƶ�ַ��Ч��ѡ��
}

inline void CL0HeadReg::setConstSelect(int sel) {

	assert(0 <= sel && sel <= 1);

	//2012.5.7 longlee ֮ǰ�����Ȼ�б��󣡣���
	//regst &= 0x0000e000;
	regst &= 0xfffeffff;
	regst |= static_cast<reg32>(sel) << 16;
}

inline int CL0HeadReg::constOffset1() const {

	return static_cast<int>((regst & 0x007e0000) >> 17);
}

inline void CL0HeadReg::setConstOffset1(int offset){

	assert(0 <= offset && offset <= 0x3f);

	//�޸� by liuxie
	//regst &= 0x00810000;
	//2012.5.7 longlee ֮ǰ�����б���
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

/*###################### Configuration Word (CW)�ж�ģʽ ####################
// CW ������Ϣ from RPU spec v1.9
[12:0]   ����
[15:13]  ��ʾ8x8 RCA��ͬ��ģʽ
         100��CW��Ч����ͬ��ģʽ��Ч��ֱ�ӷ����жϣ�
		 ����������
[31:16]  ����
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
//REDL ������Ϣ word1 from RPU Specv1.9

[0:23]	24bits	��ʾ������Դ�洢��������2D���ݵ��׵�ַ����0x000000~0xFFFFFFFF��,��Ѱַ16MByte �ռ�

[31:24] ����

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
//REDL ������Ϣword0 from Spec V1.9

[0:4]	 5bits  ��ʾ������Դ�洢��������2D���ݵ�ÿ�г���(1~32)�����ȵ�λΪ�ֽ�

[5:9]	 5bits  ��ʾ������Ŀ��洢��д���2D���ݵ�����(1~32)

[10:20]	 11bits ��ʾ������Ŀ��洢��д���2D���ݵ��м���ת����(0~2047)�����ȵ�λΪ�ֽ�

[22:21]	 2bits  ��ʾ����ƴ��ģʽ
				00�� ��ʶ����Դ�е�ÿ1������ƴ��Ϊ����Ŀ���е�1�����ݣ���ƴ�ӣ�
				01�� ��ʶ����Դ�е�ÿ2������ƴ��Ϊ����Ŀ���е�1�����ݣ��뱣֤ƴ�����õ�1�����ݲ�����32Byte��
				10�� ��ʶ����Դ�е�ÿ4������ƴ��Ϊ����Ŀ���е�1�����ݣ��뱣֤ƴ�����õ�1�����ݲ�����32Byte��
				11�� ��ʶ����Դ�е�ÿ8������ƴ��Ϊ����Ŀ���е�1�����ݣ��뱣֤ƴ�����õ�1�����ݲ�����32Byte��

[23:31]   ����
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
//REDS ������Ϣ word(1) from SPEC v1.9

[0:23]	24bits	��ʾ������Ŀ��洢��д���2D���ݵ��׵�ַ��0x000000~0xFFFFFFFF��,��Ѱַ16MByte�ռ�
[24:31] ����

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
//REDS ������Ϣ word0��1��from RPU Spec V1.9

[4:0]    5bits  ��ʾ������Դ�洢��������2D���ݵ�ÿ�г���(1~32)�����ȵ�λΪ�ֽ�

[9:5]    5bits  ��ʾ������Ŀ��洢��д���2D���ݵ�����(1~32)

[20:10]  11bits ��ʾ������Ŀ��洢��д���2D���ݵ��м���ת����(0~2047)�����ȵ�λΪ�ֽ�

[21]     1bit   ��Ϊ0����ʾ����ƴ��ģʽ����

[25:22]  4bits  ��ʾ����ƴ�ӵ�����(1~16)

[31:26]  ����

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

