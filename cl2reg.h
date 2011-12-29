// Filename: cl2reg.h
//--------------------------------------------------------------
// Author: Xie Li@ MicroE. SJTU  Time: 2010/07/10
//
// Define the registor operation of CL1, Based on RPU SPEC v1.3
// ==============================================================
// Reference: RPU SPEC v1.3.doc, Configuration1.8.doc
//
//
//
// 2:Revised Version
// Revised by Shizhu Liu on 2010/10/8
// Reference: RPU SPEC v1.9.pdf,Configuration1.9.pdf

#ifndef CL2REG_H
#define CL2REG_H

#include "reg.h"
#include <cassert>

#include "platf.h"


#define SRC_TYPE_FIFO     0
#define SRC_TYPE_LAST     1
#define SRC_TYPE_CONST    2
#define SRC_TYPE_HOLD     3

/*#################################################################################
---引自configuration1.9第三部分
Reconfigurable Cell中
*/
class InputReg: public Reg{

public:

	int inputA() const;

	void setInputA(int a_in);

	int inputAType() const;

	void setInputAType(int type);

	int inputB() const;

	void setInputB(int b_in);

	int inputBType() const;

	void setInputBType(int type);

	int inputTemp() const;

	void setInputTemp(int temp_in);

	int inputTempType() const;

	void setInputTempType(int type);

	int opecode() const;

	void setOpecode(int opecode);
};

inline int InputReg::inputA() const {

	return static_cast<int>(regst & 0x0000007f);
}

inline void InputReg::setInputA(int a_in) {

	assert(0 <= a_in && a_in <= 0x7f);

	int a_in_row = a_in/16;
	int a_in_col = a_in%16;
	int high1bit = 0;
	int low2bit = 0;
	high1bit |= (a_in_row>>2);
	low2bit = a_in_row & 3;
	low2bit = low2bit << 5;

	regst &= 0xffffff80;
	regst |= static_cast<reg32>(low2bit);
	regst |= static_cast<reg32>(a_in_col);
}

inline int InputReg::inputAType() const{

	return static_cast<int>( (regst & 0x00000180) >> 7);
}

inline void InputReg::setInputAType(int type){

	assert(0 <= type && type <= 3);

	regst &= 0xfffffe7f;
	regst |= ( static_cast<reg32>(type) << 7 );
}

inline int InputReg::inputB() const {

	return static_cast<int>( (regst & 0x0000fe00) >> 9);
}

inline void InputReg::setInputB(int b_in) {

	assert(0 <= b_in && b_in <= 0x7f);

	int b_in_row = b_in/16;
	int b_in_col = b_in%16;
	int high1bit = 0;
	int low2bit = 0;
	high1bit |= (b_in_row>>2);
	low2bit = b_in_row & 3;
	low2bit = low2bit << 14;
	b_in_col = b_in_col << 9;

	regst &= 0xffff01ff;
	regst |= ( static_cast<reg32>(b_in_col));
	regst |= ( static_cast<reg32>(low2bit));

}

inline int InputReg::inputBType() const{

	return static_cast<int>( (regst & 0x00030000) >> 16);
}

inline void InputReg::setInputBType(int type){

	assert(0 <= type && type <= 3);

	regst &= 0xfffcffff;
	regst |= (static_cast<reg32>(type) << 16);
}

inline int InputReg::inputTemp() const {

	return static_cast<int>( (regst & 0x07fc0000) >> 18);
}

inline void InputReg::setInputTemp(int temp_in) {

	assert(0 <= temp_in && temp_in <= 0xff);
	//2011.6.11 liuxie 
	int row = temp_in / FIFO_WIDTH_DATA;
	int col = temp_in % FIFO_WIDTH_DATA;
	row &= 7;  //取低3位

	regst &= 0xfe03ffff;
	//regst |= (static_cast<reg32>(temp_in) << 18);
	regst |= (static_cast<reg32>(col) << 18);
	regst |= (static_cast<reg32>(row) << 23);

}

