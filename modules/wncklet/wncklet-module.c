/*
 * Copyright (C) 2016-2018 Alberts Muktupāvels
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <glib/gi18n-lib.h>
#include <libgnome-panel/gp-module.h>
#include <libwnck/libwnck.h>

#include "showdesktop.h"
#include "window-list.h"
#include "window-menu.h"
#include "workspace-switcher.h"

static GpAppletInfo *
wncklet_get_applet_info (const gchar *applet)
{
  const gchar *name;
  const gchar *description;
  const gchar *icon;
  GpAppletInfo *info;

  if (g_strcmp0 (applet, "show-desktop") == 0)
    {
      name = _("Show Desktop");
      description = _("Hide application windows and show the desktop");
      icon = "user-desktop";
    }
  else if (g_strcmp0 (applet, "window-list") == 0)
    {
      name = _("Window List");
      description = _("Switch between open windows using buttons");
      icon = "gnome-panel-window-list";
    }
  else if (g_strcmp0 (applet, "window-menu") == 0)
    {
      name = _("Window Selector");
      description = _("Switch between open windows using a menu");
      icon = "gnome-panel-window-menu";
    }
  else if (g_strcmp0 (applet, "workspace-switcher") == 0)
    {
      name = _("Workspace Switcher");
      description = _("Switch between workspaces");
      icon = "gnome-panel-workspace-switcher";
    }
  else
    {
      g_assert_not_reached ();
      return NULL;
    }

  info = gp_applet_info_new (name, description, icon);

  gp_applet_info_set_backends (info, "x11");

  return info;
}

static GType
wncklet_get_applet_type (const gchar *applet)
{
  if (g_strcmp0 (applet, "show-desktop") == 0)
    {
      return SHOW_DESKTOP_TYPE_APPLET;
    }
  else if (g_strcmp0 (applet, "window-list") == 0)
    {
      return WINDOW_LIST_TYPE_APPLET;
    }
  else if (g_strcmp0 (applet, "window-menu") == 0)
    {
      return WINDOW_MENU_TYPE_APPLET;
    }
  else if (g_strcmp0 (applet, "workspace-switcher") == 0)
    {
      return WORKSPACE_SWITCHER_TYPE_APPLET;
    }

  g_assert_not_reached ();
  return G_TYPE_NONE;
}

static const gchar *
wncklet_get_applet_id_from_iid (const gchar *iid)
{
  if (g_strcmp0 (iid, "WnckletFactory::ShowDesktopApplet") == 0 ||
      g_strcmp0 (iid, "wncklet::show-desktop") == 0)
    return "show-desktop";
  else if (g_strcmp0 (iid, "WnckletFactory::WindowListApplet") == 0 ||
           g_strcmp0 (iid, "wncklet::window-list") == 0)
    return "window-list";
  else if (g_strcmp0 (iid, "WnckletFactory::WindowMenuApplet") == 0 ||
           g_strcmp0 (iid, "wncklet::window-menu") == 0)
    return "window-menu";
  else if (g_strcmp0 (iid, "WnckletFactory::WorkspaceSwitcherApplet") == 0 ||
           g_strcmp0 (iid, "wncklet::workspace-switcher") == 0)
    return "workspace-switcher";

  return NULL;
}

void
gp_module_load (GpModule *module)
{
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  gp_module_set_gettext_domain (module, GETTEXT_PACKAGE);

  wnck_set_client_type (WNCK_CLIENT_TYPE_PAGER);

  gp_module_set_abi_version (module, GP_MODULE_ABI_VERSION);

  gp_module_set_id (module, "org.gnome.gnome-panel.wncklet");
  gp_module_set_version (module, PACKAGE_VERSION);

  gp_module_set_applet_ids (module, "show-desktop", "window-list",
                            "window-menu", "workspace-switcher",
                            NULL);

  gp_module_set_compatibility (module, wncklet_get_applet_id_from_iid);
}

void
gp_module_get_applet_vtable (GpAppletVTable *vtable)
{
  *vtable = (GpAppletVTable) {
    wncklet_get_applet_info,
    wncklet_get_applet_type
  };
}