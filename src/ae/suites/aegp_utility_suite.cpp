#include "aegp_utility_suite.hpp"

int32_t reportInfo(int32_t pluginId, char *string) {
    qInfo() << "INFO FROM PLUGIN" << string;
    return 0;
}

int32_t reportInfoUnicode(int32_t pluginId, uint16_t *string) {
    qInfo() << "reportInfoUnicode";
    return 0;
}

int32_t getPluginInitFuncVersion(int16_t *, int16_t *) {
    qInfo() << "getPluginInitFuncVersion";
    return 0;
}

int32_t getImplementationVersion(int16_t *, int16_t *) {
    qInfo() << "getImplementationVersion";
    return 0;
}

int32_t startQuietErrors(void *) {
    qInfo() << "startQuietErrors";
    return 0;
}

int32_t endQuietErrors(bool, void *) {
    qInfo() << "endQuietErrors";
    return 0;
}

int32_t getLastErrorMessage(int32_t, char *str, int32_t *err) {
    qInfo() << "getLastErrorMessage";
    return 0;
}

int32_t startUndoGroup(char *) {
    qInfo() << "startUndoGroup";
    return 0;
}

int32_t endUndoGroup() {
    qInfo() << "endUndoGroup";
    return 0;
}

int32_t registerWithAegp(void *, char *name, int32_t *id) {
    qInfo() << "registerWithAegp";
    qInfo() << "name" << name;
    qInfo() << "id" << *id;
    return 0;
}

AEGPUtilitySuite3::AEGPUtilitySuite3() {
    reportInfo = (void *)::reportInfo;
    getPluginInitFuncVersion = (void *)::getPluginInitFuncVersion;
    getImplementationVersion = (void *)::getImplementationVersion;
    startQuietErrors = (void *)::startQuietErrors;
    endQuietErrors = (void *)::endQuietErrors;
    startUndoGroup = (void *)::startUndoGroup;
    endUndoGroup = (void *)::endUndoGroup;
    registerWithAegp = (void *)::registerWithAegp;
}

AEGPUtilitySuite6::AEGPUtilitySuite6() {
    reportInfo = (void *)::reportInfo;
    reportInfoUnicode = (void *)::reportInfoUnicode;
    getPluginInitFuncVersion = (void *)::getPluginInitFuncVersion;
    getImplementationVersion = (void *)::getImplementationVersion;
    startQuietErrors = (void *)::startQuietErrors;
    endQuietErrors = (void *)::endQuietErrors;
    getLastErrorMessage = (void *)::getLastErrorMessage;
    startUndoGroup = (void *)::startUndoGroup;
    endUndoGroup = (void *)::endUndoGroup;
    registerWithAegp = (void *)::registerWithAegp;
}
