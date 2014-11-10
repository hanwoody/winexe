/* 
   Unix SMB/CIFS implementation.
   GTK+ registry frontend
   
   Copyright (C) Jelmer Vernooij 2004-2005

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

#include "includes.h"
#include "lib/registry/registry.h"
#include "gtk/common/gtk-smb.h"
#include "lib/events/events.h"
#include "lib/registry/reg_backend_rpc.h"
#include "auth/credentials/credentials.h"

static GtkTreeStore *store_keys;
static GtkListStore *store_vals;
static GtkWidget *tree_keys;
static GtkWidget *tree_vals;
static GtkWidget *mainwin;
static GtkWidget *mnu_add_key, *mnu_set_value, *mnu_del_key, *mnu_del_value, *mnu_find;
static TALLOC_CTX *mem_ctx; /* FIXME: Split up */

static GtkWidget *save;
static GtkWidget *save_as;
static GtkWidget* create_openfilewin (void);
static GtkWidget* create_savefilewin (void);
struct registry_context *registry = NULL;
struct registry_key *current_key = NULL;

static GtkWidget* create_FindDialog (void)
{
  GtkWidget *FindDialog;
  GtkWidget *dialog_vbox2;
  GtkWidget *vbox1;
  GtkWidget *hbox1;
  GtkWidget *label6;
  GtkWidget *entry_pattern;
  GtkWidget *frame3;
  GtkWidget *alignment3;
  GtkWidget *vbox2;
  GtkWidget *checkbutton1;
  GtkWidget *checkbutton2;
  GtkWidget *checkbutton3;
  GtkWidget *label7;
  GtkWidget *dialog_action_area2;
  GtkWidget *cancelbutton2;
  GtkWidget *okbutton2;

  FindDialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (FindDialog), "Find Key or Value");
  gtk_window_set_resizable (GTK_WINDOW (FindDialog), FALSE);
  gtk_window_set_type_hint (GTK_WINDOW (FindDialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox2 = GTK_DIALOG (FindDialog)->vbox;

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (dialog_vbox2), vbox1, TRUE, TRUE, 0);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

  label6 = gtk_label_new ("Find String");
  gtk_box_pack_start (GTK_BOX (hbox1), label6, FALSE, FALSE, 0);

  entry_pattern = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox1), entry_pattern, TRUE, TRUE, 0);

  frame3 = gtk_frame_new (NULL);
  gtk_box_pack_start (GTK_BOX (vbox1), frame3, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame3), GTK_SHADOW_NONE);

  alignment3 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_container_add (GTK_CONTAINER (frame3), alignment3);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment3), 0, 0, 12, 0);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (alignment3), vbox2);

  checkbutton1 = gtk_check_button_new_with_mnemonic ("_Key Names");
  gtk_box_pack_start (GTK_BOX (vbox2), checkbutton1, FALSE, FALSE, 0);

  checkbutton2 = gtk_check_button_new_with_mnemonic ("_Value Names");
  gtk_box_pack_start (GTK_BOX (vbox2), checkbutton2, FALSE, FALSE, 0);

  checkbutton3 = gtk_check_button_new_with_mnemonic ("Value _Data");
  gtk_box_pack_start (GTK_BOX (vbox2), checkbutton3, FALSE, FALSE, 0);

  label7 = gtk_label_new ("<b>Search in</b>");
  gtk_frame_set_label_widget (GTK_FRAME (frame3), label7);
  gtk_label_set_use_markup (GTK_LABEL (label7), TRUE);

  dialog_action_area2 = GTK_DIALOG (FindDialog)->action_area;
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area2), GTK_BUTTONBOX_END);

  cancelbutton2 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_dialog_add_action_widget (GTK_DIALOG (FindDialog), cancelbutton2, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelbutton2, GTK_CAN_DEFAULT);

  okbutton2 = gtk_button_new_from_stock ("gtk-ok");
  gtk_dialog_add_action_widget (GTK_DIALOG (FindDialog), okbutton2, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton2, GTK_CAN_DEFAULT);

  gtk_widget_show_all (dialog_vbox2);

  return FindDialog;
}