inline int InputReg::inputTempType() const{

	return static_cast<int>( (regst & 0x06000000) >> 25);
}

inline void InputReg::setInputTempType(int type){

	assert(0 <= type && type <= 3);

	regst &= 0xf9ffffff;
	regst |= ( static_cast<reg32>(type) << 25 );
}

inline int InputReg::opecode() const {

	return static_cast<int>( (regst & 0xf8000000) >> 18);
}

inline void InputReg::setOpecode(int opc) {

	assert(0 <= opc && opc <= 0x1f);

	regst &= 0x07ffffff;
	regst |= ( static_cast<reg32>(opc) << 27 );
}

//##########################################################

class OutputReg: public Reg{

public:

	int reg0Out() const;

	void setReg0Out(int out0);

	int temp0Out() const;

	void setTemp0Out(int out0);

	int reg1Out() const;

	void setReg1Out(int out1);

	int temp1Out() const;

	void setTemp1Out(int out1);
};

inline int OutputReg::reg0Out() const{

	return static_cast<int>(regst & 0x000000ff);
}

inline void OutputReg::setReg0Out(int out0){

	//2011.6.9 liuxie
	assert(0 <= out0 && out0 <= 0xff);
	int row = out0 / FIFO_WIDTH_DATA;
	int col = out0 % FIFO_WIDTH_DATA;

	regst &= 0xffffff00;
	//regst |= ( static_cast<reg32>(out0) );
	regst |= ( static_cast<reg32>(col) );
	regst |= ( static_cast<reg32>(row) << 5);
}

inline int OutputReg::temp0Out() const{

	return static_cast<int>( (regst & 0x0000ff00) >> 8);
}

inline void OutputReg::setTemp0Out(int out0){

	//2011.6.11 liuxie
	assert(0 <= out0 && out0 <= 0xff);

	int row = out0 / FIFO_WIDTH_DATA;
	int col = out0 % FIFO_WIDTH_DATA;

	regst &= 0xffff00ff;
	//regst |= ( static_cast<reg32>(out0) << 8 );
	regst |= ( static_cast<reg32>(col) << 8 );
	regst |= ( static_cast<reg32>(row) << 13 );
}

inline int OutputReg::reg1Out() const{

	return static_cast<int>( (regst & 0x00ff0000) >> 16);
}

inline void OutputReg::setReg1Out(int out1){

	assert(0 <= out1 && out1 <= 0xff);
	//2011.6.9 liuxie
	int row = out1 / FIFO_WIDTH_DATA;
	int col = out1 % FIFO_WIDTH_DATA;
	
	regst &= 0xff00ffff;
	//regst |= ( static_cast<reg32>(out1) << 16 );
	regst |= ( static_cast<reg32>(col) << 16 );
	regst |= ( static_cast<reg32>(row) << 21 );

}

inline int OutputReg::temp1Out() const{

	return static_cast<int>( (regst & 0xff000000) >> 24);
}

inline void OutputReg::setTemp1Out(int out1){

	//2011.6.11 liuxie
	assert(0 <= out1 && out1 <= 0xff);

	int row = out1 / FIFO_WIDTH_DATA;
	int col = out1 % FIFO_WIDTH_DATA;

	regst &= 0x00ffffff;
	//regst |= ( static_cast<reg32>(out1) << 24 );
	regst |= ( static_cast<reg32>(col) << 24 );
	regst |= ( static_cast<reg32>(row) << 29 );
}

//############################################################

class OutEnableReg: public Reg{

public:

	bool enable(int index) const;

	void setEnable(int index);

	void setDisable(int index);
};

inline bool OutEnableReg::enable(int index) const {

	assert(0 <= index && index <= 31);

	return (regst & (1<<index)) != 0;
}

inline void OutEnableReg::setEnable(int index){

	assert(0 <= index && index <= 31);

	regst |= ( 1 << index );

}

inline void OutEnableReg::setDisable(int index){

	assert(0 <= index && index <= 31);

	regst &= ( ~(1 << index));

}

