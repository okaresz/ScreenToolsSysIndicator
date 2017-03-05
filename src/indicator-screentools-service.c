#include <gio/gio.h>
#include "config.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
	GSimpleActionGroup *actions;
	GMenu *menu;
	guint actions_export_id;
	guint menu_export_id;
} IndicatorService;
static IndicatorService indicator = { 0 };

static void bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	IndicatorService *indicator = user_data;
	GError *error = NULL;

	indicator->actions_export_id = g_dbus_connection_export_action_group (connection,
																		DBUS_PATH,
																		G_ACTION_GROUP (indicator->actions),
																		&error);
	if (indicator->actions_export_id == 0)
	{
		g_warning ("cannot export action group: %s", error->message);
		g_error_free (error);
		return;
	}

	indicator->menu_export_id = g_dbus_connection_export_menu_model (connection,
																   DBUS_PATH "/desktop",
																   G_MENU_MODEL (indicator->menu),
																   &error);
	if (indicator->menu_export_id == 0)
	{
		g_warning ("cannot export menu: %s", error->message);
		g_error_free (error);
		return;
	}
}

static void name_lost (GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	IndicatorService *indicator = user_data;

	if (indicator->actions_export_id)
		{ g_dbus_connection_unexport_action_group (connection, indicator->actions_export_id); }

	if (indicator->menu_export_id)
		{ g_dbus_connection_unexport_menu_model (connection, indicator->menu_export_id); }
}

static void indicatorShownStateChange(GSimpleAction *action, GVariant *value, gpointer user_data)
{
	return;
	/*if(g_variant_get_boolean(value))
	{
		g_message("try to set slider to current brightness...");
		// set current brightness value on slider
		// get the current brightness:
		GError *error = NULL;
		GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SESSION,
														NULL,
														&error);
		
		if (connection == NULL)
			{ g_printerr("Error connecting to session D-Bus: %s\n", error->message); }
		else
		{
			error = NULL;
			GVariant *retValue = g_dbus_connection_call_sync(connection,
	                                           "org.gnome.SettingsDaemon",
	                                           "/org/gnome/SettingsDaemon/Power",
	                                           "org.gnome.SettingsDaemon.Power.Screen",
	                                           "GetPercentage",
	                                           NULL,
	                                           G_VARIANT_TYPE("(u)"),
	                                           G_DBUS_CALL_FLAGS_NONE,
	                                           -1,
	                                           NULL,
	                                           &error);
	                                           
	        if (retValue == NULL)
	        {
	        	if(error)
					g_printerr("Error invoking GetPercentage(): %s\n", error->message);
				else
					g_printerr("Error invoking GetPercentage()");
	        }
	        else
	        {
	        	uint32_t brightnessPercent = g_variant_get_uint32(g_variant_get_child_value(retValue,0));
	        	g_message("Got current brightness=%d",brightnessPercent);
	        	double brightnessNormalized = (double)brightnessPercent/100;
	        	GVariant *brightnessGVar = g_variant_new_double(brightnessNormalized);
	        	
	        	GAction *action = g_action_map_lookup_action(G_ACTION_MAP(indicator.actions),"brightness");
	        	g_action_change_state(action,brightnessGVar);
	        	
	        	g_message("Current brightness set: %s", g_variant_print(brightnessGVar,TRUE));

	        	//g_variant_unref(brightnessGVar);
	    		g_variant_unref(retValue);
	        }
	    }

        g_object_unref(connection);
        if(error)
			{ g_error_free(error); }
	}*/
}

static void rotateActivated(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	GVariant *gvar;
	g_object_get(action,"state",&gvar,NULL);

	char buf[70];
	snprintf(buf, 70, "xrandr --output eDP-1 --rotate %s", g_variant_get_string(gvar,NULL));
	int retCode = system(buf);
	if(retCode==0)
		{ g_message("Screen rotated %s", g_variant_get_string(gvar,NULL) ); }
	else
		{ g_printerr("Screen rotation to %s failed.", g_variant_get_string(gvar,NULL)); }

	g_variant_unref(gvar);
}

