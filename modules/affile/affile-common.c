/*
 * Copyright (c) 2002-2012 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 1998-2012 Balázs Scheidler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */
#include "affile-common.h"
#include "messages.h"
#include "gprocess.h"
#include "misc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

static const gchar* spurious_paths[] = {"../", "/..", NULL};

static inline gboolean
_string_contains_fragment(const gchar *str, const gchar *fragments[])
{
  int i;

  for (i=0; i < sizeof(fragments) / sizeof(gchar *); i++)
  {
    if (strstr(str, fragments[i]))
      return TRUE;
  }
  
  return FALSE;
}

static inline gboolean
_path_is_spurious(const gchar *name, const gchar *spurious_paths[])
{
  return _string_contains_fragment(name, spurious_paths);
}

static inline gboolean
_affile_set_caps(gchar *name, FileOpenOptions *opts, cap_t *act_caps)
{
  if (opts->needs_privileges)
    {
      g_process_cap_modify(CAP_DAC_READ_SEARCH, TRUE);
      g_process_cap_modify(CAP_SYSLOG, TRUE);
    }
  else
    {
      g_process_cap_modify(CAP_DAC_OVERRIDE, TRUE);
    }

  if (opts->create_dirs && opts->perm_options &&
      !file_perm_options_create_containing_directory(opts->perm_options, name))
    {
      return FALSE;
    }

  return TRUE;
}

static inline void
_affile_set_fd_permission(FileOpenOptions *opts, int fd)
{
  if (fd != -1)
    {
      g_fd_set_cloexec(fd, TRUE);

      g_process_cap_modify(CAP_CHOWN, TRUE);
      g_process_cap_modify(CAP_FOWNER, TRUE);
      if (opts->perm_options)
        file_perm_options_apply_fd(opts->perm_options, fd);
    }
}

static inline int
_affile_open_fd(const gchar *name, FileOpenOptions *opts)
{
  int fd;

  fd = open(name, opts->open_flags,
            (!opts->perm_options || (opts->perm_options->file_perm < 0))
            ? 0600
            : opts->perm_options->file_perm);

  if (opts->is_pipe && fd < 0 && errno == ENOENT)
    {
      if (mkfifo(name, 0666) >= 0)
        fd = open(name, opts->open_flags, 0666);
    }

  return fd;
}

static inline void
_affile_check_file_type(const gchar *name, FileOpenOptions *opts)
{
  struct stat st;

  if (stat(name, &st) >= 0)
    {
      if (opts->is_pipe && !S_ISFIFO(st.st_mode))
        {
          msg_warning("WARNING: you are using the pipe driver, underlying file is not a FIFO, it should be used by file()",
                    evt_tag_str("filename", name),
                    NULL);
        }
      else if (!opts->is_pipe && S_ISFIFO(st.st_mode))
        {
          msg_warning("WARNING: you are using the file driver, underlying file is a FIFO, it should be used by pipe()",
                      evt_tag_str("filename", name),
                      NULL);
        }
    }
}

gboolean
affile_open_file(gchar *name, FileOpenOptions *opts, gint *fd)
{
  cap_t saved_caps;

  if (_path_is_spurious(name, spurious_paths))
    {
      msg_error("Spurious path, logfile not created",
                 evt_tag_str("path", name),
                 NULL);
      return FALSE;
    }

  saved_caps = g_process_cap_save();

  if (!_affile_set_caps(name, opts, &saved_caps))
    {
      g_process_cap_restore(saved_caps);
      return FALSE;
    }

  _affile_check_file_type(name, opts);

  *fd = _affile_open_fd(name, opts);

  _affile_set_fd_permission(opts, *fd);

  g_process_cap_restore(saved_caps);

  msg_trace("affile_open_file",
            evt_tag_str("path", name),
            evt_tag_int("fd", fd),
            NULL);

  return (*fd != -1);
}
