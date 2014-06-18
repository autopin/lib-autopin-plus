/*
 * Autopin+ - Automatic thread-to-core-pinning tool: Communication Libraries
 * Copyright (C) 2012 LRR
 *
 * Author:
 * Florian Walter
 *
 */

#include "libautopin+_linuxC.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

/*!
 * \mainpage
 * This is the code documentation for the autopin+ communication library for Linux. For information
 * on how to compile the library refer to the README file.
 */

/*!
 * \brief Maximum length for paths of UNIX domain sockets
 *
 * Taken from sys/un.h
 */
#define UNIX_PATH_MAX 108

/*!
 * \brief Writes debug output
 *
 * The function only writes output when debug messages are enabled.
 *
 * \param[in] msg	The debug message
 */
static void writeDebug(const char *msg);

/*!
 * \brief Tries to connect to autopin
 *
 * \return The function returns 0 if the connection can be established.
 * 	If the maximum number of connection attempts has been reached 1
 * 	is returned. In all other cases the return value is -1;
 *
 */
static int connectToAutopin(void);

/*!
 * \brief Creates the address of the UNIX domain socket
 *
 * \param[in] addr The file which will be used for the UNIX domain socket
 * 	(e. g. /tmp/autopin_socket). The file must not exist and the user needs
 * 	full access to the directory. If this argument is NULL the socket will
 * 	be created in the home directory of the user.
 *
 * \return Returns 0 on success and -1 in all other cases
 *
 */
static int createSockAddr(const char *addr);

/*!
 * \brief Receives control commands from autopin+
 *
 * \return Returns 0 on success and -1 in all other cases
 *
 */
static int getAutopinCmds(void);

/*!
 * Stores if debug messages are enabled
 */
static int debug_enabled = 0;

/*!
 * Stores the initialization state of the library.
 */
static int initialized = 0;

/*!
 * Stores if the library is active
 */
static int connected = 0;

/*!
 * Maximum number of conncection attempts to autopin+
 */
static int max_tries;

/*!
 * Address structure the UNIX domain socket
 */
static struct sockaddr_un unix_addr;

/*!
 * The communication socket
 */
static int unix_socket;

/*!
 * Mutex for thread safe operations
 */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*!
 * Stores the pid of the process
 */
pid_t pid;

/*!
 * Current execution phase of the application
 */
int current_phase;

/*!
 * Stores the last time when a phase has been reported
 */
int phase_reported;

/*!
 * Stores the minimum interval (in seconds) between two messages.
 * All requests following a message to autopin+ within this interval
 * will be ignored
 */
int interval;

int autopin_init(int debug, int maxt, int force, const char *addr) {
	int result;

	if (initialized != 0) return 1;

	debug_enabled = debug;
	writeDebug("Initializing the autopin+ communication library");
	max_tries = maxt;
	pid = getpid();

	phase_reported = -1;
	interval = 0;

	// Create path for the unix domain socket
	result = createSockAddr(addr);
	if (result != 0) {
		writeDebug("Cannot set the path for the communication socket");
		return -1;
	}

	// Create the socket
	unix_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (unix_socket == -1) {
		writeDebug("Cannot create the communication socket");
		return -1;
	}

	if (force > 0) {
		writeDebug("Connection forced");
		initialized = 1;

		result = -1;

		while (result != 1) {
			result = connectToAutopin();

			if (result == 0)
				break;
			else if (result == 1) {
				initialized = 0;
				return -1;
			}

			usleep(force);
		}
	}

	initialized = 1;

	atexit(autopin_exit);
	return 0;
}

void autopin_exit(void) {
	pthread_mutex_lock(&mutex);

	if (initialized != 0) {
		writeDebug("Uninitializing the autopin+ commmunication library");

		if (connected == 1) close(unix_socket);
		connected = 0;
		initialized = 0;
	}

	pthread_mutex_unlock(&mutex);
}

