// requires log.h
// these macros are only to be used in testing methods
// testing methods should take no arguments and return a bool (pass/fail)
#define TEST(fcn) {log("Testing '" #fcn "'\n"); if(fcn() == false){ERROR("Error during '" #fcn "'\n"); return false;}}
#define TEST_ASSERT(expr) if(false == (expr)){ERROR("Failed assertion"); return false;}
#define TEST_SUCCESS (true)
#define TEST_FAIL (false)
// test
bool test_utils();