//############################################################

#define SIGN_MODE_UNCHG   0
#define SIGN_MODE_FORCE   1

#define GRAN_IN_8              0
#define GRAN_IN_16             1

#define GRAN_OP_OUT_8_8        0
#define GRAN_OP_OUT_8_16       1
#define GRAN_OP_OUT_16_8       2
#define GRAN_OP_OUT_16_16      3

#define GRAN_CONST_8           0
#define GRAN_CONST_16          1

/*#################################################################
---引自Computing Controller接口定义（RPU_SPEC_v1.9 P70）

*/

class ControlReg: public Reg{

public:

	int inputTimes() const;

	void setInputTimes(int times);

	int outputTimes() const;

	void setOutputTimes(int times);

	int loop() const;

	void setLoop(int loop);

	int loopInterval() const;

	void setLoopInterval(int interval);

	int outputGran() const;

	void setOutputGran(int gran);

	int RIFSignMode() const;

	void setRIFSignMode(int mode);

	int outputCycle() const;

	void setOutputCycle(int cycle);

	void setOutCG0Flag() ;

	void resetOutCG0Flag() ;

	void setOutCG1Flag() ;

	void resetOutCG1Flag() ;

	int inputAGran() const;

	void setInputAGran(int gran);

	int inputBGran() const;

	void setInputBGran(int gran);

	int inputTempGran() const;

	void setInputTempGran(int gran);

	int inputGranMode() const;

	void setInputGranMode(int mode);

	int constGran() const;

	void setConstGran(int gran);
};

inline int ControlReg::inputTimes() const{

	return static_cast<int>(
		(regst & 0x00000003) 
		| ((regst & 0x08000000) >> 25)
	) + 1;
}

inline void ControlReg::setInputTimes(int times) {

	assert(0 <= times && times <= 7);

	reg32 t = static_cast<reg32>(times);
	regst &= 0xf7fffffc;
	regst |= (( (t & 0x00000004) << 25) | ( t & 0x00000003));
}

inline int ControlReg::outputTimes() const{

	return static_cast<int>( (regst & 0x0000001c) >> 2) + 1;
}

inline void ControlReg::setOutputTimes(int times){

	assert(0 <= times && times <= 7);

	regst &= 0xffffffe3;
	regst |= ( static_cast<reg32>(times) << 2 );
}

inline int ControlReg::loop() const{

	return static_cast<int>( (regst & 0x00001e00) >> 9) + 1;
}

inline void ControlReg::setLoop(int loop) {

	assert(0 <= loop && loop <= 15);

	regst &= 0xffffe1ff;
	regst |= ( static_cast<reg32>(loop) << 9 );
}

inline int ControlReg::loopInterval() const{

	return static_cast<int>( (regst & 0x0001e000) >> 13) + 1;
}

inline void ControlReg::setLoopInterval(int interval) {

	assert(0 <= interval && interval <= 15);

	regst &= 0xfffe1fff;
	regst |= ( static_cast<reg32>(interval) << 13 );
}

inline int ControlReg::outputGran() const{

	return static_cast<int>( (regst & 0x00060000) >> 17);
}

inline void ControlReg::setOutputGran(int gran) {

	assert(0 <= gran && gran <= 3);

	regst &= 0xfff9ffff;
	regst |= ( static_cast<reg32>(gran) << 17 );
}

inline int ControlReg::constGran() const{

	return static_cast<int>( (regst & 0x00080000) >> 19);
}

inline void ControlReg::setConstGran(int gran) {

	assert(0 <= gran && gran <= 1);

	regst &= 0xfff7ffff;
	regst |= ( static_cast<reg32>(gran) << 19 );
}

inline int ControlReg::RIFSignMode() const{

	return static_cast<int>( (regst & 0x00100000) >> 20);
}

inline void ControlReg::setRIFSignMode(int mode) {

	assert(0 <= mode && mode <= 1);

	regst &= 0xffefffff;
	regst |= ( static_cast<reg32>(mode) << 20 );
}

