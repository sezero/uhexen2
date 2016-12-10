#include <time.h>

void tzset(void) {
/* not using timezones */
}
void __tzset(void) __attribute__((alias("tzset")));
void tzsetwall(void) __attribute__((alias("tzset")));
void __tzsetwall(void) __attribute__((alias("tzset")));
