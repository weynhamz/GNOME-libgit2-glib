/*
 * ggit-message.c
 * This file is part of libgit2-glib
 *
 * Copyright (C) 2013 - Jesse van den Kieboom
 *
 * libgit2-glib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libgit2-glib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libgit2-glib. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ggit-message.h"
#include <git2.h>
#include <string.h>

/**
 * ggit_message_prettify:
 * @message: the message.
 * @strip_comments: whether to strip comments.
 *
 * Prettify a commit message by removing excess whitespace and making sure the
 * last line ends with a newline. If @strip_comments is %TRUE, then lines
 * starting with a '#' will be removed.
 *
 * Returns: the prettified message.
 *
 */
gchar *
ggit_message_prettify (const gchar *message,
                       gboolean     strip_comments)
{
	gchar *ret;
	gint len;
	gchar *d;

	len = strlen (message) * 2;
	ret = g_new0 (gchar, len);

	git_message_prettify (ret, len, message, strip_comments);

	d = g_strdup (ret);
	g_free (ret);

	return d;
}

/* ex:set ts=8 noet: */
