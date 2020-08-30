/*****************************************************************************************************
 * Subject                   : Safe Array Class                                                      *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"

#include "array.h"

 namespace std {

	template<class DT, size_t m> Array<DT, m>::Array(DT v) {
		for(int i=0; i<m; i++)
			this->data[i]=v;
	}
	template<class DT, size_t m> Array<DT, m>::Array(DT* data) {
		for(int i=0; i<m; i++)
			this->data[i]=data[i];
	}
	template<class DT, size_t m> Array<DT, m>::Array(const Array<DT, m>& arr) {
		for(int i=0; i<m; i++)
			this->data[i]=arr.data[i];
	}
	template<class DT, size_t m> DT& Array<DT, m>::operator[] (int i) {
		if(i<0 || i>=m) 
			throw ArrayIndexOutOfBound(i, 0, m);
		else
			return data[i];
	}
	template<class DT, size_t m> const DT& Array<DT, m>::operator[] (int i) const {
		if(i<0 || i>=m) 
			throw ArrayIndexOutOfBound(i, 0, m);
		else
			return data[i];
	}
	template<class DT, size_t m> Array<DT, m>::operator DT* () {
		return data;
	}
	template<class DT, size_t m> size_t Array<DT, m>::length() {
		return m;
	}
	template<class DT, size_t m> void Array<DT, m>::sort(int (*comparator)(DT&, DT&, void*), void* param) {
		quicksort(data, m, comparator, param);
	}

	template<class DT, size_t m, size_t n> Matrix<DT, m, n>::Matrix(DT v) {
		for(int i=0; i<m; i++)
			for(int j=0; j<n; j++)
				this->data[i][j]=v;
	}
	template<class DT, size_t m, size_t n> Matrix<DT, m, n>::Matrix(DT* data) {
		for(int i=0; i<m; i++)
			for(int j=0; j<n; j++)
				this->data[i][j]=data[i*n+j];
	}
	template<class DT, size_t m, size_t n> Matrix<DT, m, n>::Matrix(DT** data) {
		for(int i=0; i<m; i++)
			for(int j=0; j<n; j++)
				this->data[i][j]=data[i][j];
	}
	template<class DT, size_t m, size_t n> DT& Matrix<DT, m, n>::operator() (int i, int j) {
		if(i<0 || i>=m) 
			throw ArrayIndexOutOfBound(i, 0, m);
		else if(j<0 || j>=n)
			throw ArrayIndexOutOfBound(j, 0, n);
		else
			return data[i][j];
	}
	template<class DT, size_t m, size_t n> Array<DT, m>& Matrix<DT, m, n>::operator[] (int i) {
		if(i<0 || i>=m) 
			throw ArrayIndexOutOfBound(i, 0, m);
		else
			return data[i];
	}
	template<class DT, size_t m, size_t n> const Array<DT, m>& Matrix<DT, m, n>::operator[] (int i) const {
		if(i<0 || i>=m) 
			throw ArrayIndexOutOfBound(i, 0, m);
		else
			return data[i];
	}
	
	template<class DT> DynamicArray<DT>::DynamicArray(int n) {
		this->n=n;
		data=new DT[n];
	}
	template<class DT> DynamicArray<DT>::DynamicArray(int n, DT* data) {
		this->n=n;
		data=new DT[n];
		for(int i=0; i<n; i++)
			this->data[i]=data[i];
	}
	template<class DT> DynamicArray<DT>::DynamicArray(const DynamicArray<DT>& arr) {
		this->n=arr.n;
		data=new DT[n];
		for(int i=0; i<n; i++)
			this->data[i]=arr.data[i];
	}
	template<class DT> DynamicArray<DT>::~DynamicArray() {
		if(data!=NULL)
			delete[] data;
	}
	template<class DT> DynamicArray<DT>& DynamicArray<DT>::operator=(const DynamicArray<DT>& arr) {
		if(data==NULL)
			delete[] data;
		this->n=arr.n;
		data=new DT[n];
		for(int i=0; i<n; i++)
			this->data[i]=arr.data[i];
		return *this;
	}
	template<class DT> DT& DynamicArray<DT>::operator[] (int i) {
		if(i<0 || i>=n) 
			throw ArrayIndexOutOfBound(i, 0, n);
		else
			return data[i];
	}
	template<class DT> const DT& DynamicArray<DT>::operator[] (int i) const {
		if(i<0 || i>=n) 
			throw ArrayIndexOutOfBound(i, 0, n);
		else
			return data[i];
	}
	template<class DT> DynamicArray<DT>::operator DT* () {
		return data;
	}
	template<class DT> size_t DynamicArray<DT>::length() const {
		return n;
	}
	template<class DT> void DynamicArray<DT>::setAll(const DT& v) {
		for(int i=0; i<n; i++)
			data[i]=v;
	}

	template<class DT> DynamicMatrix<DT>::DynamicMatrix(int n, int m) {
		this->m=m;
		this->n=n;
		data=new DynamicArray<DT>[m];
		for(int i=0; i<m; i++)
			data[i]=DynamicArray<DT>(n);
	}
	template<class DT> DynamicMatrix<DT>::DynamicMatrix(int n, int m, DT* data) {
		this->m=m;
		this->n=n;
		data=new DynamicArray<DT>[m];
		for(int i=0; i<m; i++)
			data[i]=DynamicArray<DT>(n);
		for(int i=0; i<m; i++)
			for(int j=0; j<n; j++)
				this->data[i][j]=data[i*n+j];
	}
	template<class DT> DynamicMatrix<DT>::DynamicMatrix(int n, int m, DT** data) {
		this->m=m;
		this->n=n;
		data=new DynamicArray<DT>[m];
		for(int i=0; i<m; i++)
			data[i]=DynamicArray<DT>(n);
		for(int i=0; i<m; i++)
			for(int j=0; j<n; j++)
				this->data[i][j]=data[i][j];
	}
	template<class DT> DynamicMatrix<DT>::DynamicMatrix(const DynamicMatrix<DT>& mat) {
		this->m=mat.m;
		this->n=mat.n;
		data=new DynamicArray<DT>[m];
		for(int i=0; i<m; i++)
			data[i]=DynamicArray<DT>(n);
		for(int i=0; i<m; i++)
			for(int j=0; j<n; j++)
				this->data[i][j]=mat.data[i][j];
	}
	template<class DT> DynamicMatrix<DT>::~DynamicMatrix() {
		if(data!=NULL)
			delete[] data;
	}
	template<class DT> DynamicMatrix<DT>& DynamicMatrix<DT>::operator=(const DynamicMatrix<DT>& mat) {
		if(data!=NULL)
			delete[] data;
		this->m=mat.m;
		this->n=mat.n;
		data=new DynamicArray<DT>[m];
		for(int i=0; i<m; i++)
			data[i]=DynamicArray<DT>(n);
		for(int i=0; i<m; i++)
			for(int j=0; j<n; j++)
				this->data[i][j]=mat.data[i][j];
		return *this;
	}
	template<class DT> DT& DynamicMatrix<DT>::operator() (int i, int j) {
		if(i<0 || i>=m) 
			throw ArrayIndexOutOfBound(i, 0, m);
		else if(j<0 || j>=n)
			throw ArrayIndexOutOfBound(j, 0, n);
		else
			return data[i][j];
	}
	template<class DT> DynamicArray<DT>& DynamicMatrix<DT>::operator[] (int i) {
		if(i<0 || i>=m) 
			throw ArrayIndexOutOfBound(i, 0, m);
		else
			return data[i];
	}
	template<class DT> const DynamicArray<DT>& DynamicMatrix<DT>::operator[] (int i) const {
		if(i<0 || i>=m) 
			throw ArrayIndexOutOfBound(i, 0, m);
		else
			return data[i];
	}
	template<class DT> void DynamicMatrix<DT>::setAll(const DT& v) {
		for(int i=0; i<n; i++)
			data[i].setAll(v);
	}
}
