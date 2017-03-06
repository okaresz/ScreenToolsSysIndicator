ScreenToolsSysIndicator
======================================

A system indicator for Ubuntu Unity desktop to control screen brightness and orientation.

![screentools-indicator-screenshot](https://github.com/okaresz/ScreenToolsSysIndicator/blob/master/screenshot.jpg)

The code is based on  [sneetsher's "mysystemindicator"](https://github.com/sneetsher/mysystemindicator).
I use this indicator as a standalone application, so I don't need the unity dbus indicator service config file (indicatorDbusServiceDef), installation is commented out in CMakeLists.


Installation
---------------------------------

Dependencies: libglib2.0-dev

Steps to build & test

    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
    make
    sudo make install

Log out and back in or run `sudo service lightdm restart` (required for unity to parse indicator definition file and display indicator icon if it is registered on DBus).
Run by starting the installed executable.


Notes, TODO
---------------------------------

### GLib C

Why GLib C - the hardest way possible? Appindicators would be an easy solution: there is a [python API](http://python-gtk-3-tutorial.readthedocs.io/en/latest/), there are [tutorials](http://candidtim.github.io/appindicator/2014/09/13/ubuntu-appindicator-step-by-step.html), and [example indicators](https://code.launchpad.net/~caffeine-developers/caffeine/main). But appIndicators (with libappindicator) [can only use simple label menu items](http://askubuntu.com/a/16432), and I need a slider to control brightness.

So I stayed with C and bare GLib (no need for more, sneetsher only uses GTK+ API to display the about dialog - I don't need that...). One could try of course [gtkmm](https://developer.gnome.org/glibmm/stable/), the C++ wrapper lib for GLib.

### Brightness slider update

The brightness slider should be updated when brightness is changed with keys, or other input. A fair solution would be to check current brightness every time the indicator menu is opened and update the slider. BUT! If the correspondent GAction has a state-change handler set in actionEntries[]  (and it has, since I need it to get slider value changes), I cannot update the slider by explicitly setting the state with g_action_change_state(). But if state-change handler is NULL, g_action_change_state() works, the slider position can be updated, but now we cannot get state-change callbacks.
It seems the unity indicator framework decides "widget read/write direction" based on wheter there is an explicit state-change handler?....

I tried to understand the default sound indicator source, as it has this exaact same situation with the volume slider, but to no avail. Nor could I figure out what "x-canonical-sync-action" is for.

So the solution is to read the current brightness upon application start, set it as default state, so the slider is initialized to the correct position. As the indicator will be only started when the laptop is in tablet mode, there is no other way to set the brightness anyway; only with this indicator. Hah!


Resources, docs used
---------------------------------

- sneetsher's summary on askubuntu: http://askubuntu.com/a/752750
- https://unity.ubuntu.com/projects/system-indicators/
- https://wiki.ubuntu.com/SystemComponents
- the source of the default [ubuntu sound indicator](https://launchpad.net/ubuntu/+source/indicator-sound/12.10.2+16.04.20160502.1-0ubuntu1) 
- GLib C docs: https://developer.gnome.org/glib/stable/
- GVariant format string syntax: https://people.gnome.org/~ryanl/glib-docs/gvariant-format-strings.html