static GtkWidget* create_SetValueDialog (GtkWidget **entry_name, GtkWidget **entry_type, GtkWidget **entry_data)
{
  GtkWidget *SetValueDialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *table1;
  GtkWidget *label3;
  GtkWidget *label4;
  GtkWidget *label5;
  GtkWidget *entry_value_name;
  GtkWidget *value_data;
  GtkWidget *combo_data_type;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  SetValueDialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (SetValueDialog), "Set Registry Value");
  gtk_window_set_position (GTK_WINDOW (SetValueDialog), GTK_WIN_POS_CENTER);
  gtk_window_set_resizable (GTK_WINDOW (SetValueDialog), FALSE);
  gtk_window_set_type_hint (GTK_WINDOW (SetValueDialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (SetValueDialog)->vbox;

  table1 = gtk_table_new (3, 2, FALSE);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), table1, TRUE, TRUE, 0);

  label3 = gtk_label_new ("Value name:");
  gtk_table_attach (GTK_TABLE (table1), label3, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);

  label4 = gtk_label_new ("Data Type:");
  gtk_table_attach (GTK_TABLE (table1), label4, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label4), 0, 0.5);

  label5 = gtk_label_new ("Data:");
  gtk_table_attach (GTK_TABLE (table1), label5, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label5), 0, 0.5);

  *entry_name = entry_value_name = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table1), entry_value_name, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  *entry_data = value_data = gtk_entry_new ();
  gtk_table_attach (GTK_TABLE (table1), value_data, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  *entry_type = combo_data_type = gtk_combo_box_new_text ();

  gtk_combo_box_append_text(GTK_COMBO_BOX(combo_data_type), "REG_NONE");
  gtk_combo_box_append_text(GTK_COMBO_BOX(combo_data_type), "REG_SZ");
  gtk_combo_box_append_text(GTK_COMBO_BOX(combo_data_type), "REG_EXPAND_SZ");
  gtk_combo_box_append_text(GTK_COMBO_BOX(combo_data_type), "REG_BINARY");
  gtk_combo_box_append_text(GTK_COMBO_BOX(combo_data_type), "REG_DWORD_LE");
  gtk_combo_box_append_text(GTK_COMBO_BOX(combo_data_type), "REG_DWORD_BE");
  
  gtk_table_attach (GTK_TABLE (table1), combo_data_type, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  dialog_action_area1 = GTK_DIALOG (SetValueDialog)->action_area;
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  cancelbutton1 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_dialog_add_action_widget (GTK_DIALOG (SetValueDialog), cancelbutton1, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelbutton1, GTK_CAN_DEFAULT);

  okbutton1 = gtk_button_new_from_stock ("gtk-ok");
  gtk_dialog_add_action_widget (GTK_DIALOG (SetValueDialog), okbutton1, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);

  gtk_widget_show_all (dialog_vbox1);

  return SetValueDialog;
}

static GtkWidget* create_NewKeyDialog (GtkWidget **name_entry)
{
  GtkWidget *NewKeyDialog;
  GtkWidget *dialog_vbox2;
  GtkWidget *hbox1;
  GtkWidget *label6;
  GtkWidget *entry_key_name;
  GtkWidget *dialog_action_area2;
  GtkWidget *cancelbutton2;
  GtkWidget *okbutton2;

  NewKeyDialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (NewKeyDialog), "New Registry Key");
  gtk_window_set_position (GTK_WINDOW (NewKeyDialog), GTK_WIN_POS_CENTER);
  gtk_window_set_resizable (GTK_WINDOW (NewKeyDialog), FALSE);
  gtk_window_set_type_hint (GTK_WINDOW (NewKeyDialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox2 = GTK_DIALOG (NewKeyDialog)->vbox;

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (dialog_vbox2), hbox1, TRUE, TRUE, 0);

  label6 = gtk_label_new ("Name:");
  gtk_box_pack_start (GTK_BOX (hbox1), label6, FALSE, FALSE, 0);

  entry_key_name = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox1), entry_key_name, TRUE, TRUE, 0);

  dialog_action_area2 = GTK_DIALOG (NewKeyDialog)->action_area;
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area2), GTK_BUTTONBOX_END);

  *name_entry = entry_key_name;

  cancelbutton2 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_dialog_add_action_widget (GTK_DIALOG (NewKeyDialog), cancelbutton2, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelbutton2, GTK_CAN_DEFAULT);

  okbutton2 = gtk_button_new_from_stock ("gtk-ok");
  gtk_dialog_add_action_widget (GTK_DIALOG (NewKeyDialog), okbutton2, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton2, GTK_CAN_DEFAULT);

  gtk_widget_show_all (dialog_vbox2);

  return NewKeyDialog;
}

