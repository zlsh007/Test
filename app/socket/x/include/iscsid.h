/*
 *  Copyright (C) 2002 - 2003 Ardis Technolgies <roman@ardistech.com>
 *  Copyright (C) 2007 - 2016 Vladislav Bolkhovitin
 *  Copyright (C) 2007 - 2016 SanDisk Corporation
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation, version 2
 *  of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 */

#ifndef ISCSID_H
#define ISCSID_H

#include <search.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <assert.h>
#include <netdb.h>
#include <syslog.h>

#include "types.h"
#include "iscsi_hdr.h"
#include "param.h"

#ifndef bool
typedef enum {false = 0, true} bool;
#endif

#define sBUG() assert(0)
#define sBUG_ON(p) assert(!(p))

struct PDU {
	struct iscsi_hdr bhs;
	void *ahs;
	unsigned int ahssize;
	void *data;
	unsigned int datasize;
};

#define KEY_STATE_START		0
#define KEY_STATE_REQUEST	1
#define KEY_STATE_DONE_ADDED	2
#define KEY_STATE_DONE		3

#define IOSTATE_FREE		0
#define IOSTATE_READ_BHS	1
#define IOSTATE_READ_AHS_DATA	2
#define IOSTATE_WRITE_BHS	3
#define IOSTATE_WRITE_AHS	4
#define IOSTATE_WRITE_DATA	5

#define STATE_FREE		0
#define STATE_SECURITY		1
#define STATE_SECURITY_AUTH	2
#define STATE_SECURITY_DONE	3
#define STATE_SECURITY_LOGIN	4
#define STATE_SECURITY_FULL	5
#define STATE_LOGIN		6
#define STATE_LOGIN_FULL	7
#define STATE_FULL		8
#define STATE_KERNEL		9
#define STATE_CLOSE		10
#define STATE_EXIT		11
#define STATE_DROP		12

#define AUTH_STATE_START	0
#define AUTH_STATE_CHALLENGE	1

#define SESSION_NORMAL		0
#define SESSION_DISCOVERY	1
#define AUTH_UNKNOWN		-1
#define AUTH_NONE		0
#define AUTH_CHAP		1
#define DIGEST_UNKNOWN		-1

#define BHS_SIZE		48

/*
 * Must be 8192, since it used as MaxRecvDataSegmentLength during Login phase,
 * because iSCSI RFC requires: "The default MaxRecvDataSegmentLength is used
 * during Login".
 */
#define INCOMING_BUFSIZE	8192

#define ISCSI_USER_DIR_INCOMING	0
#define ISCSI_USER_DIR_OUTGOING	1

#define ISCSI_USER_NAME(attr)	((attr)->attr_key)
#define ISCSI_USER_PASS(attr)	((attr)->attr_value)

struct iscsi_attr {
	const char *attr_key;
	const char *attr_value;
	u32 sysfs_mode;
	char sysfs_name[64];
};

#define LISTEN_MAX		8
#define INCOMING_MAX		256

enum {
	POLL_LISTEN,
	POLL_IPC = POLL_LISTEN + LISTEN_MAX,
	POLL_ISER_LISTEN,
	POLL_NL,
	POLL_ISNS,
	POLL_SCN_LISTEN,
	POLL_SCN,
	POLL_INCOMING,
	POLL_MAX = POLL_INCOMING + INCOMING_MAX,
};
/* log.c */
extern int log_daemon;
extern int log_level;

extern void log_init(void);
extern void __log(const char *func, int line, int prio, int level, const char *fmt, ...)
	__attribute__ ((format (printf, 5, 6)));
extern void __log_pdu(const char *func, int line, int level, struct PDU *pdu);

#define log_info(args...)		__log(__func__, __LINE__, LOG_INFO, 0, ## args)
#define log_warning(args...)		__log(__func__, __LINE__, LOG_WARNING, 0, ## args)
#define log_error(args...)		__log(__func__, __LINE__, LOG_ERR, 0, ## args)
#define log_debug(level, args...)	__log(__func__, __LINE__, LOG_DEBUG, level, ## args)
#define log_pdu(level, args...)		__log_pdu(__func__, __LINE__, level, ## args)

/* Conditional versions of log_* functions. Useful when log priority depends
 * on some parameter, say recurrence of some event. In these cases the first
 * occurence could be logged as log_info while the latter ones may be logged
 * with log_debug. So, if level != 0 then log_debug is called.
 */
#define log_info_cond(level, args...)		\
	__log(__func__, __LINE__, LOG_INFO, level, ## args)
#define log_warning_cond(level, args...)	\
	__log(__func__, __LINE__, LOG_WARNING, level, ## args)
#define log_error_cond(level, args...)		\
	__log(__func__, __LINE__, LOG_ERR, level, ## args)

/* event.c */
extern int handle_iscsi_events(int fd, bool wait);
extern int nl_open(void);

#endif	/* ISCSID_H */
