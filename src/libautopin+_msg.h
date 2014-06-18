/*
 * Autopin+ - Automatic thread-to-core-pinning tool: Communication libraries
 * Copyright (C) 2012 LRR
 *
 * Author:
 * Florian Walter
 */

#ifndef LIBAUTOPIN_MSG_H
#define LIBAUTOPIN_MSG_H

// Messages from autopin+ to the application
#define APP_READY 0x0001
#define APP_INTERVAL 0x0010
// Messages from the application to autopin+
#define APP_NEW_PHASE 0x0100
#define APP_USER 0x1000

struct __attribute__((__packed__)) autopin_msg {
	unsigned long event_id;
	unsigned long arg;
	double val;
};

#endif // LIBAUTOPIN_MSG_H
