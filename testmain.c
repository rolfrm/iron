// Test Section
//#include <unistd.h>
#include "full.h"
#include "stdio.h"
bool test_util_hash_table(){
  hash_table * ht = ht_create(128,sizeof(u64),sizeof(u64));
  u64 cnt = 200;
  bool ok = true;
  for(int j = 0; j < 5; j++){ // a wash

    for(u64 i = 0; i < cnt;i++){
      u64 v = i * 20;
      ht_insert(ht,&i,&v);
    }
    
    for(u64 i = 0; i < cnt;i++){
      u64 v = i * 20;
      u64 _i = i;
      u64 v2 = *(u64 *)ht_lookup(ht,&_i);
      ok &= v == v2; 
    }

    for(u64 i = 0; i < cnt;i++){
      ht_remove(ht,&i);
    }

  
    for(u64 i = 0; i < cnt;i++){
      void * k = ht_lookup(ht,&i);
      ok &= k == NULL;
    }
  }
  return ok;
}

bool test_jump_consistent_hash(){
  i32 k = jump_consistent_hash(132132, 57);
  i32 k2 = jump_consistent_hash_str("hello world!!!", 128);
  u64 d1 = -1;
  i32 k3 = jump_consistent_hash_raw(&d1,sizeof(d1),128);
  log("%i %i %i\n",k, k2,k3);
  return true;
}

bool test_math_utils(){
  bool ok = true;
  i64 vals[] = {1,2,3};
  ok &= 6 == sum64(vals,array_count(vals));
  return ok;
}

bool test_local_expressions(){
  int result =  ({int x(){return 10;}x();});
  int z = ({int d = 4; d + 9;});
  TEST_ASSERT(13 == z);
  TEST_ASSERT(10 == result);
  return TEST_SUCCESS;
}

bool test_utils(){
  TEST(test_local_expressions);
  bool ok = true;
  ok &= test_math_utils();
  logd("Test count\n");
  i64 items[] = {1,5,10,5,321,5, -5};
  u64 fives = 3;
  u64 sum = 1 + 5 + 10 + 5 + 321 + 5 - 5;
  
  bool count_fives(const void * item){
    return *((i64*) item) == 5;
  }
  ok &= fives == count(items, array_count(items), sizeof(i64), &count_fives);
  
  logd("Test sum\n");
  ok &= sum == (u64) sum64(items,array_count(items));

  logd("Test timestamp\n");
  u64 start = timestamp();
  u64 wait = 100000;
  //struct timespec tim;
  // tim.tv_sec  = 0;
  //tim.tv_nsec = (wait - 1000) * 1000;

  //  nanosleep(tim);
  iron_usleep(wait-1000); //why is this needed?
  u64 stop = timestamp();
  ok &= ABS(stop - start - wait) < 2000; //wont be exactly precise

  logd("Test hibit\n");
  u64 smallval = 1024;
  u64 bigvalue = smallval * smallval * smallval * smallval;
  logd("bigvalue hibig: %i %llu\n", hibit(bigvalue), bigvalue);
  ok &= hibit(1) == 1 && hibit(bigvalue) == 41;

  TEST(test_jump_consistent_hash);
  return true;
}

bool mem_test(){
  allocator * ta = trace_allocator_make();  
  int * data;
  int * data2;
  with_allocator(ta,lambda(void,(){data = alloc(1024 * sizeof(int));}));
  with_allocator(ta,lambda(void,(){data2 = alloc(1024 * sizeof(int));}));
  with_allocator(ta,lambda(void,(){data = ralloc(data, 1024 * sizeof(int));}));
  for(int i = 0; i < 1024;i++){
    data[i] = i;
  }
  TEST_ASSERT(data[512] == 512);
  TEST_ASSERT(trace_allocator_allocated_pointers(ta) == 2);
  with_allocator(ta,lambda(void,(){dealloc(data);}));
  with_allocator(ta,lambda(void,(){dealloc(data2);}));
  TEST_ASSERT(trace_allocator_allocated_pointers(ta) == 0);  

  char * r = fmtstr("1%s2","hello");
  TEST_ASSERT(strcmp(r,"1hello2") == 0);
  dealloc(r);
  return TEST_SUCCESS;
}

