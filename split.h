/*************************************************************
 *			File name: splt.h
 *------------------------------------------------------------
 * Description:
 *	This class split a DFG into subgraph splits can put into RCA. 
 *
 * Author: L. Xie @ MicroE. SJTU
 * Date : 2009/11/13
 **************************************************************
 */


#ifndef SPLIT_H
#define SPLIT_H

class DFGraph;
class RPUConfig;

class SplitMethod{

public:

	// This function splits the DFG of a configuration into 
	// subgraps can put into RCA, and stores the rusult into 
	// the configuration's RCA list.

	virtual int splitDFG( RPUConfig & config ) const;

	virtual ~SplitMethod(){}
};


#endif
