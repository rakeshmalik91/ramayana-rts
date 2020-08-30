#ifndef __ARRAY_H
#define __ARRAY_H

#include "math.h"
#include "random.h"
#include "string.h"
#include "exception.h"

#include <cmath>
#include <vector>

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

#define offsetOf(class, member) (&((class*)NULL)->member)

using namespace math;

namespace std {

	class ArrayIndexOutOfBound : public Exception {
	public:
		ArrayIndexOutOfBound(int i, int lb, int ub) : Exception("Array index out of bound : " + toString(i) + " in " + toString(lb) + ":" + toString(ub)) {}
	};
	class InvalidArraySize : public Exception {
	public:
		InvalidArraySize(int size) : Exception("Invalid array size : " + toString(size)) {}
	};

	template <class DT> DT** allocate(int w, int h) {
		DT **ptr=new DT*[h];
		for(int r=0; r<h; r++)
			ptr[r]=new DT[w];
		return ptr;
	}
	template <class DT> DT** allocate(int w, int h, DT v) {
		DT **ptr=new DT*[h];
		for(int r=0; r<h; r++) {
			ptr[r]=new DT[w];
			for(int c=0; c<w; c++)
				ptr[r][c]=v;
		}
		return ptr;
	}
	template <class DT> void deallocate(DT **&ptr, int w, int h) {
		for(int r=0; r<h; r++)
			delete[] ptr[r];
		delete[] ptr;
		ptr=NULL;
	}
	template <class DT> void setAll(DT **ptr, int w, int h, DT v) {
		for(int r=0; r<h; r++)
			for(int c=0; c<w; c++)
				ptr[r][c]=v;
	}
	template <class DT> int count(DT **ptr, int w, int h, DT v) {
		int cnt=0;
		for(int r=0; r<h; r++)
			for(int c=0; c<w; c++)
				cnt++;
		return cnt;
	}

	template <class DT> bool contains(vector<DT> vec, DT v) {
		for(int i=0; i<vec.size(); i++)
			if(vec[i]==v)
				return true;
		return false;
	}

	template <class DT> void interchange(DT &a, DT &b) {
		DT c=a;
		a=b;
		b=c;
	}

	/*Comparator returns
	* -1 : less
	*  0 : equal
	*  1 : greater
	*/
	template <class DT> int __default_comparator_asc(const DT &a, const DT &b, void *param) {
		if(a<b)
			return -1;
		else if(a>b)
			return 1;
		else
			return 0;
	}
	template <class DT> int __default_comparator_desc(const DT &a, const DT &b, void *param) {
		if(a<b)
			return 1;
		else if(a>b)
			return -1;
		else
			return 0;
	}

	#define SORT_ASCENDING(DT) __default_comparator_asc<DT>
	#define SORT_DESCENDING(DT) __default_comparator_desc<DT>
	
	template <class DT> int partition(DT *list, size_t size, int (*comparator)(const DT&, const DT&, void*), void* param) {
		int pivot=0;
		for(int i=1,j=size-1;i<=j;) {
			if((*comparator)(list[i], list[pivot], param)==-1) {
				interchange(list[pivot], list[i]);
				i++;
				pivot++;
			}else {
				interchange(list[i], list[j]);
				j--;
			}
		}
		return pivot;
	}
	template <class DT> void quicksort(DT *list, size_t size, int (*comparator)(const DT&, const DT&, void*)=&__default_comparator_asc<DT>, void* param=NULL) {
		if(size>=1) {
			int pivot=partition(list, size, comparator, param);
			quicksort(list, pivot, comparator, param);
			quicksort(list+pivot+1, size-pivot-1, comparator, param);
		}
	}

	template <class DT> int find(const DT *list, size_t size, DT item, int (*comparator)(const DT&, const DT&, void*)=&__default_comparator_asc<DT>, void* param=NULL) {
		for(int i=0; i<size; i++)
			if((*comparator)(list[i], item, param)==0)
				return i;
		return -1;
	}
	template <class DT> bool contains(const DT *list, size_t size, DT item, int (*comparator)(const DT&, const DT&, void*)=&__default_comparator_asc<DT>, void* param=NULL) {
		return find(list, size, item, comparator, param)>=0;
	}

