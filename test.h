// requires log.h
// these macros are only to be used in testing methods
// testing methods should take no arguments and return a bool (pass/fail)
extern int indent;
#define TEST(fcn) {log("%*sTesting '" #fcn "'\n",++indent,""); if(fcn() == false){ERROR("%*sError during '" #fcn "'\n",indent--,""); return false;}else{log("%*sPass.\n",indent--,"");}}
#define TEST_ASSERT(expr) if(false == (expr)){ERROR("Failed assertion"); return false;}
#define TEST_SUCCESS (true)
#define TEST_FAIL (false)
// test
bool test_utils();
