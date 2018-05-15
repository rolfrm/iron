#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "mem.h"
#include "utils.h"
#include "types.h"
#include "datastream.h"
#include "log.h"

typedef struct _listener_data listener_data;

struct _listener_data {
  data_stream_listener * listener;
  listener_data * next;
};

data_stream data_stream_log = {.name = "dstream"};

static listener_data * all_listeners = NULL;
static listener_data * activity_listeners = NULL;
// lock free add to list
static void data_stream_add_listener(data_stream_listener *l, listener_data ** list){
  
  while(true){
    if(*list == NULL){
      listener_data * ld = alloc(sizeof(listener_data));
      ld->listener = l;
      ld->next = NULL;
      while(!__sync_bool_compare_and_swap(list, NULL, ld)){
	*list = (*list)->next;
      }
      //dmsg(data_stream_log, "Using new listener slot at %p (listener %p)", *list, l);
      return;
    }
    if(__sync_bool_compare_and_swap(&(*list)->listener, NULL, l)){
      //dmsg(data_stream_log, "Reusing listener slot at %p (listener %p)", *list, l);
      return;
    }
    list = &(*list)->next;
  }
}

static void data_stream_remove_listener(data_stream_listener *l, listener_data ** list){
  
  while(*list != NULL){
    __sync_bool_compare_and_swap(&(*list)->listener, l, NULL);
    list = &(*list)->next;
  }
}


static bool chk_listeners(listener_data * next){
  while(next != NULL){
    if(next->listener != NULL)
      return true;
    next = next->next;
  }
  return false;
}

void data_stream_listen_all(data_stream_listener * l){
  data_stream_add_listener(l, &all_listeners);
}

void data_stream_listen(data_stream_listener * listener, data_stream * stream){
  data_stream_add_listener(listener, (listener_data **) &stream->internal);
}

void data_stream_unlisten(data_stream_listener * listener, data_stream * stream){
  data_stream_remove_listener(listener, (listener_data **) &stream->internal);
}

void data_stream_unlisten_all(data_stream_listener * listener){
  data_stream_remove_listener(listener, &all_listeners);
}

void data_stream_listen_activity(data_stream_listener * listener){
  data_stream_add_listener(listener, &activity_listeners);
}

void data_stream_unlisten_activity(data_stream_listener * listener){
  data_stream_remove_listener(listener, &activity_listeners);
}

static void send_activity(listener_data * next, const data_stream * stream){
  while(next != NULL){
    var a = next->listener;
    if(a->process != NULL)
      a->process(stream, NULL, 0, a->userdata);
    next = next->next;
  }
}

void data_stream_data(const data_stream * stream, const void * data, size_t length){
  void send_msg(listener_data * next){
    while(next != NULL){
      var a = next->listener;
      if(a->process != NULL){
	a->process(stream, data, length, a->userdata);
      }
      next = next->next;
    }
  }
  send_activity(activity_listeners, stream);
  send_msg(stream->internal);
  send_msg(all_listeners);
}

void data_stream_message(const data_stream * stream, const char * msg, ...){
  if(!chk_listeners(all_listeners) && !chk_listeners(stream->internal)){
    // no one is listening.
    // update activity if applicable.
    send_activity(activity_listeners, stream);
    return; 
  }
  
  static __thread char writebuf[4000];
  va_list args;
  va_start (args, msg);
  int cnt = vsnprintf(writebuf, sizeof(writebuf), msg, args);
  va_end(args);
  ASSERT(cnt >= 0);
  data_stream_data(stream, writebuf, cnt);
}

