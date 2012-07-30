
#ifndef REG_H
#define REG_H

#include "tpdef.h"

#define RegSetbit(reg,bit) ( reg |= (1 << bit) )
#define RegSetbits(reg,Lowbit,Highbit)  \
        do{for(int i=Lowbit;i<Highbit;i++) RegSetbit(reg,i);}while(0)

/// 32bit registor
typedef UINT	reg32;

//Flag
enum FLAG{
	FLAG_ZERO = 0,
	FLAG_ONE = 1
};


class Reg{

public:

	Reg(reg32 init = 0): regst(init) {}

	reg32 reg() const { return regst; }

	void reset() { regst = 0;}

protected:

	reg32 regst;
};

#endif
