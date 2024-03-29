/*
 * ggit-repository.h
 * This file is part of libgit2-glib
 *
 * Copyright (C) 2011 - Ignacio Casal Quinteiro
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


#ifndef __GGIT_REPOSITORY_H__
#define __GGIT_REPOSITORY_H__

#include <glib-object.h>
#include <git2.h>
#include <gio/gio.h>
#include <libgit2-glib/ggit-config.h>
#include <libgit2-glib/ggit-index.h>
#include <libgit2-glib/ggit-types.h>
#include <libgit2-glib/ggit-native.h>
#include <libgit2-glib/ggit-object.h>
#include <libgit2-glib/ggit-tree.h>
#include <libgit2-glib/ggit-branch.h>
#include <libgit2-glib/ggit-blob-output-stream.h>

G_BEGIN_DECLS

#define GGIT_TYPE_REPOSITORY			(ggit_repository_get_type ())
#define GGIT_REPOSITORY(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GGIT_TYPE_REPOSITORY, GgitRepository))
#define GGIT_REPOSITORY_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GGIT_TYPE_REPOSITORY, GgitRepositoryClass))
#define GGIT_IS_REPOSITORY(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGIT_TYPE_REPOSITORY))
#define GGIT_IS_REPOSITORY_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), GGIT_TYPE_REPOSITORY))
#define GGIT_REPOSITORY_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGIT_TYPE_REPOSITORY, GgitRepositoryClass))

typedef struct _GgitRepositoryClass	GgitRepositoryClass;
typedef struct _GgitRepositoryPrivate	GgitRepositoryPrivate;

struct _GgitRepository
{
	/*< private >*/
	GgitNative parent;

	GgitRepositoryPrivate *priv;
};

/**
 * GgitRepositoryClass:
 * @parent_class: The parent class.
 *
 * The class structure for #GgitRepositoryClass.
 */
struct _GgitRepositoryClass
{
	/*< private >*/
	GgitNativeClass parent_class;
};

GType               ggit_repository_get_type          (void) G_GNUC_CONST;

GgitRepository     *_ggit_repository_wrap             (git_repository        *repository,
                                                       gboolean               owned);

git_repository     *_ggit_repository_get_repository   (GgitRepository        *repository);

GgitRepository     *ggit_repository_open              (GFile                 *location,
                                                       GError               **error);

GgitRepository     *ggit_repository_init_repository   (GFile                 *location,
                                                       gboolean               is_bare,
                                                       GError               **error);

GgitRepository     *ggit_repository_clone             (const gchar           *url,
                                                       GFile                 *location,
                                                       GgitCloneOptions      *options,
                                                       GError               **error);

GgitObject         *ggit_repository_lookup            (GgitRepository        *repository,
                                                       GgitOId               *oid,
                                                       GType                  gtype,
                                                       GError               **error);

GgitRef            *ggit_repository_lookup_reference  (GgitRepository        *repository,
                                                       const gchar           *name,
                                                       GError               **error);

GgitRef            *ggit_repository_create_reference  (GgitRepository        *repository,
                                                       const gchar           *name,
                                                       GgitOId               *oid,
                                                       GError               **error);

GgitRef            *ggit_repository_create_symbolic_reference
                                                      (GgitRepository        *repository,
                                                       const gchar           *name,
                                                       const gchar           *target,
                                                       GError               **error);

GgitBlobOutputStream *
                    ggit_repository_create_blob       (GgitRepository        *repository);


GgitOId            *ggit_repository_create_blob_from_buffer (
                                                       GgitRepository        *repository,
                                                       gconstpointer          buffer,
                                                       gsize                  size,
                                                       GError               **error);

GgitOId            *ggit_repository_create_blob_from_file (
                                                       GgitRepository        *repository,
                                                       GFile                 *file,
                                                       GError               **error);

GgitOId            *ggit_repository_create_blob_from_path (
                                                       GgitRepository        *repository,
                                                       const gchar           *path,
                                                       GError               **error);

GgitOId            *ggit_repository_create_commit     (GgitRepository        *repository,
                                                       const gchar           *update_ref,
                                                       GgitSignature         *author,
                                                       GgitSignature         *committer,
                                                       const gchar           *message_encoding,
                                                       const gchar           *message,
                                                       GgitTree              *tree,
                                                       GgitCommit           **parents,
                                                       gint                   parent_count,
                                                       GError               **error);

GgitOId            *ggit_repository_create_commit_from_oids (
                                                       GgitRepository        *repository,
                                                       const gchar           *update_ref,
                                                       GgitSignature         *author,
                                                       GgitSignature         *committer,
                                                       const gchar           *message_encoding,
                                                       const gchar           *message,
                                                       GgitOId               *tree,
                                                       GgitOId              **parents,
                                                       gint                   parent_count,
                                                       GError               **error);

GgitOId            *ggit_repository_create_tag        (GgitRepository        *repository,
                                                       const gchar           *tag_name,
                                                       GgitObject            *target,
                                                       GgitSignature         *tagger,
                                                       const gchar           *message,
                                                       GgitCreateFlags        flags,
                                                       GError               **error);

GgitOId            *ggit_repository_create_tag_from_buffer (
                                                       GgitRepository        *repository,
                                                       const gchar           *tag,
                                                       GgitCreateFlags        flags,
                                                       GError               **error);

GgitOId            *ggit_repository_create_tag_lightweight (
                                                       GgitRepository        *repository,
                                                       const gchar           *tag_name,
                                                       GgitObject            *target,
                                                       GgitCreateFlags        flags,
                                                       GError               **error);

