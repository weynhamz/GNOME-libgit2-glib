/*
 * ggit-blob-output-stream.h
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


#ifndef __GGIT_BLOB_OUTPUT_STREAM_H__
#define __GGIT_BLOB_OUTPUT_STREAM_H__

#include <glib-object.h>
#include <gio/gio.h>

#include "ggit-types.h"

G_BEGIN_DECLS

#define GGIT_TYPE_BLOB_OUTPUT_STREAM		(ggit_blob_output_stream_get_type ())
#define GGIT_BLOB_OUTPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GGIT_TYPE_BLOB_OUTPUT_STREAM, GgitBlobOutputStream))
#define GGIT_BLOB_OUTPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GGIT_TYPE_BLOB_OUTPUT_STREAM, GgitBlobOutputStreamClass))
#define GGIT_IS_BLOB_OUTPUT_STREAM(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGIT_TYPE_BLOB_OUTPUT_STREAM))
#define GGIT_IS_BLOB_OUTPUT_STREAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GGIT_TYPE_BLOB_OUTPUT_STREAM))
#define GGIT_BLOB_OUTPUT_STREAM_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GGIT_TYPE_BLOB_OUTPUT_STREAM, GgitBlobOutputStreamClass))

typedef struct _GgitBlobOutputStreamClass	GgitBlobOutputStreamClass;
typedef struct _GgitBlobOutputStreamPrivate	GgitBlobOutputStreamPrivate;

struct _GgitBlobOutputStream
{
	/*< private >*/
	GOutputStream parent;

	GgitBlobOutputStreamPrivate *priv;
};

/**
 * GgitBlobOutputStreamClass:
 * @parent_class: The parent class.
 *
 * The class structure for #GgitBlobOutputStreamClass.
 */
struct _GgitBlobOutputStreamClass
{
	/*< private >*/
	GOutputStreamClass parent_class;
};

GType             ggit_blob_output_stream_get_type         (void) G_GNUC_CONST;

GgitBlobOutputStream *_ggit_blob_output_stream_new         (GgitRepository *repository);

GgitOId *ggit_blob_output_stream_get_id (GgitBlobOutputStream  *stream,
                                         GError               **error);

G_END_DECLS

#endif /* __GGIT_BLOB_OUTPUT_STREAM_H__ */

/* ex:set ts=8 noet: */