// Get current brightness from the Gnome Settings Daemon through D-Bus
static int32_t getCurrentBrightness()
{
	int32_t brightnessPercent = -1;
	GError *error = NULL;
	
	GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);

	if (connection == NULL)
		{ g_printerr("Error connecting to session D-Bus: %s\n", error->message); }
	else
	{
		if(error)
			{ g_error_free(error); }
		error = NULL;

		GVariant *retValue = g_dbus_connection_call_sync(connection,
														"org.gnome.SettingsDaemon",
														"/org/gnome/SettingsDaemon/Power",
														"org.gnome.SettingsDaemon.Power.Screen",
														"GetPercentage",
														NULL,
														G_VARIANT_TYPE("(u)"),
														G_DBUS_CALL_FLAGS_NONE,
														-1,
														NULL,
														&error);
                                           
        if (retValue == NULL)
        {
        	if(error)
				g_printerr("Error invoking GetPercentage(): %s\n", error->message);
			else
				g_printerr("Error invoking GetPercentage()");
        }
        else
        {
        	brightnessPercent = g_variant_get_uint32(g_variant_get_child_value(retValue,0));
    		g_variant_unref(retValue);
        }
    }

    g_object_unref(connection);
    if(error)
		{ g_error_free(error); }
    
    return brightnessPercent;
}

static void brightnessStateChange(GSimpleAction *action, GVariant *brightnessVal, gpointer user_data)
{
	//g_message("brightnessStateChange: %f",g_variant_get_double(brightnessVal));
	
	GError *error = NULL;
	GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SESSION,
													NULL, /* GCancellable */
													&error);
	if (connection == NULL)
	{
		g_printerr("Error connecting to session D-Bus: %s\n", error->message);
	}
	else
	{
		uint32_t brightnessUint32 = (uint32_t)(g_variant_get_double(brightnessVal)*100);
		GVariant *setBrightnessVal = g_variant_new_uint32(brightnessUint32);
		GVariant* params[1] = {setBrightnessVal};
		GVariant *paramsTuple = g_variant_new_tuple(params,1);
		error = NULL;
		GVariant *retValue = g_dbus_connection_call_sync(connection,
                                           "org.gnome.SettingsDaemon",
                                           "/org/gnome/SettingsDaemon/Power",
                                           "org.gnome.SettingsDaemon.Power.Screen",
                                           "SetPercentage",
                                           paramsTuple,
                                           G_VARIANT_TYPE("(u)"),
                                           G_DBUS_CALL_FLAGS_NONE,
                                           -1,
                                           NULL,
                                           &error);
		if (retValue == NULL)
        {
        	if(error)
				g_printerr("Error invoking SetPercentage(): %s\n", error->message);
			else
				g_printerr("Error invoking SetPercentage()");
        }
        else
        {
        	// returned GVariant is a tuple, first index is the set value in uint32
        	if( g_variant_n_children(retValue) >= 1 )
        	{
        		if(brightnessUint32 != g_variant_get_uint32(g_variant_get_child_value(retValue,0)) )
        			{ g_printerr("Setting brightness value=%d failed.",brightnessUint32); }
        		else
        			{ g_message("Brightness value=%d set.",brightnessUint32); }
        	}
        }

        g_variant_unref(retValue);
        g_object_unref(connection);
        if(error)
			{ g_error_free(error); }
	}
}

