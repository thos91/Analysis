// system includes
#include <iostream>
#include <exception>

// user includes
#include "ContiguousVectors.h"
#include "wgConst.hpp"

#define TEST_SIZE1 100
#define TEST_SIZE2 200
#define TEST_SIZE3 300

int main() {
  d3vector test1(TEST_SIZE1, TEST_SIZE2, TEST_SIZE3);

  try {
	test1.Initialize(TEST_SIZE1, TEST_SIZE2, TEST_SIZE3);
  }
  catch(const exception &e) {
	std::cout << "GOOD! Trying to reinitialized should raise an exception" << std::endl;
  }
  
  test1.fill(0);

  std::cout << "test1.data() = " << test1.data() << std::endl;
  std::cout << "test1[0].data() = " << test1[0].data() << std::endl;

  if ( test1.size() != TEST_SIZE1 )
	std::cout << "test1.size() != " << TEST_SIZE1 << std::endl;

  if ( test1[0].size() != TEST_SIZE2 )
	std::cout << "test1[0].size() != " << TEST_SIZE2 << std::endl;

  if ( test1[0][0].size() != TEST_SIZE3 )
	std::cout << "test1[0][0].size() != " << TEST_SIZE3 << std::endl;
  
  for (int i = 0; i < TEST_SIZE1; i++) {
	for (int j = 0; j < TEST_SIZE2; j++) {
	  for (int k = 0; k < TEST_SIZE3; k++) {
		if ( test1[i][j][k] != 0 )
		  std::cout << "test1["<<i<<"]["<<j<<"]["<<k<<"] != 0" << std::endl;
	  }
	}
  }
  
  d3vector test2;

  try {
	test2.Initialize(TEST_SIZE1, TEST_SIZE2, TEST_SIZE3);
  }
  catch(const exception &e) {
	std::cout << "NO GOOD! This initialization should not throw any exception" << std::endl;
  }

  test2.fill(0);

  std::cout << "test2.data() = " << test2.data() << std::endl;
  std::cout << "test2[0].data() = " << test2[0].data() << std::endl;

  if ( test2.size() != TEST_SIZE1 )
	std::cout << "test2.size() != " << TEST_SIZE1 << std::endl;

  if ( test2[0].size() != TEST_SIZE2 )
	std::cout << "test2[0].size() != " << TEST_SIZE2 << std::endl;

  if ( test2[0][0].size() != TEST_SIZE3 )
	std::cout << "test2[0][0].size() != " << TEST_SIZE3 << std::endl;
  
  for (int i = 0; i < TEST_SIZE1; i++) {
	for (int j = 0; j < TEST_SIZE2; j++) {
	  for (int k = 0; k < TEST_SIZE3; k++) {
		if ( test2[i][j][k] != 0 )
		  std::cout << "test2["<<i<<"]["<<j<<"]["<<k<<"] != 0" << std::endl;
	  }
	}
  }

  i3vector test3(TEST_SIZE1, TEST_SIZE2, TEST_SIZE3);

  try {
	test3.Initialize(TEST_SIZE1, TEST_SIZE2, TEST_SIZE3);
  }
  catch(const exception &e) {
	std::cout << "GOOD! Trying to reinitialized should raise an exception" << std::endl;
  }
  
  test3.fill(0);

  std::cout << "test3.data() = " << test3.data() << std::endl;
  std::cout << "test3[0].data() = " << test3[0].data() << std::endl;

  if ( test3.size() != TEST_SIZE1 )
	std::cout << "test3.size() != " << TEST_SIZE1 << std::endl;

  if ( test3[0].size() != TEST_SIZE2 )
	std::cout << "test3[0].size() != " << TEST_SIZE2 << std::endl;

  if ( test3[0][0].size() != TEST_SIZE3 )
	std::cout << "test3[0][0].size() != " << TEST_SIZE3 << std::endl;
  
  for (int i = 0; i < TEST_SIZE1; i++) {
	for (int j = 0; j < TEST_SIZE2; j++) {
	  for (int k = 0; k < TEST_SIZE3; k++) {
		if ( test3[i][j][k] != 0 )
		  std::cout << "test3["<<i<<"]["<<j<<"]["<<k<<"] != 0" << std::endl;
	  }
	}
  }
  
  exit(0);
}
