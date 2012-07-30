/************************************************
 *			File name: util.h
 *-----------------------------------------------
 * Description:
 *
 * Author: L. Xie @ MicroE. SJTU
 * Date : 2009/11/13
 *************************************************
 */


#ifndef TYPE_DEFINE_H
#define TYPE_DEFINE_H

#include <iosfwd>
#include <string>
#include <vector>
#include <list>
#include <utility>
#include <typeinfo>

typedef int ID;
typedef int SizeT;
typedef unsigned long int UINT;
typedef std::string	String;

#define Vector  std::vector
#define List    std::list

//#define Ptr

template<class T>
class Ptr{

public:

	Ptr(T * ptr = 0): p(ptr){}

	Ptr(const Ptr<T> & ptr)
		: p(ptr.p? static_cast<T*>(ptr.p->copy()) : 0){}
    /* operator overloading of add */
	Ptr<T> & operator = (const Ptr<T> & ptr){
		T * temp = p;
		p = ptr.p? static_cast<T*>(ptr.p->copy()) : 0;
		delete temp;

		return *this;
	}

	~Ptr() { delete p; p = 0; }

	//------------------------------------------------

	T * operator ->() const { return p; }

	T & operator *() const { return * p; }

	T * get() const { return p;}

	void reset(T * ptr) { delete p; p = ptr; }

private:

	T * p;
};

#endif
