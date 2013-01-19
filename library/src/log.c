#include "log.h"

#include <glib.h>
#include <string.h>
#include <syslog.h>

static enum log_type type;

static const gchar* err_messages[2] = {
	"TEST ERROR",
	"COMMAND FAILED"
};

static const gchar* cmd_server_template =
	"running command `%s` as user `%s` in dir `%s` from host `%s`";

static const gchar* cmd_client_template =
	"running command `%s` as user `%s` in dir `%s` on hosts `%s`";

static const gchar* cmd_server_status_template =
	"command `%s` run as user `%s` in dir `%s` from host `%s` exited with code `%d`";

static const gchar* cmd_client_status_template =
	"command `%s` run as user `%s` in dir `%s` exited with code `%d` on hosts `%s`";

void init_logger(enum log_type t) {
	type = t;
	openlog(WSH_IDENT, LOG_PID, LOG_DAEMON);
}

void exit_logger(void) {
	closelog();
}

void log_message(const gchar* message) {
	syslog(LOG_INFO, "%s: %s", type == CLIENT ? "CLIENT" : "SERVER", message);
}

void log_error(gint msg_num, gchar* message) {
	syslog(LOG_ERR, "%s ERROR %d: %s: %s", type == CLIENT ? "CLIENT" : "SERVER", msg_num, err_messages[msg_num], message);
}

void log_server_cmd(const gchar* command, const gchar* user, const gchar* source, const gchar* cwd) {
	g_assert(type != CLIENT);

	gsize attempted;
	gsize str_len = strlen(cmd_server_template) + strlen(command) + strlen(user) + strlen(source) + strlen(cwd);
	gchar* msg = g_slice_alloc0(str_len);

	if ((attempted = g_snprintf(msg, str_len, cmd_server_template, command, user, cwd, source)) > str_len) {
		g_slice_free1(str_len + 1, msg);

		msg = g_slice_alloc0(attempted);
		g_snprintf(msg, attempted, cmd_server_template, command, user, cwd, source);
	}

	log_message(msg);
	g_slice_free1(strlen(msg) + 1, msg);
}

void log_client_cmd(const gchar* command, const gchar* user, gchar** dests, const gchar* cwd) {
	g_assert(type != SERVER);

	gsize attempted;
	gsize str_len = strlen(cmd_client_template) + strlen(command) + strlen(user) + strlen(cwd);

	for (gint i = 0; dests[i] != NULL; i++)
		str_len += strlen(dests[i]);
	
	gchar* msg = g_slice_alloc0(str_len);
	gchar* hosts = g_strjoinv(", ", dests);

	if ((attempted = g_snprintf(msg, str_len, cmd_client_template, command, user, cwd, hosts)) > str_len) {
		g_slice_free1(str_len + 1, msg);
		
		msg = g_slice_alloc0(attempted);
		g_snprintf(msg, attempted, cmd_client_template, command, user, cwd, hosts);
	}

	log_message(msg);
	g_free(hosts);
	g_slice_free1(strlen(msg) + 1, msg);
}

void log_server_cmd_status(const gchar* command, const gchar* user, const gchar* source, const gchar* cwd, gint status) {
	g_assert(type != CLIENT);

	gsize attempted;
	gsize str_len = strlen(cmd_server_status_template) + strlen(command) + strlen(user) + strlen(source) + strlen(cwd) + 3;
	gchar* msg = g_slice_alloc0(str_len + 1);

	if ((attempted = g_snprintf(msg, str_len, cmd_server_status_template, command, user, cwd, source, status)) > str_len) {
		g_slice_free1(str_len + 1, msg);

		msg = g_slice_alloc0(attempted);
		g_snprintf(msg, attempted, cmd_server_status_template, command, user, cwd, source, status);
	}

	log_message(msg);
	g_slice_free1(strlen(msg) + 1, msg);
}

void log_client_cmd_status(const gchar* command, const gchar* user, gchar** dests, const gchar* cwd, gint status) {
	g_assert(type != SERVER);

	gsize attempted;
	gsize str_len = strlen(cmd_client_status_template) + strlen(command) + strlen(user) + strlen(cwd) + 3;
	gchar* msg = g_slice_alloc0(str_len + 1);

	for (gint i = 0; dests[i] != NULL; i++)
		str_len += strlen(dests[i]);
	
	gchar* hosts = g_strjoinv(", ", dests);

	if ((attempted = g_snprintf(msg, str_len, cmd_client_status_template, command, user, cwd, status, hosts)) > str_len) {
		g_slice_free1(str_len + 1, msg);

		msg = g_slice_alloc0(attempted);
		g_snprintf(msg, attempted, cmd_client_status_template, command, user, cwd, status, hosts);
	}

	log_message(msg);
	g_slice_free1(strlen(msg) + 1, msg);
	g_free(hosts);
}
