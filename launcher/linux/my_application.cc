/*
 * Copyright © Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 or 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "my_application.h"

#include <flutter_linux/flutter_linux.h>
#include <gtk-layer-shell.h>

#include "flutter/generated_plugin_registrant.h"

/// TODO: This is hardcoded for now
const gint LAUNCHER_SIZE_PX = 70;

struct _MyApplication {
  GtkApplication parent_instance;
  GtkWindow *window;
  char **dart_entrypoint_arguments;
};

G_DEFINE_TYPE(MyApplication, my_application, GTK_TYPE_APPLICATION)

static void my_application_activate(GApplication *application) {
  MyApplication *self = MY_APPLICATION(application);
  GtkWindow *window =
      GTK_WINDOW(gtk_application_window_new(GTK_APPLICATION(application)));

  gtk_layer_init_for_window(window);
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_TOP, TRUE);
  gtk_layer_auto_exclusive_zone_enable(window);
  gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_TOP);
  gtk_layer_set_exclusive_zone(window, LAUNCHER_SIZE_PX);

  // This is required as of gtk-layer-shell v0.8.1, but future releases
  // shouldn't need it See https://github.com/wmww/gtk-layer-shell/pull/166 for
  // details
  gtk_widget_set_size_request(GTK_WIDGET(window), LAUNCHER_SIZE_PX, -1);
  gtk_widget_show(GTK_WIDGET(window));

  g_autoptr(FlDartProject) project = fl_dart_project_new();
  fl_dart_project_set_dart_entrypoint_arguments(
      project, self->dart_entrypoint_arguments);

  FlView *view = fl_view_new(project);
  gtk_widget_show(GTK_WIDGET(view));
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(view));
  self->window = window;
  fl_register_plugins(FL_PLUGIN_REGISTRY(view));
  gtk_widget_grab_focus(GTK_WIDGET(view));
}

static gboolean my_application_local_command_line(GApplication *application,
                                                  gchar ***arguments,
                                                  int *exit_status) {
  MyApplication *self = MY_APPLICATION(application);
  // Strip out the first argument as it is the binary name.
  self->dart_entrypoint_arguments = g_strdupv(*arguments + 1);

  g_autoptr(GError) error = nullptr;
  if (!g_application_register(application, nullptr, &error)) {
    g_warning("Failed to register: %s", error->message);
    *exit_status = 1;
    return TRUE;
  }

  g_application_activate(application);
  *exit_status = 0;

  return TRUE;
}

static void my_application_dispose(GObject *object) {
  MyApplication *self = MY_APPLICATION(object);
  g_clear_pointer(&self->dart_entrypoint_arguments, g_strfreev);
  G_OBJECT_CLASS(my_application_parent_class)->dispose(object);
}

static void my_application_class_init(MyApplicationClass *klass) {
  G_APPLICATION_CLASS(klass)->activate = my_application_activate;
  G_APPLICATION_CLASS(klass)->local_command_line =
      my_application_local_command_line;
  G_OBJECT_CLASS(klass)->dispose = my_application_dispose;
}

static void my_application_init(MyApplication *self) {}

MyApplication *my_application_new() {
  return MY_APPLICATION(g_object_new(my_application_get_type(),
                                     "application-id", APPLICATION_ID, "flags",
                                     G_APPLICATION_NON_UNIQUE, nullptr));
}
