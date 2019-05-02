#ifndef CONTIGUOUS_VECTORS_H_
#define CONTIGUOUS_VECTORS_H_

#include <cstddef>
#include <vector>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include "wgExceptions.h"

/****************************** WRAP VECTORS ********************************/

template <class T>
void wrapArrayInVector( T *sourceArray, size_t arraySize, std::vector<T, std::allocator<T> > &targetVector ) {
  typename std::_Vector_base<T, std::allocator<T> >::_Vector_impl *vectorPtr =
    (typename std::_Vector_base<T, std::allocator<T> >::_Vector_impl *)((void *) &targetVector);
  vectorPtr->_M_start = sourceArray;
  vectorPtr->_M_finish = vectorPtr->_M_end_of_storage = vectorPtr->_M_start + arraySize;
}

template <class T>
void releaseVectorWrapper( std::vector<T, std::allocator<T> > &targetVector ) {
  typename std::_Vector_base<T, std::allocator<T> >::_Vector_impl *vectorPtr =
        (typename std::_Vector_base<T, std::allocator<T> >::_Vector_impl *)((void *) &targetVector);
  vectorPtr->_M_start = vectorPtr->_M_finish = vectorPtr->_M_end_of_storage = NULL;
}

/****************************** 3D VECTORS ********************************/

template <class T>
class Contiguous3Vector {

private:
  T*** m_array_outer;
  std::size_t m_size_outer;
  std::size_t m_size_middle;
  std::size_t m_size_inner;

public:
  Contiguous3Vector() {
	m_array_outer = NULL;
	m_size_outer = 0;
	m_size_middle = 0;
	m_size_inner = 0;
  }

  Contiguous3Vector(std::size_t x_size_outer, std::size_t x_size_middle, std::size_t x_size_inner) {
	this->Initialize(x_size_outer, x_size_middle, x_size_inner);
  }

  void Initialize(std::size_t x_size_outer, std::size_t x_size_middle, std::size_t x_size_inner) {
	if (m_array_outer == NULL) {
	  if ( x_size_outer * x_size_middle * x_size_inner == 0 )
		throw std::invalid_argument("minimum dimension is 1");
	  m_size_outer = x_size_outer;
	  m_size_middle = x_size_middle;
	  m_size_inner = x_size_inner;
	  m_array_outer = new T** [x_size_outer];
	  m_array_outer[0] = new T* [x_size_outer * x_size_middle];
	  m_array_outer[0][0] = new T[x_size_outer * x_size_middle * x_size_inner]();
	  for (std::size_t i = 0; i < x_size_outer; i++) {
		if (i > 0) {
		  m_array_outer[i] = m_array_outer[i-1] + x_size_middle;
		  m_array_outer[i][0] = m_array_outer[i-1][0] + (x_size_middle*x_size_inner);
		}
		for (std::size_t j = 1; j < x_size_middle; j++) {
		  m_array_outer[i][j] = m_array_outer[i][j-1] + x_size_inner;
		}
	  }
	}
	else throw std::runtime_error("contiguous vector already initialized");
  }

