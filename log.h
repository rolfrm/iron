#define LOG_DEBUG 1
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_GRAY "\x1b[37m"

#define array_element_size(array) sizeof(array[0])
#define array_count(array) (sizeof(array)/array_element_size(array))

void log_print(char * fmt, ...);

#define log(...) {log_print(ANSI_COLOR_YELLOW __VA_ARGS__); log_print(ANSI_COLOR_RESET);}
#define logd(...) { if(LOG_DEBUG){log_print(ANSI_COLOR_GRAY); log_print(__VA_ARGS__); log_print(ANSI_COLOR_RESET);}}
#define loge(...) {log_print(ANSI_COLOR_RED __VA_ARGS__); log_print(ANSI_COLOR_RESET);}
#define ERROR_TRACE logd( "error: at '" __FILE__  "' line %i: \n",  __LINE__);

// used for error handling
// needs to be implemented in user code
void _error(const char * file, int line, const char * message, ...);
#define ERROR(msg,...) _error(__FILE__,__LINE__,msg, ##__VA_ARGS__) 
#define ASSERT(expr) if(!(expr)){ERROR("Assertion '" ##expr "' Failed");}