int reportPhase(int phase) {
	pthread_mutex_lock(&mutex);

	if (initialized != 0 && connectToAutopin() == 0) {

		getAutopinCmds();

		int current_time = time(NULL);

		if (phase_reported == -1 || (current_time - phase_reported) > interval) {

			if (phase_reported == -1 || phase != current_phase) {
				writeDebug("Reporting new phase");

				struct autopin_msg msg;
				msg.event_id = APP_NEW_PHASE;
				msg.arg = phase;

				int result = send(unix_socket, &msg, sizeof(msg), 0);
				if (result == -1) {
					writeDebug("Communication error: cannot send message to autopin+");
					pthread_mutex_unlock(&mutex);
					return -1;
				}

				phase_reported = current_time;
				current_phase = phase;
			}
			pthread_mutex_unlock(&mutex);
			return 0;
		}
	}

	pthread_mutex_unlock(&mutex);
	return 1;
}

int sendMsg(int arg, double val) {
	pthread_mutex_lock(&mutex);

	if (initialized != 0 && connectToAutopin() == 0) {
		getAutopinCmds();

		struct autopin_msg msg;

		msg.event_id = APP_USER;
		msg.arg = arg;
		msg.val = val;

		int result = send(unix_socket, &msg, sizeof(msg), 0);
		if (result == -1) {
			writeDebug("Communication error: cannot send message to autopin+");
			pthread_mutex_unlock(&mutex);
			return -1;
		} else {
			pthread_mutex_unlock(&mutex);
			return 0;
		}
	}

	pthread_mutex_unlock(&mutex);

	return -1;
}

static void writeDebug(const char *msg) {
	if (debug_enabled) printf("libautopin+: %s\n", msg);
}

static int createSockAddr(const char *addr) {
	// Set the type of the address structure
	unix_addr.sun_family = AF_UNIX;

	// Set the path of the address structure
	if (addr == NULL) {
		// Get home directory of the user
		const char *home = getenv("HOME");
		int homelen;

		if (home == NULL) {
			writeDebug("Could not determine the user's home directory");
			return -1;
		}

		homelen = strlen(home);

		if (homelen + 1 > UNIX_PATH_MAX - strlen("/.autopin_socket")) return -1;

		strcpy(unix_addr.sun_path, home);
		strcat(unix_addr.sun_path, "/.autopin_socket");
	} else {
		int addrlen = strlen(addr);

		if (addrlen + 1 > UNIX_PATH_MAX) return -1;

		strcpy(unix_addr.sun_path, addr);
	}

	return 0;
}

static int connectToAutopin(void) {
	int result;

	if (initialized == 0) return -1;
	if (max_tries == 0) return 1;
	if (connected == 1) return 0;

	writeDebug("Trying to connect to autpin+");

	result = connect(unix_socket, (const struct sockaddr *)&unix_addr, sizeof(struct sockaddr_un));

	if (result == 0) {
		writeDebug("Connection to autopin+ established");
		connected = 1;
		struct autopin_msg recv_buffer;
		// Wait until autopin+ is ready
		writeDebug("Waiting for response from autopin+");
		result = recv(unix_socket, &recv_buffer, sizeof(recv_buffer), 0);
		if (result == -1 && recv_buffer.event_id != APP_READY) {
			writeDebug("Communication error - connection has been disabled");
			max_tries = 0;
			connected = 0;
			return 1;
		}

		writeDebug("Received response");
		return 0;
	}

	writeDebug("Could not connect to autopin+");

	if (max_tries > 0) max_tries--;

	return -1;
}

static int getAutopinCmds(void) {
	int result;
	struct autopin_msg msg;

	result = recv(unix_socket, &msg, sizeof(msg), MSG_DONTWAIT);

	while (result > 0) {
		switch (msg.event_id) {
		case APP_INTERVAL:
			interval = (msg.arg >= 0) ? msg.arg : interval;
			writeDebug("New interval has been set");
			break;

		default:
			break;
		}

		result = recv(unix_socket, &msg, sizeof(msg), MSG_DONTWAIT);
	}

	return 0;
}
