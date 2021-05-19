/*#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_GRAY "\x1b[37m"
*/
#define ANSI_COLOR_RED     ""
#define ANSI_COLOR_GREEN   ""
#define ANSI_COLOR_YELLOW  ""
#define ANSI_COLOR_BLUE    ""
#define ANSI_COLOR_MAGENTA ""
#define ANSI_COLOR_CYAN    ""
#define ANSI_COLOR_RESET   ""
#define ANSI_COLOR_GRAY ""
typedef enum{
  LOG_DEBUG = 1,
  LOG_INFO = 2,
  LOG_ERROR = 3

}log_level;
void log_print(log_level level, const char * fmt, ...);
extern int logd_enable;
#define logi(...) ({log_print(LOG_INFO, __VA_ARGS__);})
#define logd(...) ({log_print(LOG_DEBUG, __VA_ARGS__);})
#define loge(...) ({log_print(LOG_ERROR, __VA_ARGS__);})
#define ERROR_TRACE logd( "error: at '" __FILE__  "' line %i: \n",  __LINE__);

// used for error handling
// needs to be implemented in user code
void _error(const char * file, int line, const char * message, ...);
#define ERROR(msg,...) _error(__FILE__,__LINE__,msg, ##__VA_ARGS__)

//#ifdef 1
#define ASSERT(expr) if(__builtin_expect(!(expr), 0)){ERROR("Assertion '" #expr "' Failed");}

#define UNREACHABLE() {ERROR("Should not be reachable");}
//#else
//#define ASSERT(expr) if(expr){};

//#define UNREACHABLE();
//#endif

void iron_log_stacktrace(void);
//extern void (* iron_log_printer)(const char * fnt, va_list lst);
