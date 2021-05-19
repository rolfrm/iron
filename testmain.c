// Test Section
//#include <unistd.h>
#include "full.h"

#include "gl.h"
#include "stdio.h"
#include <unistd.h>
bool test_util_hash_table(){
  hash_table * ht = ht_create2(128,sizeof(u64),sizeof(u64));
  u64 cnt = 200;
  for(int j = 0; j < 5; j++){ // a wash

    for(u64 i = 0; i < cnt;i++){
      u64 v = i * 20;
      ht_set(ht,&i,&v);
    }
    
    for(u64 i = 0; i < cnt;i++){
      u64 v = i * 20;
      u64 _i = i;
      u64 v2 = 0;
      ASSERT(ht_get(ht,&_i, &v2));
      ASSERT(v == v2); 
    }

    for(u64 i = 0; i < cnt;i++){
      ht_remove(ht,&i);
    }
  
    for(u64 i = 0; i < cnt;i++){
      void * k = NULL;
      ASSERT(ht_get(ht,&i, &k) == false);
      ASSERT(k == NULL);
    }
  }
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
  ok &= ABS(((i64)(stop - start - wait))) < 2000; //wont be exactly precise

  logd("Test hibit\n");
  u64 smallval = 1024;
  u64 bigvalue = smallval * smallval * smallval * smallval;
  logd("bigvalue hibig: %i %llu\n", hibit(bigvalue), bigvalue);
  ok &= hibit(1) == 1 && hibit(bigvalue) == 41;

  //TEST(test_jump_consistent_hash);
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
  const char * r2 = quickfmt("1%s2","hello");
  TEST_ASSERT(strcmp(r,r2) == 0);
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
  size_t cnt = 100;
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

/*
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
  }*/

bool strtest(){
  { // split / join test
    char * testbuffer = "1:2:3  5:5:6";
    int cnt;
    char ** buffers = string_split(testbuffer, ":", &cnt);
    
    TEST_ASSERT(strlen(buffers[0]) == 1);
    TEST_ASSERT(strlen(buffers[2]) == 4);
    TEST_ASSERT(cnt == 5);
    char * testbuffer2 = string_join(cnt, ":", buffers);
    ASSERT(strcmp(testbuffer, testbuffer2) == 0);
  }
  
  { // replace inplace test
    char * str = fmtstr("%s", "        |REPLACE_THIS| |REPLACE_THIS|    |REPLACE_THIS||REPLACE_THIS||REPLACE_THIS|    ");
    char * str_bak = fmtstr("%s", str); // backup
    str = realloc(str, 1000);
    replace_inplace(str, "REPLACE_THIS", ".");

    ASSERT(strcmp(str, "        |.| |.|    |.||.||.|    ") == 0);
    replace_inplace(str, ".", "REPLACE_THIS");
    ASSERT(strcmp(str, str_bak) == 0);

    dealloc(str);
    dealloc(str_bak);
  }

  {
    logd("testing string commands\n");
    TEST_ASSERT(string_startswith("hello world", "hell"));
    TEST_ASSERT(string_startswith("hello world", "hello "));
    TEST_ASSERT(string_startswith("hello world", "hello  ") == false);

    TEST_ASSERT(string_skip("hello world", "hello")[0] == ' ');
    TEST_ASSERT(string_skip("hello world", "hello  ") == NULL);
    TEST_ASSERT(string_skip_all(string_skip("hello    world", "hello"), " ")[0] == 'w');
    TEST_ASSERT(string_skip_all(string_skip("helloworld", "hello"), " ")[0] == 'w');
    TEST_ASSERT(string_skip_all(string_skip("helworld", "hello"), " ") == NULL);
    
  }
  

  return TEST_SUCCESS;
}

bool test_mutex(){

    iron_mutex mtex = iron_mutex_create();
    iron_mutex mtex2 = iron_mutex_create();
    iron_mutex_lock(mtex);
    int limit = 10000;
    int counter = 0;
    void _test_mutex(){
      for(int i = 0; i < limit; i++){
	iron_mutex_lock(mtex);
	iron_mutex_lock(mtex2);
	counter += 1;
	iron_mutex_unlock(mtex2);
	iron_mutex_unlock(mtex);
      }
    }
    int threads = 15;
    iron_thread * trds[threads];
    for(int i = 0; i < threads;i++)
      trds[i] = iron_start_thread0(_test_mutex);
    
    iron_mutex_unlock(mtex);
    for(int i = 0; i < threads;i++)
      iron_thread_join(trds[i]);

    logd("CNT: %i\n", counter);
    ASSERT(counter == limit * threads);
    iron_mutex_destroy(&mtex);


  return true;
}

static void listen(const data_stream * s, const void * data, size_t length, void * userdata){
  int * cnt = userdata;
  if((strcmp("Datastream 1", s->name) == 0) || strcmp("Datastream 2", s->name) == 0){
    *cnt += 1;
  }
  logd("%p  %s  '%s'\n", s, s->name, data);
}

static void listen_activity(const data_stream * s, const void * data, size_t length, void * userdata){
  logd("Activity: on '%s' (%p)\n", s->name, s);
}

bool test_datastream(){
  static data_stream str1 = { .name = "Datastream 1"};
  static data_stream str2 = { .name = "Datastream 2"};

  data_stream_listener * activity_listener = alloc0(sizeof(data_stream_listener));
  activity_listener->process = listen_activity;
  data_stream_listen_activity(activity_listener);
  data_stream_listener * l = alloc0(sizeof(data_stream_listener));
  l->process = listen;
  int * cnt = alloc0(sizeof(int));
  l->userdata = cnt;
  data_stream_listen_all(l);

  data_stream_listener * l2 = alloc0(sizeof(data_stream_listener));
  l2->process = listen;
  int * cnt2 = alloc0(sizeof(int));
  l2->userdata = cnt2;
  data_stream_listen(l2, &str1);
  
  {
    const char * msg = "test1";
    dlog(str1, msg, sizeof(msg));
  }
  
  {
    const char * msg = "test1 asddwa dsa dsa";
    dlog(str2, msg, sizeof(msg));
    dlog(str1, msg, sizeof(msg));
  }

  dmsg(str2, "hello %i %i %i: %s", 1,2 ,3, "...");
  ASSERT(*cnt == 4);
  ASSERT(*cnt2 == 2);
  data_stream_unlisten(l2, &str1);
  dmsg(str2, "hello %i %i %i: %s", 1,2 ,5, "...");
  ASSERT(*cnt2 == 2);
  data_stream_unlisten_all(l);
  dmsg(str2, "last... hello %i %i %i: %s", 1,2 ,3, "...");
  ASSERT(*cnt == 5);
  data_stream_listen_all(l);
  data_stream_listen(l2, &str1);
  return true;  
}

bool test_process(){

  return true;
// todo: Find bette way of testing
  //const char * args1[] = {"/usr/bin/xclip", "/proc/cpuinfo", 0};
  //const char * args1[] = {"/bin/cat", "/proc/cpuinfo", 0};
  //const char * args1[] = {"", NULL};
  //const char * args2[] = {"/usr/bin/xclip", "-se", "clipboard", "-target", "image/png", "-out", 0};
  //iron_process p;
  /*
  //iron_process_run("/usr/bin/xclip",args1 , &p)
  iron_process_run("./hello.sh",args1 , &p);
  //iron_process_run("/bin/cat", args1 , &p);
  //iron_process_run("/bin/sleep",args1 , &p);
  while(true){
    iron_process_wait(p, 100000);
    char buffer[1024] = {0};
    printf("Reading...\n");
    int rd = read(p.stdout_pipe, buffer, sizeof(buffer));
    printf("rd: %i\n", rd);
    if(rd == -1)
      break;
    if(rd == 0)
      break;
    printf("Buf: %s\n", buffer);
  }
  printf("stat: %i\n", iron_process_get_status(p));
  //iron_process_wait(p, 10000);
  iron_process_clean(&p);
    
*/
  /*  p = (iron_process){0};
  iron_process_run("/usr/bin/xclip",args2 , &p);
  while(true){
    iron_process_wait(p, 100000);
    char buffer[1024] = {0};
    int rd = read(p.stdout_pipe, buffer, sizeof(buffer));
    printf("rd: %i\n", rd);
    if(rd == -1 || rd == 0)
      break;
    printf("Buf: %s\n", buffer);
  }
  printf("stat: %i\n", iron_process_get_status(p));
  
  return true;*/
}
extern texture * font_tex;
#include "duck_img.png.c"
bool ht2_test();
bool ht2_string_test();
void hash_table_bench();
bool bf_test();
void do_bench(){
  BENCH(hash_table_bench);
}
int main(){
  
  TEST(ht2_test);
  TEST(ht2_string_test);
  
  TEST(test_util_hash_table);
  TEST(test_hibit);
  TEST(test_list);
  TEST(test_reallocation);
  TEST(mem_test);
  TEST(test_math_utils);
  TEST(test_local_expressions);
  TEST(test_utils);
  
  TEST(bench_list_add_test);
  TEST(strtest);
  //TEST(test_mutex);
  TEST(test_datastream);
  TEST(test_process);
  TEST(linmath_test);
  TEST(bf_test);
  //TEST(block_allocator_test);
  log("TEST SUCCESS\n");

  gl_window * w = gl_window_open(200,196);
  gl_window_make_current(w);
  image img = image_from_data(duck_png, duck_png_len);
  printf("duck: %i %i %i\n", img.width, img.height, img.channels);
  image img2 = image_from_file("duck.png");

  printf("duck: %i %i %i\n", img2.width, img2.height, img2.channels);
  texture duck_tex = texture_from_image(&img);

  gl_window_make_current(w);
  blit_framebuffer fbuf = {.width = 64, .height = 64}; 
  var blit3d = blit3d_context_new();
  var poly1 = blit3d_polygon_new();
  float vertexes[] = {0,0, 0, 1,0,0, 0,1,0};
  blit3d_polygon_load_data(poly1, vertexes, sizeof(vertexes));
  blit3d_polygon_configure(poly1, 3);

  {
    var poly1 = blit3d_polygon_new();
    float vertexes[] = {0,0, 0, 1,0,0, 0,1,0};
    blit3d_polygon_load_data(poly1, vertexes, sizeof(vertexes));
    blit3d_polygon_configure(poly1, 3);
    blit3d_polygon_destroy(&poly1);
  }

  
  blit_create_framebuffer(&fbuf);
  blit_use_framebuffer(&fbuf);
  blit_begin(BLIT_MODE_UNIT);
  blit(0,0,&duck_tex);
  blit_unuse_framebuffer(&fbuf);

  u8 checkered[] = {0, 255, 0, 255, 0,
		 255, 0, 255, 0, 255,
		 0, 255, 0, 255, 0,
		 255, 0, 255,0 , 255,
		 0, 255, 0, 255, 0};
  var checkered_image = image_from_bitmap(checkered, 5, 5, 1);
  //checkered_image.mode = GRAY_AS_ALPHA;
 
  var checkered_texture = texture_from_image2(&checkered_image, TEXTURE_INTERPOLATION_NEAREST);
  UNUSED(checkered_texture);
  const char * fontfile_init = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"; //DejaVuSansMono
  var fnt0 =  blit_load_font_file(fontfile_init,  20);
  blit_set_current_font(fnt0);
  const char * fontfile = "/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf"; //DejaVuSansMono
  var fnt = blit_load_font_file(fontfile, 32);
  gl_window_event events[10];
  for(int i = 0; i <30000 ;i++){
    gl_window_make_current(w);
    size_t event_count = gl_get_events(events, array_count(events));
    for(size_t i = 0; i < event_count; i++){
      printf("evt: %luu\n", events[i].type);
      switch(events[i].type){
      case EVT_KEY_DOWN:
	printf("Key: %i\n", events[i].key.scancode);
	break;
      }
    }

    blit_begin(BLIT_MODE_UNIT);
    blit_rectangle(-1,-1,2,2, 1,1,1,1);
    
    blit_blit_framebuffer(&fbuf);
    /*//blit_begin(BLIT_MODE_PIXEL);
    blit(0,0,&duck_tex);
    
    //blit(-15,0,&duck_tex);
    printf("%i\n", gl_window_get_key_state(w, KEY_CTRL));
    if(gl_window_get_key_state(w, KEY_CTRL)){
      var clipboard = gl_window_get_clipboard(w);
      if(clipboard)
	printf("%s\n", clipboard);
      iron_usleep(1000000);
      TEST(test_process);
      }*/
    blit3d_context_load(blit3d);
    
    blit3d_view(blit3d, mat4_rotate(mat4_identity(),0,0,1,0.1f * i));
    blit3d_color(blit3d, vec4_new(1, 1, 0,1));
    blit3d_polygon_blit(blit3d, poly1);
    blit_begin(BLIT_MODE_PIXEL_SCREEN);
    
    blit_color(0.2,0.9,0.2,1.0);
    char * lines1[] = {"It's a", "  DUCK!", "  DUCK!", "  DUCK!", "  DUCK!", "  DUCK!", 0};//, "", "I Dont give ", "  a duck!"};
    char * lines2[] =  {"It's a", "  DUCK!", "  DUCK!", "  DUCK!", "I Dont give ", "  a duck!", 0};
    char ** lines = lines1;
    
    bool a = gl_window_get_key_state(w, KEY_LEFT);
    if(a){
      lines = lines2;
    }


    float offsety = 0;
    blit_set_current_font(NULL);
    blit_set_current_font(fnt);
    for(int i = 0; i < 100; i++){
      if(lines[i] == 0) break;
      if(i > 2){
	blit_color(0.0,0.0,0.0,1.0);
      }
      blit_push();
      blit_translate(0, offsety);
      blit_text(lines[i]);
      blit_pop();
      offsety += blit_measure_text(lines[i]).y;
    }

    
    //blit_bind_texture(font_tex);
    //blit_bind_texture(&checkered_texture);
    //blit_quad();
    //vec2 meas_text = measure_text("Test", 4);

    gl_window_poll_events();
    gl_window_swap(w);
    iron_usleep(100000/3);
  }
  gl_window_destroy(&w);
}