inline int ControlReg::outputCycle() const{

	return static_cast<int>( (regst & 0x01e00000) >> 21);
}

inline void ControlReg::setOutputCycle(int cycle) {

	assert(0 <= cycle && cycle <= 15);

	regst &= 0xfe1fffff;
	regst |= ( static_cast<reg32>(cycle) << 21 );
}

inline void ControlReg::setOutCG0Flag() {

	regst |= (1 << 25);
}

inline void ControlReg::resetOutCG0Flag() {

	regst &= ~(1 << 25);
}

inline void ControlReg::setOutCG1Flag() {

	regst |= (1 << 26);
}

inline void ControlReg::resetOutCG1Flag() {

	regst &= ~(1 << 26);
}

inline int ControlReg::inputAGran() const{

	return static_cast<int>( (regst & 0x10000000) >> 28);
}

inline void ControlReg::setInputAGran(int gran) {

	assert(0 <= gran && gran <= 1);

	regst &= 0xefffffff;
	regst |= ( static_cast<reg32>(gran) << 28 );
}

inline int ControlReg::inputBGran() const{

	return static_cast<int>( (regst & 0x20000000) >> 29);
}

inline void ControlReg::setInputBGran(int gran) {

	assert(0 <= gran && gran <= 1);

	regst &= 0xdfffffff;
	regst |= ( static_cast<reg32>(gran) << 29 );
}

inline int ControlReg::inputTempGran() const{

	return static_cast<int>( (regst & 0x40000000) >> 30);
}

inline void ControlReg::setInputTempGran(int gran) {

	assert(0 <= gran && gran <= 1);

	regst &= 0xbfffffff;
	regst |= ( static_cast<reg32>(gran) << 30 );
}

inline int ControlReg::inputGranMode() const{

	return static_cast<int>( (regst & 0x80000000) >> 31);
}

inline void ControlReg::setInputGranMode(int gran) {

	assert(0 <= gran && gran <= 1);

	regst &= 0x7fffffff;
	regst |= ( static_cast<reg32>(gran) << 31 );
}

//############################################################

class InputHBitReg: public Reg{

public:

	FLAG bit(int index) const;

	void setBit(int index);

	void resetBit(int index);
};

inline FLAG InputHBitReg::bit(int index) const {

	assert(0 <= index && index <= 31);

	return (regst & (1<<index))? FLAG_ONE : FLAG_ZERO;
}

inline void InputHBitReg::setBit(int index) {

	assert(0 <= index && index <= 31);

	regst |= ( 1 << index );
}

inline void InputHBitReg::resetBit(int index) {

	assert(0 <= index && index <= 31);

	regst &= ~(1 << index);
}

//############################################################

class CL2Reg{

public:

	int inputA(int index) const;

	int inputAType(int index) const;

	void setInputA(int index, int a_in, int type);

	int inputB(int index) const;

	int inputBType(int index) const;

	void setInputB(int index, int b_in, int type);

	int inputTemp(int index) const;

	int inputTempType(int index) const;

	void setInputTemp(int index, int temp_in, int type);

	int RCOut(int index) const;

	void setRCOut(int index, int out);

	int TempOut(int index) const;

	void setTempOut(int index, int out);

	int RCOpecode(int index) const;

	void setRCOpecode(int index, int opd);

	bool RCOutEnable(int index) const;

	void setRCOutEnable(int index, bool enable);

	bool TempOutEnable(int index) const;

	void setTempOutEnable(int index, bool enable);

	//----------------------------------------------

	InputReg & inputReg(int index);

	OutputReg & outputReg(int index);

	OutEnableReg & outEnableReg(int index);

	ControlReg & controlReg();

	InputHBitReg & inputHBitReg(int index);

    ControlReg CL2ControlReg;

private:

	InputReg CL2InputReg[IN_OP_REG_SUM];

	OutputReg CL2OutputReg[OUT_REG_SUM];

	OutEnableReg CL2OutEnableReg[OUT_EN_SUM];