static void expand_key(GtkTreeView *treeview, GtkTreeIter *parent, GtkTreePath *arg2)
{
	GtkTreeIter firstiter, iter, tmpiter;
	struct registry_key *k, *sub;
	char *name;
	WERROR error;
	int i;

	gtk_tree_model_iter_children(GTK_TREE_MODEL(store_keys), &firstiter, parent);

	/* See if this row has ever had a name gtk_tree_store_set()'ed to it.
       	   If not, read the directory contents */
	gtk_tree_model_get(GTK_TREE_MODEL(store_keys), &firstiter, 0, &name, -1);

	if(name) return;

	gtk_tree_model_get(GTK_TREE_MODEL(store_keys), parent, 1, &k, -1);

	g_assert(k);
	
	for(i = 0; W_ERROR_IS_OK(error = reg_key_get_subkey_by_index(mem_ctx, k, i, &sub)); i++) {
		uint32_t count;
		/* Replace the blank child with the first directory entry
           You may be tempted to remove the blank child node and then 
           append a new one.  Don't.  If you remove the blank child 
           node GTK gets confused and won't expand the parent row. */

		if(i == 0) {
			iter = firstiter;
		} else {
			gtk_tree_store_append(store_keys, &iter, parent);
		}
		gtk_tree_store_set (store_keys,
					    &iter, 
						0,
						sub->name,
						1, 
						sub,
						-1);
		
		if(W_ERROR_IS_OK(reg_key_num_subkeys(sub, &count)) && count > 0) 
			gtk_tree_store_append(store_keys, &tmpiter, &iter);
	}

	if(!W_ERROR_EQUAL(error, WERR_NO_MORE_ITEMS)) { 
		gtk_show_werror(mainwin, "While enumerating subkeys", error);
	}
}

static void registry_load_hive(struct registry_key *root)
{
	GtkTreeIter iter, tmpiter;
	gtk_list_store_clear(store_vals);
	/* Add the root */
	gtk_tree_store_append(store_keys, &iter, NULL);
	gtk_tree_store_set (store_keys,
				    &iter, 
					0,
					root->name?root->name:"",
					1,
					root,
					-1);

	gtk_tree_store_append(store_keys, &tmpiter, &iter);

  	gtk_widget_set_sensitive( save, True );
  	gtk_widget_set_sensitive( save_as, True );
}

static void registry_load_root(void) 
{
	struct registry_key *root;
	uint32_t i = 0;
	if(!registry) return;

	gtk_list_store_clear(store_vals);
	gtk_tree_store_clear(store_keys);

	for(i = HKEY_CLASSES_ROOT; i <= HKEY_PERFORMANCE_NLSTEXT; i++) 
	{
		if (!W_ERROR_IS_OK(reg_get_predefined_key(registry, i, &root))) { continue; }

		registry_load_hive(root);
	}
}

static void on_open_file_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *openfilewin;
	gint result;
	char *filename, *tmp;
	struct registry_key *root;
	WERROR error;

	openfilewin = create_openfilewin();

	result = gtk_dialog_run(GTK_DIALOG(openfilewin));

	switch(result) {
	case GTK_RESPONSE_OK:
		filename = strdup(gtk_file_selection_get_filename(GTK_FILE_SELECTION(openfilewin)));
		error = reg_open_hive(NULL, user_data, filename, NULL, NULL, &root);
		if(!W_ERROR_IS_OK(error)) {
			gtk_show_werror(mainwin, "Error while opening hive", error);
			break;
		}

		tmp = g_strdup_printf("Registry Editor - %s", filename);
		gtk_window_set_title (GTK_WINDOW (mainwin), tmp);
		g_free(tmp);
		gtk_tree_store_clear(store_keys);
		registry_load_hive(root);
		break;
	default:
		break;
	}

	gtk_widget_destroy(openfilewin);
}

static void on_open_gconf_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	struct registry_key *root;
	WERROR error = reg_open_hive(NULL, "gconf", NULL, NULL, NULL, &root);
	if(!W_ERROR_IS_OK(error)) {
		gtk_show_werror(mainwin, "Error while opening GConf", error);
		return;
	}

	gtk_window_set_title (GTK_WINDOW (mainwin), "Registry Editor - GConf");

	gtk_tree_store_clear(store_keys);
	registry_load_hive(root);
}

