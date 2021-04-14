#ifndef _UTIL_H
#define _UTIL_H

#include <errno.h>
#include <syslog.h>

# define ERR(fmt, args...)       do { syslog(LOG_ERR,     "%s() - " fmt,     __func__, ##args); } while(0)
# define PERROR(fmt, args...)    do { syslog(LOG_ERR,     fmt ". Error %d: %m", ##args, errno); } while(0)
# define WARN(fmt, args...)      do { syslog(LOG_WARNING, "%s() - " fmt,     __func__, ##args); } while(0)
# define LOG(fmt, args...)       do { syslog(LOG_NOTICE,            fmt,               ##args); } while(0)

# define BUG(fmt, args...)       do { syslog(LOG_CRIT,    "%s() - " fmt,     __func__, ##args); } while(0)
# define DBG(fmt, args...)       do { syslog(LOG_DEBUG,   "%s() - " fmt,     __func__, ##args); } while(0)

#define container_of(_ptr, _type, _member) \
	((_type *)(((void *)(_ptr)) - __builtin_offsetof(_type, _member)))

#endif	/* _UTIL_H */