	InputHBitReg CL2InputHBitReg[IN_HBIT_SUM];
};

inline int CL2Reg::inputA(int index) const{

	assert(0 <= index && index <= 63);

	return CL2InputReg[index].inputA() 
		| ( 
			( (index < 32) ? CL2InputHBitReg[0].bit(index) 
			: CL2InputHBitReg[1].bit(index - 32) ) << 7
		  );
}

inline int CL2Reg::inputAType(int index) const{

	return CL2InputReg[index].inputAType();
}

inline void CL2Reg::setInputA(int index, int a_in, int type) {

	assert(0 <= index && index <= 63);
	//2011.5.28  liuxie
	assert(0 <= a_in && a_in <= 0xff);
	assert(0 <= a_in && a_in <= 0xff);

	CL2InputReg[index].setInputA(a_in & 0x7f);
	CL2InputReg[index].setInputAType(type);

	// Set Highest bit
	if(index < 32)
	{
		/*if(a_in & 0x80) CL2InputHBitReg[0].setBit(index);
		else CL2InputHBitReg[0].resetBit(index);*/
		if(a_in >= 64)
			CL2InputHBitReg[0].setBit(index);
		else
			CL2InputHBitReg[0].resetBit(index);
	} 
	else 
	{
		/*if(a_in & 0x80)CL2InputHBitReg[1].setBit(index - 32);
		else CL2InputHBitReg[1].resetBit(index - 32);*/
		if(a_in >= 64)
			CL2InputHBitReg[1].setBit(index-32);
		else
			CL2InputHBitReg[1].resetBit(index-32);

	}
}

inline int CL2Reg::inputB(int index) const{

	assert(0 <= index && index <= 63);

	return CL2InputReg[index].inputB() 
		| ( 
			( (index < 32) ? CL2InputHBitReg[2].bit(index) 
			: CL2InputHBitReg[3].bit(index - 32) ) << 7
		  );
}

inline int CL2Reg::inputBType(int index) const{

	return CL2InputReg[index].inputBType();
}

inline void CL2Reg::setInputB(int index, int b_in, int type) {

	assert(0 <= index && index <= 63);
	//2011.5.28 liuxie
	assert(0 <= b_in && b_in <= 0xff);
	assert(0 <= b_in && b_in <= 0xff);

	CL2InputReg[index].setInputB(b_in & 0x7f);
	CL2InputReg[index].setInputBType(type);

	// Set Highest bit
	if(index < 32){

		/*if(b_in & 0x80)CL2InputHBitReg[2].setBit(index);
		else CL2InputHBitReg[2].resetBit(index);*/
		if(b_in >= 64)
			CL2InputHBitReg[2].setBit(index);
		else
			CL2InputHBitReg[2].resetBit(index);
	} else {

		/*if(b_in & 0x80)CL2InputHBitReg[3].setBit(index - 32);
		else CL2InputHBitReg[3].resetBit(index - 32);*/
		if(b_in >= 64)
			CL2InputHBitReg[3].setBit(index-32);
		else
			CL2InputHBitReg[3].resetBit(index-32);
	}
}

inline int CL2Reg::inputTemp(int index) const{

	assert(0 <= index && index <= 63);

	return CL2InputReg[index].inputTemp() 
		+ ( 
			( (index < 32) ? CL2InputHBitReg[4].bit(index) 
			: CL2InputHBitReg[5].bit(index - 32) ) << 7
		  );
}

inline int CL2Reg::inputTempType(int index) const{

	return CL2InputReg[index].inputTempType();
}