  /***************************** PROXY MIDDLE ***************************************/
  class ProxyMiddle {                                                               //
  protected:                                                                        //
	T **m_array_middle;                                                             //
	std::size_t m_size_middle;                                                      //
	std::size_t m_size_inner;                                                       //
  public:                                                                           //
	ProxyMiddle(T **x_array_middle, std::size_t x_size_middle, std::size_t x_size_inner) {
	  m_array_middle = x_array_middle;                                              //
	  m_size_middle = x_size_middle;                                                //
	  m_size_inner = x_size_inner;                                                  //
	}                                                                               //
                                                                                    //
	/*************************** PROXY INNER **************************/            //
	class ProxyInner {                                                //            //
	public:                                                           //            //
	  ProxyInner(T* x_array_inner, std::size_t x_size_inner) {        //            //
		m_array_inner = x_array_inner;                                //            //
		m_size_inner = x_size_inner;                                  //            //
	  }                                                               //            //
                                                                      //            //
	  // get                                                          //            //
	  const T & operator[](std::size_t index) const {                 //            //
		if (m_array_inner == NULL)                                    //            //
		  throw wgNotInitialized("inner array not initialized");      //            //
		else if (index >= m_size_inner)                               //            //
		  throw std::out_of_range("inner index " + std::to_string(index) +          //
								  " out of range " + std::to_string(m_size_inner)); //
		else return m_array_inner[index];                             //            //
	  }                                                               //            //
                                                                      //            //
	  // set                                                          //            //
	  T & operator [](std::size_t index) {                            //            //
		if (m_array_inner == NULL)                                    //            //
		  throw wgNotInitialized("inner array not initialized");      //            //
		else if (index >= m_size_inner)                               //            //
		  throw std::out_of_range("inner index " + std::to_string(index) +          //
								  " out of range " + std::to_string(m_size_inner)); //
		else return m_array_inner[index];                             //            //
	  }                                                               //            //
                                                                      //            //
	  std::size_t size() {                                            //            //
		return m_size_inner;                                          //            //
	  }                                                               //            //
                                                                      //            //
	  T * data() {                                                    //            //
		return m_array_inner;                                         //            //
	  }                                                               //            //
	                                                                  //            //
	protected:                                                        //            //
	  T* m_array_inner;                                               //            //
	  std::size_t m_size_inner;                                       //            //
	};                                                                //            //
	/******************************************************************/            //
                                                                                    //
	ProxyInner operator[](std::size_t index) {                                      //
	  if (m_array_middle == NULL)                                                   //
		throw wgNotInitialized("middle array not initialized");                     //
	  else if (index >= m_size_middle)                                              //
		throw std::out_of_range("middle index " + std::to_string(index) +           //
								" out of range " + std::to_string(m_size_middle));  //
	  else return ProxyInner(m_array_middle[index], m_size_inner);                  //
	}                                                                               //
                                                                                    //
	T * data() {                                                                    //
	  if (m_array_middle == NULL) return NULL;                                      //
	  else return m_array_middle[0];                                                //
	}                                                                               //
	                                                                                //
	std::size_t size() {                                                            //
	  return m_size_middle;                                                         //
	}                                                                               //
	                                                                                //
  };                                                                                //
  /**********************************************************************************/

  ProxyMiddle operator[](std::size_t index) {
	if (m_array_outer == NULL)
	  throw wgNotInitialized("outer array not initialized");
	else if (index >= m_size_outer)
	  throw std::out_of_range("outer index " + std::to_string(index) +
							  " out of range " + std::to_string(m_size_outer));
	else return ProxyMiddle(m_array_outer[index], m_size_middle, m_size_inner);
  }

  void fill(T value) {
	if (m_array_outer == NULL)
	  throw wgNotInitialized("outer array not initialized");
	else {
	  for (std::size_t i = 0; i < m_size_outer; i++) {
		if (m_array_outer[i] == NULL)
		  throw wgNotInitialized("middle array not initialized");
		for (std::size_t j = 0; j < m_size_middle; j++) {
		  if (m_array_outer[i][j] == NULL)
			throw wgNotInitialized("inner array not initialized");
		  for (std::size_t k = 0; k < m_size_inner; k++) {
			m_array_outer[i][j][k] = value;
		  }
		}
	  }
	}
  }

  std::size_t size() {
	return m_size_outer;
  }

  T * data() {
	if (m_array_outer == NULL) return NULL;
	else if (m_array_outer[0] == NULL) return NULL;
	else return m_array_outer[0][0];
  }

  ~Contiguous3Vector() {
	delete [] m_array_outer[0][0];
	delete [] m_array_outer[0];
	delete [] m_array_outer;
  }
};

