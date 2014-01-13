/*
 * ggit-commit.h
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


#ifndef __GGIT_COMMIT_H__
#define __GGIT_COMMIT_H__

#include <glib-object.h>
#include <git2.h>

#include <libgit2-glib/ggit-object.h>
#include <libgit2-glib/ggit-types.h>

G_BEGIN_DECLS

#define GGIT_TYPE_COMMIT		(ggit_commit_get_type ())
#define GGIT_COMMIT(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GGIT_TYPE_COMMIT, GgitCommit))
#define GGIT_COMMIT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GGIT_TYPE_COMMIT, GgitCommitClass))
#define GGIT_IS_COMMIT(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGIT_TYPE_COMMIT))
#define GGIT_IS_COMMIT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GGIT_TYPE_COMMIT))
#define GGIT_COMMIT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GGIT_TYPE_COMMIT, GgitCommitClass))

typedef struct _GgitCommitClass		GgitCommitClass;
typedef struct _GgitCommitPrivate	GgitCommitPrivate;

struct _GgitCommit
{
	/*< private >*/
	GgitObject parent;

	/* priv padding */
	GgitCommitPrivate *priv;
};

/**
 * GgitCommitClass:
 * @parent_class: The parent class.
 *
 * The class structure for #GgitCommitClass.
 */
struct _GgitCommitClass
{
	/*< private >*/
	GgitObjectClass parent_class;
};

GType                ggit_commit_get_type             (void) G_GNUC_CONST;
GType                ggit_commit_parents_get_type     (void) G_GNUC_CONST;

GgitCommit          *_ggit_commit_wrap                (git_commit        *commit,
                                                       gboolean           owned);

const gchar         *ggit_commit_get_message_encoding (GgitCommit        *commit);

const gchar         *ggit_commit_get_message          (GgitCommit        *commit);

const gchar         *ggit_commit_get_subject          (GgitCommit        *commit);

const gchar         *ggit_commit_get_message_body     (GgitCommit        *commit);

GgitSignature       *ggit_commit_get_committer        (GgitCommit        *commit);

GgitSignature       *ggit_commit_get_author           (GgitCommit        *commit);

GgitCommitParents   *ggit_commit_get_parents          (GgitCommit        *commit);



GgitCommitParents   *ggit_commit_parents_ref          (GgitCommitParents *parents);
void                 ggit_commit_parents_unref        (GgitCommitParents *parents);

guint                ggit_commit_parents_size         (GgitCommitParents *parents);

GgitCommit          *ggit_commit_parents_get          (GgitCommitParents *parents,
                                                       guint              idx);

GgitOId             *ggit_commit_parents_get_id       (GgitCommitParents *parents,
                                                       guint              idx);

GgitTree            *ggit_commit_get_tree             (GgitCommit        *commit);

GgitOId             *ggit_commit_get_tree_id          (GgitCommit        *commit);

GgitCommit          *ggit_commit_get_nth_ancestor     (GgitCommit        *commit,
                                                       guint              n,
                                                       GError           **error);

G_END_DECLS

#endif /* __GGIT_COMMIT_H__ */

/* ex:set ts=8 noet: */
