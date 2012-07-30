

#ifndef DFGPORT_H
#define DFGPORT_H

#include "dfgvex.h"
#include <iostream>


class DFGPort: public DFGVex{

public:

	DFGPort(): portSeqNo(-1), portSSRAMAddr(-1), IsImmPort(false){}

	//---------------------------------------------------

	int seqNo() const { return portSeqNo; }

	void setSeqNo(int number) { portSeqNo = number; }

	// Hardware Address
	int SSRAMAddress() const { return portSSRAMAddr; }

	void setSSRAMAddress(int addr) { portSSRAMAddr = addr; }

	bool isImmPort() {return IsImmPort;}

	void setPortType(bool is_ImmPort) {IsImmPort = is_ImmPort;}

private:

	int portSeqNo;

	int portSSRAMAddr;

	bool IsImmPort;
};


class DFGImmPort: public DFGPort {

public:

	DFGImmPort() : IsImmPort(true){}

	DFGImmPort(int immediate): immdt(immediate) {}

	int value() const { return immdt; }

	void setValue(int value) { immdt = value; }

	DFGVex * copy() const { return new DFGImmPort(*this); }

	void print(std::ostream & out) const { out <<immdt; }

	bool isImmPort() {return IsImmPort;}

	void setPortType(bool is_ImmPort) {IsImmPort = is_ImmPort;}

private:

	int immdt;
	
	bool IsImmPort;
};


class DFGVarPort: public DFGPort{

public:

	DFGVarPort() : IsImmPort(false) {}

	DFGVarPort(const String & name): varName(name),IsImmPort(false){}

	const String & name() const { return varName; }

	void setName(const String & name) { varName = name; }

	DFGVex * copy() const { return new DFGVarPort(*this); }

	void print(std::ostream & out) const { out<<varName; }

    bool isImmPort() {return IsImmPort;}

private:

	String varName;

	bool IsImmPort;
};


#endif