static void on_open_local_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	WERROR error = reg_open_local(NULL, &registry, NULL, NULL);
	if(!W_ERROR_IS_OK(error)) {
		gtk_show_werror(mainwin, "Error while opening local registry", error);
		return;
	}
	registry_load_root();
}

static void on_open_remote_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	char *tmp;
	GtkWidget *rpcwin = GTK_WIDGET(gtk_rpc_binding_dialog_new(NULL));
	gint result = gtk_dialog_run(GTK_DIALOG(rpcwin));
	WERROR error;
	struct cli_credentials *creds;
	
	if(result != GTK_RESPONSE_ACCEPT)
	{
		gtk_widget_destroy(rpcwin);
		return;
	}

	creds = cli_credentials_init(mem_ctx);
	cli_credentials_guess(creds);
	cli_credentials_set_gtk_callbacks(creds);

	error = reg_open_remote(&registry, 
				NULL,
				creds,
				gtk_rpc_binding_dialog_get_binding_string(GTK_RPC_BINDING_DIALOG(rpcwin), mem_ctx),
				NULL);

	if(!W_ERROR_IS_OK(error)) {
		gtk_show_werror(mainwin, "Error while opening remote registry", error);
		gtk_widget_destroy(rpcwin);
		return;
	}

	tmp = g_strdup_printf("Registry Editor - Remote Registry at %s", gtk_rpc_binding_dialog_get_host(GTK_RPC_BINDING_DIALOG(rpcwin)));
	gtk_window_set_title (GTK_WINDOW (mainwin), tmp);
	g_free(tmp);

	registry_load_root();


	gtk_widget_destroy(rpcwin);
}


static void on_save_as_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	gint result;
	WERROR error = WERR_OK;
	GtkWidget *savefilewin = create_savefilewin();
	result = gtk_dialog_run(GTK_DIALOG(savefilewin));
	switch(result) {
	case GTK_RESPONSE_OK:
	/* FIXME:		error = reg_dump(registry, gtk_file_selection_get_filename(GTK_FILE_SELECTION(savefilewin))); */
		if(!W_ERROR_IS_OK(error)) {
			gtk_show_werror(mainwin, "Error while saving as", error);
		}
		break;

	default:
		break;

	}
	gtk_widget_destroy(savefilewin);
}


static void on_quit_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_main_quit();
}


static void on_delete_value_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	WERROR error;
	GtkTreeIter iter;
	const char *value;

	if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_vals)), NULL, &iter)) {
		return;
	}

	gtk_tree_model_get(GTK_TREE_MODEL(store_vals), &iter, 0, &value, -1);
	
	error = reg_del_value(current_key, value);

	if (!W_ERROR_IS_OK(error)) {
		gtk_show_werror(NULL, "Error while deleting value", error);
		return;
	}
}

static void on_delete_key_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	WERROR error;
	GtkTreeIter iter, parentiter;
	struct registry_key *parent_key;

	if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_keys)), NULL, &iter)) {
		return;
	}

	if (!gtk_tree_model_iter_parent(GTK_TREE_MODEL(store_keys), &parentiter, &iter)) {
		return;
	}
	
	gtk_tree_model_get(GTK_TREE_MODEL(store_keys), &parentiter, 1, &parent_key, -1);
	
	error = reg_key_del(parent_key, current_key->name);

	if (!W_ERROR_IS_OK(error)) {
		gtk_show_werror(NULL, "Error while deleting key", error);
		return;
	}
}

static void on_add_key_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *entry;
	GtkDialog *addwin = GTK_DIALOG(create_NewKeyDialog(&entry));
	gint result = gtk_dialog_run(addwin);

	if (result == GTK_RESPONSE_OK)
	{
		struct registry_key *newkey;
		WERROR error = reg_key_add_name(mem_ctx, current_key, gtk_entry_get_text(GTK_ENTRY(entry)), 0, NULL, &newkey);

		if (!W_ERROR_IS_OK(error)) {
			gtk_show_werror(NULL, "Error while adding key", error);
		}
	}

	gtk_widget_destroy(GTK_WIDGET(addwin));
}

