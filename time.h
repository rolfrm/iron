//requires bitguy.h

// gets time in microseconds, which is the highest achivable resolution.
u64 timestamp();

// sleeps for a number of microseconds
// dont use for polling, use semaphores/mutexes instead if possible.
void iron_usleep(int microseconds);

u64 measure_elapsed(void (*fcn)());
