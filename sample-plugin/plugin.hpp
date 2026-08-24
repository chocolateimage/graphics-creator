#pragma once
#include "gc_plugin_api.hpp"

extern "C" {
PLUGIN_EXPORT extern int gcPluginInit(PluginInterface *intf,
                                      PluginInitData *data);
}
