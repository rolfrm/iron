// requires log.h
// these macros are only to be used in testing methods
// testing methods should take no arguments and return a bool (pass/fail)

#define TEST(fcn) {log("%sTesting '" #fcn "'\n",""); if(fcn() == false){ERROR("%sError during '" #fcn "'\n",""); return false;}else{log("%sPass.\n","");}}
#define TEST1(fcn,a) {log("%sTesting '" #fcn "(" #a ")" "'\n",""); if(fcn(a) == false){ERROR("%sError during '" #fcn "'\n",""); return false;}else{log("%sPass.\n","");}}
#define TEST_ASSERT(expr) if(false == (expr)){ERROR("Failed assertion :'" #expr ";"); return false;}
#define TEST_ASSERT_EQUAL(expr1, expr2) if(expr1 != expr2){ERROR("Failed assertion :'" #expr1 " == " #expr2 "'\n"); return false;}
#define TEST_ASSERT_NOT_EQUAL(expr1, expr2) if(expr1 == expr2){ERROR("Failed assertion :'" #expr1 " != " #expr2 "'\n"); return false;}
#define TEST_ASSERT_STRING_EQUAL(s1, s2) if(strcmp(s1,s2) != 0){ERROR("Failed assertion: strcmp(\"%s\", \"%s\") == 0", s1, s2);}
#define TEST_SUCCESS (true)
#define TEST_FAIL (false)

#define BENCH(fcn) {						\
    log("Benchmarking '" #fcn "':");				\
    u64 t1 = timestamp();					\
    fcn();							\
    u64 t2 = timestamp();					\
    log("  %ius\n", t2 - t1);			\
  }								\


const char * get_test_opt(const char * name);
void set_test_opt(const char ** args, int arg_cnt);

// test
bool test_utils(void);
