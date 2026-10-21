#include "ae_panel.hpp"
#ifdef Q_OS_WIN
#include "ae/aegp.hpp"
#include "ae/suites/aegp_workspace_panel_suite.hpp"

AEPanelWindow::AEPanelWindow(AEGPPanelRegistration *registration)
    : QWidget(), registration(registration) {
    setWindowTitle(registration->matchName);
}

bool AEPanelWindow::nativeEvent(const QByteArray &eventType, void *message,
                                qintptr *result) {
    MSG *msg = (MSG *)message;
    if (msg->message == WM_SYSCOMMAND) {
        if (menuItemCommandMapping.contains(msg->wParam)) {
            functions.doPopupCommand(instanceRefcon,
                                     menuItemCommandMapping[msg->wParam]);
        }
    }
    return false;
}

void AEPanelWindow::showEvent(QShowEvent *event) {
    if (created) {
        return;
    }
    created = true;

    HWND hwnd = (HWND)window()->winId();

    registration->createPanelFunc(registration->aegp->globalRefcon,
                                  registration->funcData, hwnd, this,
                                  &functions, &instanceRefcon);

    createPopup();
}

void AEPanelWindow::createPopup() {
    if (functions.populatePopup == nullptr)
        return;

    HWND hwnd = (HWND)window()->winId();
    int32_t count = 512;
    PopupMenuItem *items = new PopupMenuItem[count]();
    functions.populatePopup(instanceRefcon, items, &count);
    if (count == 0) {
        delete[] items;
        return;
    }

    PopupMenuItem2 *rootItem = new PopupMenuItem2();
    HMENU menu = GetSystemMenu(hwnd, false);
    rootItem->menu = menu;
    rootItem->indent = 0;

    PopupMenuItem2 *lastItem = rootItem;

    for (int i = 0; i < count; i++) {
        PopupMenuItem *itemInfo = items + i;
        PopupMenuItem2 *item = new PopupMenuItem2();
        item->info = itemInfo;
        item->indent = itemInfo->indent;
        if (item->indent > lastItem->indent) {
            item->parent = lastItem;
        } else if (item->indent == lastItem->indent) {
            item->parent = lastItem->parent;
        } else {
            item->parent = lastItem->parent->parent;
        }
        item->parent->children.append(item);
        lastItem = item;
    }

    lastItemId = 300;
    createPopupItems(rootItem);

    delete[] items;
}

void AEPanelWindow::createPopupItems(PopupMenuItem2 *item) {
    for (auto child : item->children) {
        if (!child->children.isEmpty()) {
            child->menu = CreatePopupMenu();
            InsertMenuA(item->menu, 0, MF_POPUP, (UINT_PTR)child->menu,
                        child->info->name);
        } else {
            uint32_t flags = MF_STRING;
            switch (child->info->type) {
            case PopupMenuItem::ItemType::Normal:
                break;
            case PopupMenuItem::ItemType::Checked:
                flags |= MF_CHECKED;
                break;
            case PopupMenuItem::ItemType::Radio:
                // TODO: implement
                break;
            case PopupMenuItem::ItemType::Separator:
                flags = MF_SEPARATOR;
                break;
            }
            InsertMenuA(item->menu, 0, flags, lastItemId, child->info->name);
        }
        menuItemCommandMapping[lastItemId] = child->info->commandId;
        lastItemId++;
        createPopupItems(child);
    }
    delete item;
}

#endif
