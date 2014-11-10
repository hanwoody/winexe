/* 
   Unix SMB/CIFS implementation.
   SMB-related GTK+ functions
   
   Copyright (C) Jelmer Vernooij 2004

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __GTK_SELECT_H__
#define __GTK_SELECT_H__

#define GTK_DISABLE_DEPRECATED
#include <gtk/gtk.h>

typedef struct _GtkSelectDomainDialog GtkSelectDomainDialog;

struct _GtkSelectDomainDialog 
{
	GtkDialog dialog;
	GtkWidget *entry_domain;
	GtkWidget *list_domains;
	GtkListStore *store_domains;
	TALLOC_CTX *mem_ctx;
	struct dcerpc_pipe *sam_pipe;
};

typedef struct _GtkSelectDomainDialogClass GtkSelectDomainDialogClass;

struct _GtkSelectDomainDialogClass
{
	GtkDialogClass parent_class;
};

#define GTK_SELECT_DOMAIN_DIALOG(obj)          GTK_CHECK_CAST (obj, gtk_select_domain_dialog_get_type (), GtkSelectDomainDialog)
#define GTK_SELECT_DOMAIN_DIALOG_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, gtk_select_domain_dialog_class_get_type (), GtkSelectDomainDialogClass)
#define IS_GTK_SELECT_DOMAIN_DIALOG(obj)       GTK_CHECK_TYPE (obj, gtk_select_domain_dialog_get_type ())

typedef struct _GtkSelectHostDialog GtkSelectHostDialog;

struct _GtkSelectHostDialog 
{
	GtkDialog dialog;
	GtkWidget *entry_host;
	GtkWidget *tree_host;
	GtkTreeStore *store_host;
	struct dcerpc_pipe *sam_pipe;
	TALLOC_CTX *mem_ctx;
};

typedef struct _GtkSelectHostDialogClass GtkSelectHostDialogClass;

struct _GtkSelectHostDialogClass
{
	GtkDialogClass parent_class;
};

#define GTK_SELECT_HOST_DIALOG(obj)          GTK_CHECK_CAST (obj, gtk_select_host_dialog_get_type (), GtkSelectHostDialog)
#define GTK_SELECT_HOST_DIALOG_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, gtk_select_host_dialog_class_get_type (), GtkSelectHostDialogClass)
#define IS_GTK_SELECT_HOST_DIALOG(obj)       GTK_CHECK_TYPE (obj, gtk_select_host_dialog_get_type ())

GtkWidget *gtk_select_domain_dialog_new (struct dcerpc_pipe *sam_pipe);
GType gtk_select_domain_dialog_get_type (void);
struct policy_handle gtk_select_domain_dialog_get_handle(GtkSelectDomainDialog *d);
GtkWidget *gtk_select_host_dialog_new (struct dcerpc_pipe *sam_pipe);
const char *gtk_select_host_dialog_get_host (GtkSelectHostDialog *d);
GType gtk_select_host_dialog_get_type (void);

struct dcerpc_interface_table;
struct dcerpc_pipe *gtk_connect_rpc_interface(TALLOC_CTX *mem_ctx, const struct dcerpc_interface_table *table);

#endif