static void on_value_activate(GtkTreeView *treeview, GtkTreePath *arg1,
         GtkTreeViewColumn *arg2, gpointer user_data)
{
	GtkWidget *entry_name, *entry_type, *entry_value;
	GtkDialog *addwin = GTK_DIALOG(create_SetValueDialog(&entry_name, &entry_type, &entry_value));
	GtkTreeIter iter;
	struct registry_value *value;
	gint result;

	gtk_tree_model_get_iter(GTK_TREE_MODEL(store_vals), &iter, arg1);

	gtk_tree_model_get(GTK_TREE_MODEL(store_vals), &iter, 3, &value, -1);

	gtk_widget_set_sensitive(entry_name, FALSE);
	gtk_entry_set_text(GTK_ENTRY(entry_name), value->name);
	gtk_entry_set_text(GTK_ENTRY(entry_value), reg_val_data_string(mem_ctx, value->data_type, &value->data));
	gtk_combo_box_set_active(GTK_COMBO_BOX(entry_type), value->data_type);
	
	result = gtk_dialog_run(addwin);
	if (result == GTK_RESPONSE_OK) 
	{
		WERROR error;
		DATA_BLOB data;
		uint32_t data_type;
		
		reg_string_to_val(mem_ctx,str_regtype(gtk_combo_box_get_active(GTK_COMBO_BOX(entry_type))), gtk_entry_get_text(GTK_ENTRY(entry_value)), &data_type, &data);
		
		error = reg_val_set(current_key, gtk_entry_get_text(GTK_ENTRY(entry_name)), data_type, data);

		if (!W_ERROR_IS_OK(error)) {
			gtk_show_werror(NULL, "Error while setting value", error);
		}
	}
	gtk_widget_destroy(GTK_WIDGET(addwin));
}

static void on_set_value_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *entry_name, *entry_type, *entry_value;
	GtkDialog *addwin = GTK_DIALOG(create_SetValueDialog(&entry_name, &entry_type, &entry_value));
	gint result = gtk_dialog_run(addwin);
	if (result == GTK_RESPONSE_OK) 
	{
		WERROR error;
		uint32_t data_type;
		DATA_BLOB data;
		
		reg_string_to_val(mem_ctx,str_regtype(gtk_combo_box_get_active(GTK_COMBO_BOX(entry_type))), gtk_entry_get_text(GTK_ENTRY(entry_value)), &data_type, &data);
		
		error = reg_val_set(current_key, gtk_entry_get_text(GTK_ENTRY(entry_name)), data_type, data);

		if (!W_ERROR_IS_OK(error)) {
			gtk_show_werror(NULL, "Error while setting value", error);
		}
	}
	gtk_widget_destroy(GTK_WIDGET(addwin));
}

static void on_find_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GtkDialog *findwin = GTK_DIALOG(create_FindDialog());
	/*gint result = gtk_dialog_run(findwin);
	FIXME */
	gtk_widget_destroy(GTK_WIDGET(findwin));
}

static void on_about_activate (GtkMenuItem *menuitem, gpointer user_data)
{
    GtkDialog *aboutwin = GTK_DIALOG(create_gtk_samba_about_dialog("gregedit"));
    gtk_dialog_run(aboutwin);
    gtk_widget_destroy(GTK_WIDGET(aboutwin));
}

static gboolean on_key_activate(GtkTreeSelection *selection,
                                             GtkTreeModel *model,
                                             GtkTreePath *path,
                                             gboolean path_currently_selected,
                                             gpointer data)
{
	int i;
	struct registry_key *k;
	struct registry_value *val;
	WERROR error;
	GtkTreeIter parent;

	gtk_widget_set_sensitive(mnu_add_key, !path_currently_selected);
	gtk_widget_set_sensitive(mnu_set_value, !path_currently_selected);
	gtk_widget_set_sensitive(mnu_del_key, !path_currently_selected);
	gtk_widget_set_sensitive(mnu_del_value, !path_currently_selected);
	gtk_widget_set_sensitive(mnu_find, !path_currently_selected);

	if(path_currently_selected) { 
		current_key = NULL; 
		return TRUE; 
	}

	gtk_tree_model_get_iter(GTK_TREE_MODEL(store_keys), &parent, path);
	gtk_tree_model_get(GTK_TREE_MODEL(store_keys), &parent, 1, &k, -1);

	current_key = k;

	if (!k) return FALSE;

	gtk_list_store_clear(store_vals);

	for(i = 0; W_ERROR_IS_OK(error = reg_key_get_value_by_index(mem_ctx, k, i, &val)); i++) {
		GtkTreeIter iter;
		gtk_list_store_append(store_vals, &iter);
		gtk_list_store_set (store_vals,
					    &iter, 
						0,
						val->name,
						1,
						str_regtype(val->data_type),
						2,
						reg_val_data_string(mem_ctx, val->data_type, &val->data),
						3, 
						val,
						-1);
	}

	if(!W_ERROR_EQUAL(error, WERR_NO_MORE_ITEMS)) {
		 gtk_show_werror(mainwin, "Error while enumerating values",  error);
		 return FALSE;
	}
	return TRUE;
}

