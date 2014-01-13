/*
 * ggit-diff.c
 * This file is part of libgit2-glib
 *
 * Copyright (C) 2012 - Garrett Regier
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

#include <git2.h>

#include "ggit-diff.h"
#include "ggit-diff-delta.h"
#include "ggit-diff-options.h"
#include "ggit-diff-line.h"
#include "ggit-diff-hunk.h"
#include "ggit-patch.h"
#include "ggit-error.h"
#include "ggit-repository.h"
#include "ggit-diff-file.h"

#define GGIT_DIFF_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE((object), GGIT_TYPE_DIFF, GgitDiffPrivate))

struct _GgitDiffPrivate
{
	GgitRepository *repository;
	gchar *encoding;
};

typedef struct {
	GgitDiff *diff;
	const gchar *encoding;

	gpointer user_data;

	GgitDiffFileCallback file_cb;
	GgitDiffHunkCallback hunk_cb;
	GgitDiffLineCallback line_cb;
} CallbackWrapperData;

G_DEFINE_TYPE (GgitDiff, ggit_diff, GGIT_TYPE_NATIVE)

enum
{
	PROP_0,
	PROP_REPOSITORY
};

static gint
ggit_diff_file_callback_wrapper (const git_diff_delta *delta,
                                 gfloat                progress,
                                 gpointer              user_data)
{
	CallbackWrapperData *data = user_data;
	GgitDiffDelta *gdelta;
	gint ret;

	gdelta = _ggit_diff_delta_wrap (delta);

	data->encoding = NULL;

	if (data->diff != NULL)
	{
		GgitDiffFile *file;

		if (ggit_diff_delta_get_status (gdelta) == GGIT_DELTA_DELETED)
		{
			file = ggit_diff_delta_get_old_file (gdelta);
		}
		else
		{
			file = ggit_diff_delta_get_new_file (gdelta);
		}

		if (file != NULL)
		{
			const gchar *path;

			path = ggit_diff_file_get_path (file);

			data->encoding =
				ggit_repository_get_attribute (data->diff->priv->repository,
			                                       path,
			                                       "encoding",
			                                       GGIT_ATTRIBUTE_CHECK_FILE_THEN_INDEX,
			                                       NULL);
		}
	}

	ret = data->file_cb (gdelta, progress, data->user_data);

	ggit_diff_delta_unref (gdelta);

	return ret;
}

static gint
ggit_diff_hunk_callback_wrapper (const git_diff_delta *delta,
                                 const git_diff_hunk  *hunk,
                                 gpointer              user_data)
{
	CallbackWrapperData *data = user_data;
	GgitDiffDelta *gdelta;
	GgitDiffHunk *ghunk;
	gint ret;

	gdelta = _ggit_diff_delta_wrap (delta);
	ghunk = _ggit_diff_hunk_wrap (hunk);

	ret = data->hunk_cb (gdelta, ghunk, data->user_data);

	ggit_diff_hunk_unref (ghunk);
	ggit_diff_delta_unref (gdelta);

	return ret;
}

static gint
ggit_diff_line_callback_wrapper (const git_diff_delta *delta,
                                 const git_diff_hunk  *hunk,
                                 const git_diff_line  *line,
                                 gpointer              user_data)
{
	CallbackWrapperData *data = user_data;
	GgitDiffDelta *gdelta;
	GgitDiffHunk *ghunk;
	GgitDiffLine *gline;
	gint ret;
	const gchar *encoding = NULL;

	if (data->encoding != NULL)
	{
		encoding = data->encoding;
	}
	else if (data->diff != NULL)
	{
		encoding = data->diff->priv->encoding;
	}

	gdelta = _ggit_diff_delta_wrap (delta);
	ghunk = hunk == NULL ? NULL : _ggit_diff_hunk_wrap (hunk);
	gline = line == NULL ? NULL : _ggit_diff_line_wrap (line, encoding);

	ret = data->line_cb (gdelta, ghunk, gline, data->user_data);

	if (gline != NULL)
	{
		ggit_diff_line_unref (gline);
	}

	if (ghunk != NULL)
	{
		ggit_diff_hunk_unref (ghunk);
	}

	ggit_diff_delta_unref (gdelta);

	return ret;
}

static void
ggit_diff_finalize (GObject *object)
{
	GgitDiffPrivate *priv = GGIT_DIFF (object)->priv;

	g_free (priv->encoding);

	G_OBJECT_CLASS (ggit_diff_parent_class)->finalize (object);
}

static void
ggit_diff_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
	GgitDiff *self = GGIT_DIFF (object);

	switch (prop_id)
	{
		case PROP_REPOSITORY:
			self->priv->repository = g_value_dup_object (value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ggit_diff_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
	GgitDiff *self = GGIT_DIFF (object);

	switch (prop_id)
	{
		case PROP_REPOSITORY:
			g_value_set_object (value, self->priv->repository);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ggit_diff_constructed (GObject *object)
{
	GgitDiffPrivate *priv = GGIT_DIFF (object)->priv;
	GgitConfig *config = NULL;

	if (priv->repository != NULL)
	{
		config = ggit_repository_get_config (priv->repository, NULL);
	}

	if (config != NULL)
	{
		const gchar *enc;

		enc = ggit_config_get_string (config, "gui.encoding", NULL);

		if (enc != NULL)
		{
			priv->encoding = g_strdup (enc);
		}

		g_object_unref (config);
	}
}

static void
ggit_diff_class_init (GgitDiffClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = ggit_diff_finalize;
	object_class->constructed = ggit_diff_constructed;

	object_class->get_property = ggit_diff_get_property;
	object_class->set_property = ggit_diff_set_property;

	g_object_class_install_property (object_class,
	                                 PROP_REPOSITORY,
	                                 g_param_spec_object ("repository",
	                                                      "Repository",
	                                                      "Repository",
	                                                      GGIT_TYPE_REPOSITORY,
	                                                      G_PARAM_READWRITE |
	                                                      G_PARAM_CONSTRUCT_ONLY |
	                                                      G_PARAM_STATIC_STRINGS));

	g_type_class_add_private (object_class, sizeof (GgitDiffPrivate));
}

static void
ggit_diff_init (GgitDiff *self)
{
	self->priv = GGIT_DIFF_GET_PRIVATE (self);
}

static GgitDiff *
_ggit_diff_wrap (GgitRepository *repository,
                 git_diff       *diff)
{
	GgitDiff *gdiff;

	gdiff = g_object_new (GGIT_TYPE_DIFF, "repository", repository, NULL);
	_ggit_native_set (gdiff, diff, (GDestroyNotify)git_diff_free);

	return gdiff;
}

/**
 * ggit_diff_new_tree_to_tree:
 * @repository: a #GgitRepository.
 * @old_tree: (allow-none): a #GgitTree to diff from.
 * @new_tree: (allow-none): a #GgitTree to diff to.
 * @diff_options: (allow-none): a #GgitDiffOptions, or %NULL.
 * @error: a #GError for error reporting, or %NULL.
 *
 * Creates a #GgitDiff which compares @old_tree and @new_tree.
 *
 * If @diff_options is %NULL then the defaults specified in
 * ggit_diff_options_new() are used.
 *
 * Returns: a newly allocated #GgitDiff if there was no error, %NULL otherwise.
 */
