#pragma once
#include <QMap>

class Plugin;
class PluginPipl;
struct SuiteLoader;

class AEGP {
  public:
    static QMap<int32_t, AEGP *> loadedAegps;
    static AEGP *getPlugin(int32_t pluginId);

    AEGP(Plugin *plugin, PluginPipl *pipl);

    void *globalRefcon{nullptr};

  private:
    int pluginId;
    Plugin *plugin;
    SuiteLoader *suiteLoader;
};