static GtkWidget* create_mainwindow(void)
{
	GtkWidget *vbox1;
	GtkWidget *menubar;
	GtkWidget *menu_file;
	GtkWidget *menu_file_menu;
	GtkWidget *open_nt4;
	GtkWidget *open_ldb;
	GtkWidget *open_w95;
	GtkWidget *open_gconf;
	GtkWidget *open_remote;
	GtkWidget *open_local;
	GtkWidget *separatormenuitem1;
	GtkWidget *quit;
	GtkWidget *men_key;
	GtkWidget *men_key_menu;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *curcol;
	GtkWidget *help;
	GtkWidget *help_menu;
	GtkWidget *about;
	GtkWidget *hbox1;
	GtkWidget *scrolledwindow1;
	GtkWidget *scrolledwindow2;
	GtkWidget *statusbar;
	GtkAccelGroup *accel_group;

	accel_group = gtk_accel_group_new ();

	mainwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (mainwin), "Registry editor");
	gtk_window_set_default_size (GTK_WINDOW (mainwin), 642, 562);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (mainwin), vbox1);

	menubar = gtk_menu_bar_new ();
	gtk_box_pack_start (GTK_BOX (vbox1), menubar, FALSE, FALSE, 0);

	menu_file = gtk_menu_item_new_with_mnemonic ("_File");
	gtk_container_add (GTK_CONTAINER (menubar), menu_file);

	menu_file_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_file), menu_file_menu);

	open_local = gtk_menu_item_new_with_mnemonic ("Open _Local");
	gtk_container_add (GTK_CONTAINER (menu_file_menu), open_local);
	g_signal_connect ((gpointer) open_local, "activate",
					  	  G_CALLBACK (on_open_local_activate), NULL);

	if(reg_has_backend("rpc")) {
		open_remote = gtk_menu_item_new_with_mnemonic ("Open _Remote");
		gtk_container_add (GTK_CONTAINER (menu_file_menu), open_remote);

		g_signal_connect ((gpointer) open_remote, "activate",
						  G_CALLBACK (on_open_remote_activate),
						  NULL);
	}

	separatormenuitem1 = gtk_menu_item_new ();
	gtk_container_add (GTK_CONTAINER (menu_file_menu), separatormenuitem1);
	gtk_widget_set_sensitive (separatormenuitem1, FALSE);


	if(reg_has_backend("nt4")) {
		open_nt4 = gtk_image_menu_item_new_with_mnemonic("Open _NT4 file");
		gtk_container_add (GTK_CONTAINER (menu_file_menu), open_nt4);

		g_signal_connect(open_nt4, "activate",
				 G_CALLBACK (on_open_file_activate),
				 discard_const_p(char, "nt4"));
	}

	if(reg_has_backend("w95")) {
		open_w95 = gtk_image_menu_item_new_with_mnemonic("Open Win_9x file");
		gtk_container_add (GTK_CONTAINER (menu_file_menu), open_w95);

		g_signal_connect (open_w95, "activate",
				  G_CALLBACK (on_open_file_activate),
				  discard_const_p(char, "w95"));
	}

	if(reg_has_backend("gconf")) {
		open_gconf = gtk_image_menu_item_new_with_mnemonic ("Open _GConf");
		gtk_container_add (GTK_CONTAINER (menu_file_menu), open_gconf);

		g_signal_connect ((gpointer) open_gconf, "activate",
						  G_CALLBACK (on_open_gconf_activate),
						  NULL);
	}

	if(reg_has_backend("ldb")) {
		open_ldb = gtk_image_menu_item_new_with_mnemonic("Open _LDB file");
		gtk_container_add (GTK_CONTAINER (menu_file_menu), open_ldb);

		g_signal_connect(open_ldb, "activate",
				 G_CALLBACK (on_open_file_activate),
				 discard_const_p(char, "ldb"));
	}

	separatormenuitem1 = gtk_menu_item_new ();
	gtk_container_add (GTK_CONTAINER (menu_file_menu), separatormenuitem1);
	gtk_widget_set_sensitive (separatormenuitem1, FALSE);

	save = gtk_image_menu_item_new_from_stock ("gtk-save", accel_group);
	gtk_widget_set_sensitive( save, False );
	gtk_container_add (GTK_CONTAINER (menu_file_menu), save);

	save_as = gtk_image_menu_item_new_from_stock ("gtk-save-as", accel_group);
	gtk_widget_set_sensitive( save_as, False );
	gtk_container_add (GTK_CONTAINER (menu_file_menu), save_as);

	separatormenuitem1 = gtk_menu_item_new ();
	gtk_container_add (GTK_CONTAINER (menu_file_menu), separatormenuitem1);
	gtk_widget_set_sensitive (separatormenuitem1, FALSE);

	quit = gtk_image_menu_item_new_from_stock ("gtk-quit", accel_group);
	gtk_container_add (GTK_CONTAINER (menu_file_menu), quit);

	men_key = gtk_menu_item_new_with_mnemonic ("_Key");
	gtk_container_add (GTK_CONTAINER (menubar), men_key);

	men_key_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (men_key), men_key_menu);

	mnu_add_key = gtk_image_menu_item_new_with_mnemonic("Add _Subkey");
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mnu_add_key), gtk_image_new_from_stock ("gtk-add", GTK_ICON_SIZE_MENU));

	gtk_widget_set_sensitive(mnu_add_key, False);
	gtk_container_add (GTK_CONTAINER (men_key_menu), mnu_add_key);

	mnu_set_value = gtk_image_menu_item_new_with_mnemonic("Set _Value");
	gtk_widget_set_sensitive(mnu_set_value, False);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mnu_set_value), gtk_image_new_from_stock ("gtk-add", GTK_ICON_SIZE_MENU));
	gtk_container_add (GTK_CONTAINER (men_key_menu), mnu_set_value);

	mnu_find = gtk_image_menu_item_new_from_stock ("gtk-find", accel_group);
	gtk_widget_set_sensitive(mnu_find, False);
	gtk_container_add (GTK_CONTAINER (men_key_menu), mnu_find);

	mnu_del_key = gtk_image_menu_item_new_with_mnemonic ("Delete Key"); 
	gtk_widget_set_sensitive(mnu_del_key, False);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mnu_del_value), gtk_image_new_from_stock ("gtk-delete", GTK_ICON_SIZE_MENU));
	gtk_container_add (GTK_CONTAINER (men_key_menu), mnu_del_key);

	mnu_del_value = gtk_image_menu_item_new_with_mnemonic ("Delete Value"); 
	gtk_widget_set_sensitive(mnu_del_value, False);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (mnu_del_value), gtk_image_new_from_stock ("gtk-delete", GTK_ICON_SIZE_MENU));
	gtk_container_add (GTK_CONTAINER (men_key_menu), mnu_del_value);


	help = gtk_menu_item_new_with_mnemonic ("_Help");
	gtk_container_add (GTK_CONTAINER (menubar), help);

	help_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (help), help_menu);

	about = gtk_menu_item_new_with_mnemonic ("_About");
	gtk_container_add (GTK_CONTAINER (help_menu), about);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (hbox1), scrolledwindow1, TRUE, TRUE, 0);

	tree_keys = gtk_tree_view_new ();

	/* Column names */
	curcol = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title(curcol, "Name");
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(curcol, renderer, True);

	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_keys), curcol);

	gtk_tree_view_column_add_attribute(curcol, renderer, "text", 0);
	gtk_container_add (GTK_CONTAINER (scrolledwindow1), tree_keys);
	store_keys = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree_keys), GTK_TREE_MODEL(store_keys));
	g_object_unref(store_keys);

	gtk_tree_selection_set_select_function (gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_keys)), on_key_activate, NULL, NULL);

	g_signal_connect ((gpointer) tree_keys, "row-expanded",
					  G_CALLBACK (expand_key),
					  NULL);


	scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (hbox1), scrolledwindow2, TRUE, TRUE, 0);

	tree_vals = gtk_tree_view_new ();
	/* Column names */

	curcol = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title(curcol, "Name");
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(curcol, renderer, True);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_vals), curcol);
	gtk_tree_view_column_add_attribute(curcol, renderer, "text", 0);

	curcol = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title(curcol, "Type");
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(curcol, renderer, True);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_vals), curcol);
	gtk_tree_view_column_add_attribute(curcol, renderer, "text", 1);

	curcol = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title(curcol, "Value");
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(curcol, renderer, True);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_vals), curcol);
	gtk_tree_view_column_add_attribute(curcol, renderer, "text", 2);


	gtk_container_add (GTK_CONTAINER (scrolledwindow2), tree_vals);

	store_vals = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree_vals), GTK_TREE_MODEL(store_vals));
	g_object_unref(store_vals);

	statusbar = gtk_statusbar_new ();
	gtk_box_pack_start (GTK_BOX (vbox1), statusbar, FALSE, FALSE, 0);
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar), FALSE);

	g_signal_connect ((gpointer) save_as, "activate",
					  G_CALLBACK (on_save_as_activate),
					  NULL);
	g_signal_connect ((gpointer) quit, "activate",
					  G_CALLBACK (on_quit_activate),
					  NULL);
	g_signal_connect ((gpointer) mnu_add_key, "activate",
					  G_CALLBACK (on_add_key_activate),
					  NULL);
	g_signal_connect ((gpointer) mnu_set_value, "activate",
					  G_CALLBACK (on_set_value_activate),
					  NULL);
	g_signal_connect ((gpointer) mnu_find, "activate",
					  G_CALLBACK (on_find_activate),
					  NULL);
	g_signal_connect ((gpointer) mnu_del_key, "activate",
					  G_CALLBACK (on_delete_key_activate),
					  NULL);
	g_signal_connect ((gpointer) mnu_del_value, "activate",
					  G_CALLBACK (on_delete_value_activate),
					  NULL);
	g_signal_connect ((gpointer) about, "activate",
					  G_CALLBACK (on_about_activate),
					  NULL);

	g_signal_connect ((gpointer) tree_vals, "row-activated",
					  G_CALLBACK (on_value_activate),
					  NULL);


	gtk_window_add_accel_group (GTK_WINDOW (mainwin), accel_group);

	return mainwin;
}