	template <class DT> void reverse(DT *list, size_t size) {
		for(int i=0; i<size/2; i++)
			interchange(list[i], list[size-i-1]);
	}

	#define arrayLength(a) (sizeof(a)/sizeof(a[0]))

	template <class DT> vector<DT>& operator+=(vector<DT>& v1, const vector<DT>& v2) {
		for(int i=0; i<v2.size(); i++)
			v1.push_back(v2[i]);
		return v1;
	}
	template <class DT> vector<DT> operator+(const vector<DT>& v1, const vector<DT>& v2) {
		vector<DT> v3;
		v3+=v1;
		v3+=v2;
		return v3;
	}
	template <class DT> vector<DT>& operator+=(vector<DT>& v, const DT& e) {
		v.push_back(e);
		return v;
	}
	template <class DT> vector<DT> operator+(const vector<DT>& v, const DT& e) {
		vector<DT> v2;
		v2+=v;
		v2+=e;
		return v2;
	}
	template <class DT> vector<DT> operator+(const DT& e, const vector<DT>& v) {
		vector<DT> v2;
		v2+=v;
		v2.insert(v2.begin(), e);
		return v2;
	}
	template <class DT> vector<DT> operator<<(vector<DT> vec, DT d) {
		vec.push_back(d);
		return vec;
	}
	template <class DT> vector<DT>& operator<<(vector<DT>& vec, DT d) {
		vec.push_back(d);
		return vec;
	}
			
	template<class DT, size_t m> class Array {
		DT data[m];
	public:
		Array() {}
		Array(DT);
		Array(DT*);
		Array(const Array<DT, m>&);
		DT& operator[] (int);
		const DT& operator[] (int) const;
		operator DT* ();
		size_t length();
		void sort(int (*)(DT&, DT&, void*), void*);
	};
	template<class DT, size_t m, size_t n> class Matrix {
		Array<DT, m> data[n];
	public:
		unsigned int rows() const {return m;}
		unsigned int columns() const {return n;}
		Matrix() {}
		Matrix(DT);
		Matrix(DT*);
		Matrix(DT**);
		operator DT*() {return (DT*)data;}
		operator DT**() {return (DT**)data;}
		DT* toArray() {return (DT*)data;}
		DT& operator() (int, int);
		Array<DT, m>& operator[] (int);
		const Array<DT, m>& operator[] (int) const;
	};
	template<class DT> class DynamicArray {
		DT* data;
		int n;
	public:
		DynamicArray() : data(NULL), n(0) {}
		DynamicArray(int n);
		DynamicArray(int n, DT*);
		DynamicArray(const DynamicArray<DT>&);
		~DynamicArray();
		DynamicArray<DT>& operator=(const DynamicArray<DT>&);
		DT& operator[] (int);
		const DT& operator[] (int) const;
		operator DT* ();
		size_t length() const;
		void setAll(const DT&);
	};
	template<class DT> class DynamicMatrix {
		DynamicArray<DT> *data;
		int m, n;
	public:
		unsigned int rows() const {return m;}
		unsigned int columns() const {return n;}
		DynamicMatrix() : data(NULL), m(0), n(0) {}
		DynamicMatrix(int, int);
		DynamicMatrix(int, int, DT*);
		DynamicMatrix(int, int, DT**);
		DynamicMatrix(const DynamicMatrix<DT>&);
		~DynamicMatrix();
		DynamicMatrix<DT>& operator=(const DynamicMatrix<DT>&);
		operator DT*() {return (DT*)data;}
		operator DT**() {return (DT**)data;}
		DT* toArray() {return (DT*)data;}
		DT& operator() (int, int);
		DynamicArray<DT>& operator[] (int);
		const DynamicArray<DT>& operator[] (int) const;
		void setAll(const DT&);
	};

}

#endif
