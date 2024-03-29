/*
 * ggit-config-entry.h
 * This file is part of libgit2-glib
 *
 * Copyright (C) 2012 - Ignacio Casal Quinteiro
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

#ifndef __GGIT_CONFIG_ENTRY_H__
#define __GGIT_CONFIG_ENTRY_H__

#include <glib-object.h>
#include <git2.h>

#include "ggit-types.h"

G_BEGIN_DECLS

#define GGIT_TYPE_CONFIG_ENTRY       (ggit_config_entry_get_type ())
#define GGIT_CONFIG_ENTRY(obj)       ((GgitConfigEntry *)obj)

GType              ggit_config_entry_get_type       (void) G_GNUC_CONST;

GgitConfigEntry  *_ggit_config_entry_wrap           (const git_config_entry *entry);

GgitConfigEntry   *ggit_config_entry_ref            (GgitConfigEntry        *entry);
void               ggit_config_entry_unref          (GgitConfigEntry        *entry);

const gchar       *ggit_config_entry_get_name       (GgitConfigEntry        *entry);

const gchar       *ggit_config_entry_get_value      (GgitConfigEntry        *entry);

GgitConfigLevel    ggit_config_entry_get_level      (GgitConfigEntry        *entry);

G_END_DECLS

#endif /* __GGIT_CONFIG_ENTRY_H__ */

/* ex:set ts=8 noet: */