GgitDiff *
ggit_diff_new_tree_to_tree (GgitRepository   *repository,
                            GgitTree         *old_tree,
                            GgitTree         *new_tree,
                            GgitDiffOptions  *diff_options,
                            GError          **error)
{
	git_diff *diff;
	gint ret;

	g_return_val_if_fail (GGIT_IS_REPOSITORY (repository), NULL);
	g_return_val_if_fail (old_tree == NULL || GGIT_IS_TREE (old_tree), NULL);
	g_return_val_if_fail (new_tree == NULL || GGIT_IS_TREE (new_tree), NULL);
	g_return_val_if_fail (old_tree != NULL || new_tree != NULL, NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	ret = git_diff_tree_to_tree (&diff,
	                             _ggit_native_get (repository),
	                             old_tree ? _ggit_native_get (old_tree) : NULL,
	                             new_tree ? _ggit_native_get (new_tree) : NULL,
	                             _ggit_diff_options_get_diff_options (diff_options));

	if (ret != GIT_OK)
	{
		_ggit_error_set (error, ret);
		return NULL;
	}

	return _ggit_diff_wrap (repository, diff);
}

/**
 * ggit_diff_new_tree_to_index:
 * @repository: a #GgitRepository.
 * @old_tree: (allow-none): a #GgitTree to diff from.
 * @index: (allow-none): a #GgitIndex, or %NULL.
 * @diff_options: (allow-none): a #GgitDiffOptions, or %NULL.
 * @error: a #GError for error reporting, or %NULL.
 *
 * Creates a #GgitDiff which compares @old_tree and the index.
 *
 * If @index is %NULL then @repository index is used.
 * If @diff_options is %NULL then the defaults specified in
 * ggit_diff_options_new() are used.
 *
 * Returns: a newly allocated #GgitDiff if there was no error, %NULL otherwise.
 */
GgitDiff *
ggit_diff_new_tree_to_index (GgitRepository   *repository,
                             GgitTree         *old_tree,
                             GgitIndex        *index,
                             GgitDiffOptions  *diff_options,
                             GError          **error)
{
	git_diff *diff;
	gint ret;

	g_return_val_if_fail (GGIT_IS_REPOSITORY (repository), NULL);
	g_return_val_if_fail (old_tree == NULL || GGIT_IS_TREE (old_tree), NULL);
	g_return_val_if_fail (index == NULL || GGIT_IS_INDEX (index), NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	ret = git_diff_tree_to_index (&diff,
	                              _ggit_native_get (repository),
	                              old_tree ? _ggit_native_get (old_tree) : NULL,
	                              index ? _ggit_native_get (index) : NULL,
	                              _ggit_diff_options_get_diff_options (diff_options));

	if (ret != GIT_OK)
	{
		_ggit_error_set (error, ret);
		return NULL;
	}

	return _ggit_diff_wrap (repository, diff);
}

/**
 * ggit_diff_new_index_to_workdir:
 * @repository: a #GgitRepository.
 * @index: (allow-none): a #GgitIndex, or %NULL.
 * @diff_options: (allow-none): a #GgitDiffOptions, or %NULL.
 * @error: a #GError for error reporting, or %NULL.
 *
 * Creates a #GgitDiff which compares the working directory and the index.
 *
 * If @index is %NULL then @repository index is used.
 * If @diff_options is %NULL then the defaults specified in
 * ggit_diff_options_new() are used.
 *
 * Returns: a newly allocated #GgitDiff if there was no error, %NULL otherwise.
 */
GgitDiff *
ggit_diff_new_index_to_workdir (GgitRepository   *repository,
                                GgitIndex        *index,
                                GgitDiffOptions  *diff_options,
                                GError          **error)
{
	git_diff *diff;
	gint ret;

	g_return_val_if_fail (GGIT_IS_REPOSITORY (repository), NULL);
	g_return_val_if_fail (index == NULL || GGIT_IS_INDEX (index), NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	ret = git_diff_index_to_workdir (&diff,
	                                 _ggit_native_get (repository),
	                                 index ? _ggit_native_get (index) : NULL,
	                                 _ggit_diff_options_get_diff_options (diff_options));

	if (ret != GIT_OK)
	{
		_ggit_error_set (error, ret);
		return NULL;
	}

	return _ggit_diff_wrap (repository, diff);
}

/**
 * ggit_diff_new_tree_to_workdir:
 * @repository: a #GgitRepository.
 * @old_tree: (allow-none): a #GgitTree to diff from.
 * @diff_options: (allow-none): a #GgitDiffOptions, or %NULL.
 * @error: a #GError for error reporting, or %NULL.
 *
 * Creates a #GgitDiff which compares the working directory and @old_tree.
 *
 * If @diff_options is %NULL then the defaults specified in
 * ggit_diff_options_new() are used.
 *
 * Returns: a newly allocated #GgitDiff if there was no error, %NULL otherwise.
 */
GgitDiff *
ggit_diff_new_tree_to_workdir (GgitRepository   *repository,
                               GgitTree         *old_tree,
                               GgitDiffOptions  *diff_options,
                               GError          **error)
{
	git_diff *diff;
	gint ret;

	g_return_val_if_fail (GGIT_IS_REPOSITORY (repository), NULL);
	g_return_val_if_fail (old_tree == NULL || GGIT_IS_TREE (old_tree), NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	ret = git_diff_tree_to_workdir (&diff,
	                                _ggit_native_get (repository),
	                                old_tree ? _ggit_native_get (old_tree) : NULL,
	                                _ggit_diff_options_get_diff_options (diff_options));

	if (ret != GIT_OK)
	{
		_ggit_error_set (error, ret);
		return NULL;
	}

	return _ggit_diff_wrap (repository, diff);
}

/**
 * ggit_diff_find_similar:
 * @diff: the #GgitDiff to work on.
 * @error: a #GError for error reporting, or %NULL.
 *
 * Transform a @diff marking file renames, copies, etc.
 */
void
ggit_diff_find_similar (GgitDiff  *diff,
                        GError   **error)
{
	gint ret;

	g_return_if_fail (GGIT_IS_DIFF (diff));
	g_return_if_fail (error == NULL || *error == NULL);

	ret = git_diff_find_similar (_ggit_native_get (diff),
	                             NULL);

	if (ret != GIT_OK)
	{
		_ggit_error_set (error, ret);
	}
}

/**
 * ggit_diff_merge:
 * @onto: the #GgitDiff to merge into.
 * @from: the #GgitDiff to merge.
 * @error: a #GError for error reporting, or %NULL.
 *
 * Merges @from into @onto unless @error is set.
 */
void
ggit_diff_merge (GgitDiff  *onto,
                 GgitDiff  *from,
                 GError   **error)
{
	gint ret;

	g_return_if_fail (GGIT_IS_DIFF (onto));
	g_return_if_fail (GGIT_IS_DIFF (from));
	g_return_if_fail (error == NULL || *error == NULL);

	ret = git_diff_merge (_ggit_native_get (onto),
	                      _ggit_native_get (from));

	if (ret != GIT_OK)
	{
		_ggit_error_set (error, ret);
	}
}

/**
 * ggit_diff_foreach:
 * @diff: a #GgitDiff.
 * @file_cb: (allow-none) (scope call) (closure user_data):
 *  a #GgitDiffFileCallback.
 * @hunk_cb: (allow-none) (scope call) (closure user_data):
 *  a #GgitDiffHunkCallback.
 * @line_cb: (allow-none) (scope call) (closure user_data):
 *  a #GgitDiffLineCallback.
 * @user_data: callback user data.
 * @error: a #GError for error reporting, or %NULL.
 *
 * Iterates over the diff calling @file_cb, @hunk_cb and @line_cb.
 */
void
ggit_diff_foreach (GgitDiff              *diff,
                   GgitDiffFileCallback   file_cb,
                   GgitDiffHunkCallback   hunk_cb,
                   GgitDiffLineCallback   line_cb,
                   gpointer              *user_data,
                   GError               **error)
{
	gint ret;
	CallbackWrapperData wrapper_data;
	git_diff_file_cb real_file_cb = NULL;
	git_diff_hunk_cb real_hunk_cb = NULL;
	git_diff_line_cb real_line_cb = NULL;

	g_return_if_fail (GGIT_IS_DIFF (diff));
	g_return_if_fail (file_cb != NULL && hunk_cb != NULL && line_cb != NULL);
	g_return_if_fail (error == NULL || *error == NULL);

	wrapper_data.user_data = user_data;
	wrapper_data.diff = diff;
	wrapper_data.encoding = NULL;

	if (file_cb != NULL)
	{
		real_file_cb = ggit_diff_file_callback_wrapper;
		wrapper_data.file_cb = file_cb;
	}

	if (hunk_cb != NULL)
	{
		real_hunk_cb = ggit_diff_hunk_callback_wrapper;
		wrapper_data.hunk_cb = hunk_cb;
	}

	if (line_cb != NULL)
	{
		real_line_cb = ggit_diff_line_callback_wrapper;
		wrapper_data.line_cb = line_cb;
	}

	ret = git_diff_foreach (_ggit_native_get (diff),
	                        real_file_cb, real_hunk_cb, real_line_cb,
	                        &wrapper_data);

	if (ret != GIT_OK)
	{
		_ggit_error_set (error, ret);
	}
}

/**
 * ggit_diff_print:
 * @diff: a #GgitDiff.
 * @type: a #GgitDiffFormatType.
 * @print_cb: (scope call) (closure user_data): a #GgitDiffLineCallback.
 * @user_data: callback user data.
 * @error: a #GError for error reporting, or %NULL.
 *
 * Iterates over @diff generating text output like "git diff".
 */
void
ggit_diff_print (GgitDiff              *diff,
                 GgitDiffFormatType     type,
                 GgitDiffLineCallback   print_cb,
                 gpointer              *user_data,
                 GError               **error)
{
	gint ret;
	CallbackWrapperData wrapper_data;

	g_return_if_fail (GGIT_IS_DIFF (diff));
	g_return_if_fail (print_cb != NULL);
	g_return_if_fail (error == NULL || *error == NULL);

	wrapper_data.user_data = user_data;
	wrapper_data.diff = diff;
	wrapper_data.encoding = NULL;

	wrapper_data.line_cb = print_cb;

	ret = git_diff_print (_ggit_native_get (diff), (git_diff_format_t)type,
	                      ggit_diff_line_callback_wrapper,
	                      &wrapper_data);

	if (ret != GIT_OK)
	{
		_ggit_error_set (error, ret);
	}
}

/**
 * ggit_diff_get_num_deltas:
 * @diff: a #GgitDiff.
 *
 * Get the number of deltas in the diff.
 *
 * Returns: the number of deltas.
 *
 **/
gint
ggit_diff_get_num_deltas (GgitDiff *diff)
{
	g_return_val_if_fail (GGIT_IS_DIFF (diff), 0);

	return git_diff_num_deltas (_ggit_native_get (diff));
}

/**
 * ggit_diff_blobs:
 * @old_blob: (allow-none): a #GgitBlob to diff from.
 * @old_as_path: (allow-none): treat @old_blob as if it had this filename, or %NULL,
 * @new_blob: (allow-none): a #GgitBlob to diff to.
 * @new_as_path: (allow-none): treat @new_blob as if it had this filename, or %NULL,
 * @diff_options: (allow-none): a #GgitDiffOptions, or %NULL.
 * @file_cb: (allow-none) (scope call) (closure user_data):
 *  a #GgitDiffFileCallback.
 * @hunk_cb: (allow-none) (scope call) (closure user_data):
 *  a #GgitDiffHunkCallback.
 * @line_cb: (allow-none) (scope call) (closure user_data):
 *  a #GgitDiffLineCallback.
 * @user_data: callback user data.
 * @error: a #GError for error reporting, or %NULL.
 *
 * Iterates over the diff calling @file_cb, @hunk_cb and @line_cb.
 *
 * The #GgitDiffFile mode always be 0, path will be %NULL and when a blob is
 * %NULL the oid will be 0.
 *
 * If @diff_options is %NULL then the defaults specified in
 * ggit_diff_options_new() are used.
 */
void
ggit_diff_blobs (GgitBlob              *old_blob,
                 const gchar           *old_as_path,
                 GgitBlob              *new_blob,
                 const gchar           *new_as_path,
                 GgitDiffOptions       *diff_options,
                 GgitDiffFileCallback   file_cb,
                 GgitDiffHunkCallback   hunk_cb,
                 GgitDiffLineCallback   line_cb,
                 gpointer              *user_data,
                 GError               **error)
{
	gint ret;
	const git_diff_options *gdiff_options;
	CallbackWrapperData wrapper_data;
	git_diff_file_cb real_file_cb = NULL;
	git_diff_hunk_cb real_hunk_cb = NULL;
	git_diff_line_cb real_line_cb = NULL;

	g_return_if_fail (error == NULL || *error == NULL);

	gdiff_options = _ggit_diff_options_get_diff_options (diff_options);

	wrapper_data.user_data = user_data;
	wrapper_data.diff = NULL;
	wrapper_data.encoding = NULL;

	if (file_cb != NULL)
	{
		real_file_cb = ggit_diff_file_callback_wrapper;
		wrapper_data.file_cb = file_cb;
	}

	if (hunk_cb != NULL)
	{
		real_hunk_cb = ggit_diff_hunk_callback_wrapper;
		wrapper_data.hunk_cb = hunk_cb;
	}

	if (line_cb != NULL)
	{
		real_line_cb = ggit_diff_line_callback_wrapper;
		wrapper_data.line_cb = line_cb;
	}

	ret = git_diff_blobs (old_blob ? _ggit_native_get (old_blob) : NULL,
	                      old_as_path,
	                      new_blob ? _ggit_native_get (new_blob) : NULL,
	                      new_as_path,
	                      (git_diff_options *) gdiff_options,
	                      real_file_cb, real_hunk_cb, real_line_cb,
	                      &wrapper_data);

	if (ret != GIT_OK)
	{
		_ggit_error_set (error, ret);
	}
}

/**
 * ggit_diff_blob_to_buffer:
 * @old_blob: (allow-none): a #GgitBlob to diff from.
 * @old_as_path: (allow-none): treat @old_blob as if it had this filename, or %NULL,
 * @buffer: (allow-none) (array length=buffer_len): a buffer to diff to.
 * @buffer_len: length of @buffer.
 * @buffer_as_path: (allow-none): treat @buffer as if it had this filename, or %NULL,
 * @diff_options: (allow-none): a #GgitDiffOptions, or %NULL.
 * @file_cb: (allow-none) (scope call) (closure user_data):
 *  a #GgitDiffFileCallback.
 * @hunk_cb: (allow-none) (scope call) (closure user_data):
 *  a #GgitDiffHunkCallback.
 * @line_cb: (allow-none) (scope call) (closure user_data):
 *  a #GgitDiffLineCallback.
 * @user_data: callback user data.
 * @error: a #GError for error reporting, or %NULL.
 *
 * Same as ggit_diff_blobs() but using a buffer.
 */
void
ggit_diff_blob_to_buffer (GgitBlob              *old_blob,
                          const gchar           *old_as_path,
                          const gchar           *buffer,
                          gsize                  buffer_len,
                          const gchar           *buffer_as_path,
                          GgitDiffOptions       *diff_options,
                          GgitDiffFileCallback   file_cb,
                          GgitDiffHunkCallback   hunk_cb,
                          GgitDiffLineCallback   line_cb,
                          gpointer              *user_data,
                          GError               **error)
{
	gint ret;
	const git_diff_options *gdiff_options;
	CallbackWrapperData wrapper_data;
	git_diff_file_cb real_file_cb = NULL;
	git_diff_hunk_cb real_hunk_cb = NULL;
	git_diff_line_cb real_line_cb = NULL;

	g_return_if_fail (error == NULL || *error == NULL);

	gdiff_options = _ggit_diff_options_get_diff_options (diff_options);

	wrapper_data.user_data = user_data;
	wrapper_data.diff = NULL;
	wrapper_data.encoding = NULL;

	if (file_cb != NULL)
	{
		real_file_cb = ggit_diff_file_callback_wrapper;
		wrapper_data.file_cb = file_cb;
	}

	if (hunk_cb != NULL)
	{
		real_hunk_cb = ggit_diff_hunk_callback_wrapper;
		wrapper_data.hunk_cb = hunk_cb;
	}

	if (line_cb != NULL)
	{
		real_line_cb = ggit_diff_line_callback_wrapper;
		wrapper_data.line_cb = line_cb;
	}

	ret = git_diff_blob_to_buffer (old_blob ? _ggit_native_get (old_blob) : NULL,
	                               old_as_path,
	                               buffer,
	                               buffer_len,
	                               buffer_as_path,
	                               (git_diff_options *) gdiff_options,
	                               real_file_cb, real_hunk_cb, real_line_cb,
	                               &wrapper_data);

	if (ret != GIT_OK)
	{
		_ggit_error_set (error, ret);
	}
}

/* ex:set ts=8 noet: */