bool test_reallocation(){
  static __thread bool is_ongoing = false;
  bool start_new = !is_ongoing;
  is_ongoing = true;
  void * ptr = NULL;
  void * heap_dis = alloc(100);
  for(int z = 0 ; z < 2; z++){
    for(int i = 0 ; i < 23; i += 3){
      ptr = ralloc(ptr, 1 << i);
      void * last = ptr;
      for(int j = 0; j < 5; j++){
	ptr = ralloc(ptr, 1 << i);
	dealloc(heap_dis);
	heap_dis = alloc(100);
	TEST_ASSERT(ptr == last);
	ptr = last;
	if(start_new)
	  test_reallocation();
      }
    }
  }
  dealloc(ptr);
  dealloc(heap_dis);
  if(start_new)
    is_ongoing = false;
  return TEST_SUCCESS;
}

bool test_hibit(){
  for(int i = 0; i < 100; i++){
    hibit(i);
  }
  return TEST_SUCCESS;
}

void bench_list_add(size_t icnt){
  void * ptr = NULL;
  size_t cnt = 0;
  size_t item_size = sizeof(int);
  for(size_t i = 0; i < icnt; i++){
    list_add(&ptr,&cnt,&i,item_size);
  }

  list_remove(&ptr,&cnt,2,item_size);
  list_remove(&ptr,&cnt,1,item_size);
  list_remove(&ptr,&cnt,0,item_size);
  size_t s1 = cnt;
  list_remove(&ptr,&cnt,2,item_size);
  list_remove(&ptr,&cnt,2,item_size);
  list_remove(&ptr,&cnt,2,item_size);
  list_remove(&ptr,&cnt,2,item_size);
  size_t s2 = cnt;
  logd("s2 %i, s1 %i\n", s2, s1);
  ASSERT(s2 == (s1 - 4));
  int * items = (int *) ptr;
  ASSERT(items[0] == 3);
  logd("cnt: %i\n", cnt);
  list_remove(&ptr,&cnt,4,item_size);
  list_remove(&ptr,&cnt,6,item_size);
  list_remove(&ptr,&cnt,16,item_size);
  logd("cnt: %i\n", cnt);
  items = (int *) ptr;
  for(size_t j = 0; j < cnt; j++)
    logd("i: %p %p\n", items[j], cnt);
  dealloc(ptr);
}

bool bench_list_add_test(){
  bench_list_add(100);
  return TEST_SUCCESS;
}

bool test_list(){
  size_t cnt = 1000000;
  u64 fast = measure_elapsed(lambda( void, (){bench_list_add(cnt);}));
  logd("listadd: %i\n", fast);
  return TEST_SUCCESS;
}


bool do_allocator_test(){
  size_t tstcnt = 20;
  i64 * ptrs[tstcnt]; 
  for(size_t i = 0; i < tstcnt; i++){
    ptrs[i] = alloc(tstcnt * sizeof(i64));
    i64 * ptr = ptrs[i];
    for(size_t k = 0; k < tstcnt ; k++)
      ptr[k] = i * tstcnt + k;
  }
  for(size_t i = 0; i < tstcnt*tstcnt; i++)
    ASSERT(ptrs[i / tstcnt][i % tstcnt] == (i64)i);
  for(size_t i = 0; i < tstcnt; i++){
    ptrs[i] = ralloc(ptrs[i],tstcnt * sizeof(i64));
  }
  for(size_t i = 0; i < tstcnt; i++){
    dealloc(ptrs[i]);
    ptrs[i] = NULL;
  }
  return TEST_SUCCESS;
}

bool block_allocator_test(){
  bool ok = TEST_SUCCESS;
  for(int j = 0; j < 10; j++){
    allocator * balloc = block_allocator_make();
    with_allocator(balloc,lambda(void, (){
	  ok &= do_allocator_test();
	}));
    block_allocator_release(balloc);
  }
  
  return ok;
}

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
void _error(const char * file, int line, const char * msg, ...){
  loge("Got error at %s line %i\n", file,line);
  log("Error message *****\n");
  va_list args;
  va_start (args, msg);
  vprintf (msg, args);
  va_end (args);
  log("*******************\n");
  exit(255);
}

int main(){


  TEST(test_hibit);
  TEST(test_list);
  TEST(test_reallocation);
  TEST(mem_test);
  TEST(test_math_utils);
  TEST(test_local_expressions);
  TEST(test_utils);
  TEST(test_util_hash_table);
  TEST(bench_list_add_test);
  TEST(block_allocator_test);
  log("TEST SUCCESS\n");
}
