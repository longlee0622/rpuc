/************************************************
 *			File name: dfgraph.h
 *-----------------------------------------------
 * Description:
 *
 * Author: L. Xie @ MicroE. SJTU
 * Date : 2009/11/13
 *************************************************
 */

#ifndef DFGRAPH_H
#define DFGRAPH_H


#include "dfgport.h"
#include "dfgnode.h"

#include <iostream>


class OptimizeMethod;


class DFGraph {

public:

	DFGraph() {}

	DFGraph(const DFGraph & graph);

	//-------------------------------------------------

	const String & name() const { return dfgName; }

	void setName(const String & name) { dfgName = name; }

	int width() const { return dfgWidth; }

	int depth() const { return dfgDepth; }

	int stepWidth(int step) const { return dfgStepWidth.at(step); }

	//-------------------------------------------------

	int parse(std::istream & dfgFile);

	List<List<DFGNode*> >  sort(); // sort the DFG nodes accoding to topologic location

	int optmize(OptimizeMethod * method);

	void display(std::ostream & out = std::cout);	

	//-------------------------------------------------

	int inSize() const { return inports.size(); }

	int outSize() const { return outports.size(); }

	const DFGPort & inport(int index) const { return * inports.at(index); }

	DFGPort & inport(int index) { return * inports.at(index); }

	const DFGPort & outport(int index) const { return * outports.at(index); }

	DFGPort & outport(int index) { return * outports.at(index); }

	DFGPort * newInport(const String & name);

	DFGPort * newOutport(const String & name);

	//2011.7.12 liuxie
	//Vector<Ptr<DFGPort> >  inports() { return inports; }

	//const Vector<Ptr<DFGPort> >  inports() const { return inports; }

	//Vector<Ptr<DFGPort> >  outports() { return outports; }

	//const Vector<Ptr<DFGPort> >  outports() const { return outports; }

	//-------------------------------------------------

	DFGNode * newNode(int opecode);

	int nodeSize() const { return nodes.size(); }

	const DFGNode & node(int index) const { return * nodes.at(index); }

	DFGNode & node(int index) { return * nodes.at(index); }

private:

	String dfgName;

	int dfgWidth;

	int dfgDepth;

	Vector<int> dfgStepWidth;

	Vector<Ptr<DFGPort> > inports, outports;  //the whole DFG input

	Vector<Ptr<DFGNode> > nodes;              //the whole nodes
};

#endif