int main (int argc, char **argv)
{
	// Query current brightness
	int32_t currBrightness = getCurrentBrightness();
	gchar *defBrightnessState = {"0.35"};
	if( currBrightness >= 0 )
	{
		double brightnessNormalized = (double)currBrightness/100;
		GVariant *brightnessGVar = g_variant_new_double(brightnessNormalized);
		defBrightnessState = g_variant_print(brightnessGVar,TRUE);
	}

	// === Build Actions =================================
	GActionEntry actionEntries[] = {
		{ "_header", NULL, NULL, "@a{sv} {}", NULL },
		{ "brightness", NULL, NULL, defBrightnessState, brightnessStateChange },
		{ "rotLeft", rotateActivated, NULL, "'right'", NULL },	// xrandr rotation is more logical to me the other way
		{ "rotRight", rotateActivated, NULL, "'left'", NULL },	// xrandr rotation is more logical to me the other way
		{ "rotInv", rotateActivated, NULL, "'inverted'", NULL },
		{ "rotNorm", rotateActivated, NULL, "'normal'", NULL },
		{ "shown", NULL, NULL, "@b false", indicatorShownStateChange }
	};

	// Build root menu item action state as a Vardict type GVariant
	GVariantBuilder *rootMenuItemActionVarBuilder = g_variant_builder_new (G_VARIANT_TYPE ("a{sv}"));
	g_variant_builder_add (rootMenuItemActionVarBuilder, "{sv}", "title", g_variant_new_string ("Screen"));
	g_variant_builder_add (rootMenuItemActionVarBuilder, "{sv}", "accessible-desc", g_variant_new_string ("Change screen brightness and orientation."));
	g_variant_builder_add (rootMenuItemActionVarBuilder, "{sv}", "icon", g_icon_serialize(g_themed_icon_new("gsd-xrandr")));
	g_variant_builder_add (rootMenuItemActionVarBuilder, "{sv}", "visible", g_variant_new_boolean (TRUE));
	GVariant *rootMenuItemActionVar = g_variant_builder_end (rootMenuItemActionVarBuilder);
	actionEntries[0].state = g_variant_print(rootMenuItemActionVar,TRUE);

	// build GSimpleActionGroup from actionEntries
	indicator.actions = g_simple_action_group_new();
	g_action_map_add_action_entries(G_ACTION_MAP(indicator.actions), actionEntries, G_N_ELEMENTS(actionEntries), NULL);

	// === Build Menu =================================

	GMenu *submenu = g_menu_new();

	// create root item (indicator icon)
	GMenuItem *menuRootItem = g_menu_item_new(NULL, "indicator._header");
	g_menu_item_set_attribute(menuRootItem, "x-canonical-type", "s", "com.canonical.indicator.root");
	g_menu_item_set_attribute(menuRootItem, "submenu-action", "s", "indicator.shown");
	g_menu_item_set_submenu (menuRootItem, G_MENU_MODEL(submenu));
	indicator.menu = g_menu_new();
	g_menu_append_item (indicator.menu, menuRootItem);

	// brightness slider
	GMenuItem *brightnessItem = g_menu_item_new(NULL, "indicator.brightness");
	g_menu_item_set_attribute(brightnessItem, "x-canonical-type", "s", "com.canonical.unity.slider");
	g_menu_item_set_attribute(brightnessItem, "min-value", "d", 0.0);
	g_menu_item_set_attribute(brightnessItem, "max-value", "d", 1.0);
	g_menu_item_set_attribute(brightnessItem, "step", "d", 0.02);
	//g_menu_item_set_attribute(brightnessItem, "x-canonical-sync-action", "s", "indicator.brightness_sync");
	g_menu_append_item(submenu, brightnessItem);

	// rotate left item
	GMenuItem *rotLeftItem = g_menu_item_new("rotate Left", "indicator.rotLeft");
	g_menu_item_set_attribute(rotLeftItem, "icon", "s", "stock_left");
	g_menu_append_item (submenu, rotLeftItem);

	// rotate right item
	GMenuItem *rotRightItem = g_menu_item_new("rotate Right", "indicator.rotRight");
	g_menu_item_set_attribute(rotRightItem, "icon", "s", "stock_right");
	g_menu_append_item (submenu, rotRightItem);

	// rotate inverted item
	GMenuItem *rotInvItem = g_menu_item_new("rotate 180°", "indicator.rotInv");
	g_menu_item_set_attribute(rotInvItem, "icon", "s", "stock_up");
	g_menu_append_item (submenu, rotInvItem);

	// rotate normal item
	GMenuItem *rotNormItem = g_menu_item_new("rotate to Normal", "indicator.rotNorm");
	g_menu_item_set_attribute(rotNormItem, "icon", "s", "undo");
	g_menu_append_item (submenu, rotNormItem);

	// ==================================================================
	
	// register on DBus
	g_bus_own_name (G_BUS_TYPE_SESSION,
				  DBUS_NAME,
				  G_BUS_NAME_OWNER_FLAGS_NONE,
				  bus_acquired,
				  NULL,
				  name_lost,
				  &indicator,
				  NULL);

	// Start GMainLoop
	GMainLoop *loop = g_main_loop_new (NULL, FALSE);
	g_main_loop_run (loop);

	// clean...
	g_variant_unref(rootMenuItemActionVar);
	g_object_unref (submenu);
	g_object_unref (menuRootItem);
	g_object_unref (indicator.actions);
	g_object_unref (indicator.menu);
	g_object_unref (loop);

	return 0;
}