GgitTreeBuilder    *ggit_repository_create_tree_builder_from_tree (
                                                       GgitRepository        *repository,
                                                       GgitTree              *tree,
                                                       GError               **error);

GgitTreeBuilder    *ggit_repository_create_tree_builder (
                                                       GgitRepository        *repository);

GgitIndexEntry     *ggit_repository_create_index_entry_for_file (
                                                       GgitRepository        *repository,
                                                       GFile                 *file,
                                                       GgitOId               *id,
                                                       GError               **error);

GgitIndexEntry     *ggit_repository_create_index_entry_for_path (
                                                       GgitRepository        *repository,
                                                       const gchar           *path,
                                                       GgitOId               *id,
                                                       GError               **error);

gchar             **ggit_repository_list_tags         (GgitRepository        *repository,
                                                       GError               **error);

GgitBranch         *ggit_repository_create_branch     (GgitRepository        *repository,
                                                       const gchar           *branch_name,
                                                       GgitObject            *target,
                                                       GgitCreateFlags        flags,
                                                       GError               **error);

GgitBranchEnumerator *ggit_repository_enumerate_branches (GgitRepository        *repository,
                                                          GgitBranchType         list_flags,
                                                          GError               **error);

GgitBranch         *ggit_repository_lookup_branch     (GgitRepository        *repository,
                                                       const gchar           *branch_name,
                                                       GgitBranchType         branch_type,
                                                       GError               **error);

GgitRemote         *ggit_repository_get_remote        (GgitRepository        *repository,
                                                       const gchar           *name,
                                                       GError               **error);

GgitRemote         *ggit_repository_create_remote     (GgitRepository        *repository,
                                                       const gchar           *name,
                                                       const gchar           *url,
                                                       GError               **error);

gchar             **ggit_repository_list_remotes      (GgitRepository        *repository,
                                                       GError               **error);

GgitRef            *ggit_repository_get_head          (GgitRepository        *repository,
                                                       GError               **error);

GFile              *ggit_repository_discover          (GFile                 *location,
                                                       GError               **error);

gboolean            ggit_repository_is_head_detached  (GgitRepository        *repository,
                                                       GError               **error);

gboolean            ggit_repository_is_head_unborn    (GgitRepository        *repository,
                                                       GError               **error);

gboolean            ggit_repository_is_empty          (GgitRepository        *repository,
                                                       GError               **error);

GFile              *ggit_repository_get_location      (GgitRepository        *repository);
GFile              *ggit_repository_get_workdir       (GgitRepository        *repository);

void                ggit_repository_set_workdir       (GgitRepository        *repository,
                                                       GFile                 *workdir,
                                                       gboolean               update_gitlink);

gboolean            ggit_repository_is_bare           (GgitRepository        *repository);

GgitStatusFlags     ggit_repository_file_status       (GgitRepository        *repository,
                                                       GFile                 *location,
                                                       GError               **error);

gboolean            ggit_repository_file_status_foreach
                                                     (GgitRepository         *repository,
                                                      GgitStatusOptions      *options,
                                                      GgitStatusCallback      callback,
                                                      gpointer                user_data,
                                                      GError                **error);

gboolean            ggit_repository_references_foreach_name (GgitRepository             *repository,
                                                             GgitReferencesNameCallback  callback,
                                                             gpointer                    user_data,
                                                             GError                    **error);

GgitConfig         *ggit_repository_get_config         (GgitRepository          *repository,
                                                        GError                 **error);

GgitIndex          *ggit_repository_get_index          (GgitRepository          *repository,
                                                        GError                 **error);

GgitSubmodule      *ggit_repository_lookup_submodule   (GgitRepository          *repository,
                                                        const gchar             *name,
                                                        GError                 **error);

gboolean            ggit_repository_submodule_foreach  (GgitRepository          *repository,
                                                        GgitSubmoduleCallback    callback,
                                                        gpointer                 user_data,
                                                        GError                 **error);

void                ggit_repository_reset              (GgitRepository          *repository,
                                                        GgitObject              *target,
                                                        GgitResetType            reset_type,
                                                        GError                 **error);

GgitObject         *ggit_repository_revparse          (GgitRepository        *repository,
                                                       const gchar           *spec,
                                                       GError               **error);

GgitOId            *ggit_repository_save_stash        (GgitRepository        *repository,
                                                       GgitSignature         *stasher,
                                                       const gchar           *message,
                                                       GgitStashFlags         flags,
                                                       GError               **error);

void                ggit_repository_drop_stash        (GgitRepository        *repository,
                                                       gsize                  index,
                                                       GError               **error);

gboolean            ggit_repository_stash_foreach     (GgitRepository        *repository,
                                                       GgitStashCallback      callback,
                                                       gpointer               user_data,
                                                       GError               **error);

void                ggit_repository_get_ahead_behind  (GgitRepository        *repository,
                                                       GgitOId               *local,
                                                       GgitOId               *upstream,
                                                       gsize                 *ahead,
                                                       gsize                 *behind,
                                                       GError               **error);

GgitBlame          *ggit_repository_blame_file        (GgitRepository        *repository,
                                                       GFile                 *file,
                                                       GgitBlameOptions      *blame_options,
                                                       GError               **error);

const gchar        *ggit_repository_get_attribute     (GgitRepository           *repository,
                                                       const gchar              *path,
                                                       const gchar              *name,
                                                       GgitAttributeCheckFlags   flags,
                                                       GError                  **error);

G_END_DECLS

#endif /* __GGIT_REPOSITORY_H__ */

/* ex:set ts=8 noet: */