inline void CL2Reg::setInputTemp(int index, int temp_in, int type) {

	assert(0 <= index && index <= 63);
	assert(0 <= temp_in && temp_in <= 0xff);

	CL2InputReg[index].setInputTemp(temp_in & 0x7f);
	CL2InputReg[index].setInputTempType(type);

	// Set Highest bit
	if(index < 32){

		/*if(temp_in & 0x80)CL2InputHBitReg[4].setBit(index);
		else CL2InputHBitReg[4].resetBit(index);*/
		if(temp_in >= 64)
			CL2InputHBitReg[4].setBit(index);
		else
			 CL2InputHBitReg[4].resetBit(index);
	} else {

		/*if(temp_in & 0x80)CL2InputHBitReg[5].setBit(index - 32);
		else CL2InputHBitReg[5].resetBit(index - 32);*/
		if(temp_in >= 64)
			CL2InputHBitReg[5].setBit(index-32);
		else
			 CL2InputHBitReg[5].resetBit(index-32);

	}
}

inline int CL2Reg::RCOut(int index) const{

	return (index & 0x01)? CL2OutputReg[index >>1].reg1Out()
		: CL2OutputReg[index >>1].reg0Out();
}

inline void CL2Reg::setRCOut(int index, int out) {

	if(index & 0x01)CL2OutputReg[index >>1].setReg1Out(out);
	else CL2OutputReg[index >>1].setReg0Out(out);
}

inline int CL2Reg::TempOut(int index) const{

	return (index & 0x01)? CL2OutputReg[index >>1].temp1Out()
		: CL2OutputReg[index >>1].temp0Out();
}

inline void CL2Reg::setTempOut(int index, int out) {

	if(index & 0x01)
		CL2OutputReg[index >>1].setTemp1Out(out);
	else 
		CL2OutputReg[index >>1].setTemp0Out(out);
}

inline int CL2Reg::RCOpecode(int index) const {

	assert(0 <= index && index <= 63);

	return CL2InputReg[index].opecode();
}

inline void CL2Reg::setRCOpecode(int index, int opd) {

	assert(0 <= index && index <= 63);

	CL2InputReg[index].setOpecode(opd);
}

inline bool CL2Reg::RCOutEnable(int index) const {

	assert(0 <= index && index <= 63);

	return (index < 32) ? CL2OutEnableReg[0].enable(index)
				: CL2OutEnableReg[1].enable(index - 32);
}

inline void CL2Reg::setRCOutEnable(int index, bool enable) {

	assert(0 <= index && index <= 63);

	if(index < 32) {

		if(enable)CL2OutEnableReg[0].setEnable(index);
		else if(enable)CL2OutEnableReg[0].setDisable(index);
	} else {

		if(enable)CL2OutEnableReg[1].setEnable(index - 32);
		else if(enable)CL2OutEnableReg[1].setDisable(index - 32);
	}
}

inline bool CL2Reg::TempOutEnable(int index) const {

	assert(0 <= index && index <= 63);

	return (index < 32) ? CL2OutEnableReg[2].enable(index)
				: CL2OutEnableReg[3].enable(index - 32);
}

inline void CL2Reg::setTempOutEnable(int index, bool enable) {

	assert(0 <= index && index <= 63);

	if(index < 32) {

		if(enable)CL2OutEnableReg[2].setEnable(index);
		else if(enable)CL2OutEnableReg[2].setDisable(index);
	} else {

		if(enable)CL2OutEnableReg[3].setEnable(index - 32);
		else if(enable)CL2OutEnableReg[3].setDisable(index - 32);
	}
}

inline InputReg & CL2Reg::inputReg(int index){

	assert(0 <= index && index <= IN_OP_REG_SUM-1);

	return CL2InputReg[index];
}

inline OutputReg & CL2Reg::outputReg(int index){

	assert(0 <= index && index <= OUT_REG_SUM-1);

	return CL2OutputReg[index];
}

inline OutEnableReg & CL2Reg::outEnableReg(int index) {

	assert(0 <= index && index <= OUT_EN_SUM-1);

	return CL2OutEnableReg[index];
}

inline ControlReg & CL2Reg::controlReg() {

	return CL2ControlReg;
}

inline InputHBitReg & CL2Reg::inputHBitReg(int index) {

	assert(0 <= index && index <= IN_HBIT_SUM-1);

	return CL2InputHBitReg[index];
}

#endif
