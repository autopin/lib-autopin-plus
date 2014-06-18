/*
 * Autopin+ - Automatic thread-to-core-pinning tool: Communication libraries
 * Copyright (C) 2012 LRR
 *
 * Author:
 * Florian Walter
 */

#ifndef LIBAUTOPIN_LINUXC_H
#define LIBAUTOPIN_LINUXC_H

#include "libautopin+_msg.h"

/*!
 * \brief Initialize the library
 *
 * \param[in] debug If set to 0 debug output of the library is disabled. In
 *	all other cases the library will print debug messages to the standard
 *	output.
 * \param[in] maxt Specfies the maximum number of tries which will be made to
 * 	connect to autopin. A value <= 0 will lead to an infinite number of
 * 	connections attempts.
 * \param[in] force If this argument is set to a value > 0 the function will block
 *	until a connection has been established or the maximum number of connection
 * 	attempts has been reached. The value defines the number of microseconds
 * 	between two connection attempts.
 * \param[in] addr The file which will be used for the UNIX domain socket
 * 	(e. g. /tmp/autopin_socket). The file must not exist and the user needs
 * 	full access to the directory. If this argument is NULL the socket will
 * 	be created in the home directory of the user.
 *
 * \return On success the function returns 0. If the library has already been
 * 	initialized 1 is returned. In all other cases the result is -1.
 */
extern int autopin_init(int debug, int maxt, int force, const char *addr);

/*!
 * \brief Uninitialize the library
 *
 */
extern void autopin_exit(void);

/*!
 * \brief Report a new phase to autopin+
 *
 * Only phase changes will be reported to autopin+. If autopin+ has sent a
 * filter request the phase change might not be reported to autopin+. In this
 * case the function returns -1.
 *
 * \param[in] phase The id of the new phase
 *
 * \return 0 if the phase can be reported or if it already has been reported. If the
 * 	message is blocked because of a filter the result will be 1. In all other cases
 * 	the function returns -1.
 *
 */
extern int reportPhase(int phase);

/*!
 * \brief Send a custom message to autopin+
 *
 * This method can be used to send custom messages to a pinning strategy with
 * support for these messages. Messages sent via this functions are not affected
 * by filter settings!
 *
 * \param[in] arg An integer argument
 * \param[in] val A doubel value
 *
 * \return 0 if the messages has been transmitted successfully and -1 in all other
 * 	cases
 *
 */
extern int sendMsg(int arg, double val);

#endif // LIBAUTOPIN_LINUXC_H
