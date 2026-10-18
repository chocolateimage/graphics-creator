#include "ae_data.hpp"
#include "suite_loader.hpp"
#include "utility_callbacks.hpp"

InData::InData() {
    interactionCallbacks = InteractionCallbacks();
    utilityCallbacks = new UtilityCallbacks();
    suiteLoader = new SuiteLoader();
}

InData::~InData() {
    delete utilityCallbacks;
    delete suiteLoader;
}