static GtkWidget* create_openfilewin (void)
{
	GtkWidget *openfilewin;
	GtkWidget *ok_button;
	GtkWidget *cancel_button;

	openfilewin = gtk_file_selection_new ("Select File");
	gtk_container_set_border_width (GTK_CONTAINER (openfilewin), 10);

	ok_button = GTK_FILE_SELECTION (openfilewin)->ok_button;
	GTK_WIDGET_SET_FLAGS (ok_button, GTK_CAN_DEFAULT);

	cancel_button = GTK_FILE_SELECTION (openfilewin)->cancel_button;
	GTK_WIDGET_SET_FLAGS (cancel_button, GTK_CAN_DEFAULT);

	return openfilewin;
}

static GtkWidget* create_savefilewin (void)
{
	GtkWidget *savefilewin;
	GtkWidget *ok_button;
	GtkWidget *cancel_button;

	savefilewin = gtk_file_selection_new ("Select File");
	gtk_container_set_border_width (GTK_CONTAINER (savefilewin), 10);

	ok_button = GTK_FILE_SELECTION (savefilewin)->ok_button;
	GTK_WIDGET_SET_FLAGS (ok_button, GTK_CAN_DEFAULT);

	cancel_button = GTK_FILE_SELECTION (savefilewin)->cancel_button;
	GTK_WIDGET_SET_FLAGS (cancel_button, GTK_CAN_DEFAULT);

	return savefilewin;
}

static int gregedit_load_defaults(void)
{
	WERROR error = reg_open_local(NULL, &registry, NULL, NULL);
	if(!W_ERROR_IS_OK(error)) {
		gtk_show_werror(mainwin, "Error while loading local registry", error);
		return -1;
	}
	registry_load_root();

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;

	lp_load();
	setup_logging(argv[0], DEBUG_STDERR);

	mem_ctx = talloc_init("gregedit");

	registry_init();

	gtk_init(&argc, &argv);
	mainwin = create_mainwindow();
	gtk_widget_show_all(mainwin);

	ret = gregedit_load_defaults();
	if (ret != 0) goto failed;

	ret = gtk_event_loop();

failed:
	talloc_free(mem_ctx);
	return ret;
}
