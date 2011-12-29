/*************************************************************
 *			File name: optmz.h
 *------------------------------------------------------------
 * Description:
 *	This class optimize DFG's operation in order to reduce node
 * of DFG and the execution time.
 *
 * Author: L. Xie @ MicroE. SJTU
 * Date : 2009/11/13
 **************************************************************
 */


#ifndef OPTIMIZE_H
#define OPTIMIZE_H

class DFGraph;
class RPUConfig;

class OptimizeMethod{

public:

	virtual int optimizeDFG( DFGraph & graph ) const { return 0;}

	virtual ~OptimizeMethod(){}

};


#endif
