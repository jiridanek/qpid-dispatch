#include <stdarg.h>

extern void closelog(void) {

}
extern void openlog(const char *ident, int option, int facility) {

}
extern void syslog(int priority, const char *format, ...) {

}
extern void vsyslog(int priority, const char *format, va_list ap) {

}
