#include "aegp_command_suite.hpp"
#include <QAction>

QList<QAction *> aegpGlobalCommands = {};

QAction *getQAction(int32_t command) { return aegpGlobalCommands[command]; }

int32_t getUniqueCommand(int32_t *command) {
    *command = aegpGlobalCommands.length();
    aegpGlobalCommands.append(new QAction());
    return 0;
}
int32_t insertMenuCommand(int32_t command, char *name,
                          AEGPCommandSuite1::Menu menu, int32_t afterItem) {
    getQAction(command)->setText(name);
    return 0;
}
int32_t removeMenuCommand(int32_t command) {
    qInfo() << "removeMenuCommand";
    return 0;
}
int32_t setMenuCommandName(int32_t command, char *name) {
    getQAction(command)->setText(name);
    return 0;
}
int32_t enableCommand(int32_t command) {
    getQAction(command)->setEnabled(true);
    return 0;
}
int32_t disableCommand(int32_t command) {
    getQAction(command)->setDisabled(true);
    return 0;
}
int32_t checkmarkMenuCommand(int32_t command, bool checkbox) {
    qInfo() << "checkmarkMenuCommand";
    getQAction(command)->setChecked(checkbox);
    return 0;
}
int32_t doCommand(int32_t command) {
    getQAction(command)->trigger();
    return 0;
}

AEGPCommandSuite1::AEGPCommandSuite1() {
    getUniqueCommand = (void *)::getUniqueCommand;
    insertMenuCommand = (void *)::insertMenuCommand;
    removeMenuCommand = (void *)::removeMenuCommand;
    setMenuCommandName = (void *)::setMenuCommandName;
    enableCommand = (void *)::enableCommand;
    disableCommand = (void *)::disableCommand;
    checkmarkMenuCommand = (void *)::checkmarkMenuCommand;
    doCommand = (void *)::doCommand;
}