/****************************** 2D VECTORS ********************************/

template <class T>
class Contiguous2Vector {

private:
  T** m_array_outer;
  std::size_t m_size_outer;
  std::size_t m_size_inner;

public:
  Contiguous2Vector() {
  	m_array_outer = NULL;
	m_size_outer = 0;
	m_size_inner = 0;
  }

  Contiguous2Vector(std::size_t x_size_outer, std::size_t x_size_inner) {
	this->Initialize(x_size_outer, x_size_inner);
  }

  void Initialize(std::size_t x_size_outer, std::size_t x_size_inner) {
	if (m_array_outer == NULL) {
	  if ( x_size_outer * x_size_inner == 0 )
		throw std::invalid_argument("minimum dimension is 1");
	  m_size_outer = x_size_outer;
	  m_size_inner = x_size_inner;
	  m_array_outer = new T* [x_size_outer];
	  m_array_outer[0] = new T [x_size_outer * x_size_inner]();
	  for (std::size_t i = 1; i < x_size_outer;  i++) {
		m_array_outer[i] = m_array_outer[i-1] + x_size_inner;
	  }
	}
	else throw std::runtime_error("contiguous vector already initialized");
  }

  /*************************** PROXY ********************************/
  class Proxy {                                                     //
  public:                                                           //
	Proxy(T* _array_inner, std::size_t x_size_inner) {              //
	  m_array_inner = _array_inner;                                 //
	  m_size_inner = x_size_inner;                                  //
	}                                                               //
                                                                    //
	// get                                                          //
	const T & operator[](std::size_t index) const {                 //
	  if (m_array_inner == NULL)                                    //
		throw wgNotInitialized("inner array not initialized");      //
	  else if (index >= m_size_inner)                               //
		throw std::out_of_range("inner index " + std::to_string(index) +
								" out of range " + std::to_string(m_size_inner));
	  else return m_array_inner[index];                             //
	}                                                               //
                                                                    //
	// set                                                          //
	T & operator [](std::size_t index) {                            //
	  if (m_array_inner == NULL)                                    //
		throw wgNotInitialized("inner array not initialized");      //
	  else if (index >= m_size_inner)                               //
		throw std::out_of_range("inner index " + std::to_string(index) +
								" out of range " + std::to_string(m_size_inner));
	  else return m_array_inner[index];                             //
	}                                                               //
                                                                    //
	std::size_t size() {                                            //
	  return m_size_inner;                                          //
	}                                                               //
                                                                    //
	T * data() {                                                    //
	  return m_array_inner;                                         //
	}                                                               //
                                                                    //
  protected:                                                        //
	T* m_array_inner;                                               //
	std::size_t m_size_inner;                                       //
  };                                                                //
  /******************************************************************/

  Proxy operator[](std::size_t index) {
	if (m_array_outer == NULL)
	  throw wgNotInitialized("outer array not initialized");
	else if (index >= m_size_outer)
	  throw std::out_of_range("outer index " + std::to_string(index) +
							  " out of range " + std::to_string(m_size_outer));
	else return Proxy(m_array_outer[index], m_size_inner);
  }

  std::size_t size() {
	return m_size_outer;
  }

  T * data() {
	if (m_array_outer == NULL) return NULL;
	return m_array_outer[0];
  }

  void fill(T value) {
	if (m_array_outer == NULL)
	  throw wgNotInitialized("outer array not initialized");
	else {
	  for (std::size_t i = 0; i < m_size_outer; i++) {
		if (m_array_outer[i] == NULL)
		  throw wgNotInitialized("inner array not initialized");
		for (std::size_t k = 0; k < m_size_inner; k++) {
		  m_array_outer[i][k] = value;
		}
	  }
	}
  }

  ~Contiguous2Vector() {
	delete [] m_array_outer[0];
	delete [] m_array_outer;
  }
};

#endif /* CONTIGUOUS_VECTORS_H_ */
