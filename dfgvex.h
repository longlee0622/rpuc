/************************************************
 *			File name: dfgvex.h
 *-----------------------------------------------
 * Description:
 *
 * Author: L. Xie @ MicroE. SJTU
 * Date : 2009/11/13
 *************************************************
 */

#ifndef DFGVEX_H
#define DFGVEX_H

#include "tpdef.h"

#include <iosfwd>

class RCA;

////////////////////////////////////////////////////////////////////////////

class DFGVex {

public:

	DFGVex(): vexID(genarateID()){}

	ID id() const { return vexID; }

	//-----------------------------
	
	virtual DFGVex * copy() const = 0;

	virtual void print(std::ostream & out) const = 0;

	virtual ~DFGVex() {}

private:

	ID vexID;

	static ID genarateID();
};

inline ID DFGVex::genarateID(){
	static ID number = 0;
	return number ++;
}

inline std::ostream & 
operator <<(std::ostream & out, const DFGVex & vex){

	vex.print(out);
	return out;
}

#endif

