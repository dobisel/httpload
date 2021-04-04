#include "logging.h"


char log_level = LOG_INFO;


const char * log_levelnames [] = {
	"ERROR",	// 0
	"WARNING",	// 1
	"INFO",		// 2
	"DEBUG",	// 3
};


void log_init(enum log_level level) {
	log_level = level;
	L_INFO("Logging Initialized, level: %s", log_levelnames[level]);
}

