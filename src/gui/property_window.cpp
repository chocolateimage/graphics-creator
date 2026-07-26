#include "property_window.hpp"
#include "property_edit.hpp"
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>

PropertyWindow::PropertyWindow(Scene *scene) : scene(scene) {
    formLayout = new QFormLayout(this);
    formLayout->setVerticalSpacing(2);
    connect(scene, &Scene::elementSelectionChanged, this,
            &PropertyWindow::selectedElementsUpdated);
    connect(scene, &Scene::framesChanging, this,
            &PropertyWindow::framesChanging);
    selectedElementsUpdated({});
}

void PropertyWindow::framesChanging(bool changing) {
    if (changing) {
        setDisabled(true);
        return;
    }

    setDisabled(false);
    selectedElementsUpdated(scene->selectedElements);
}

void PropertyWindow::selectedElementsUpdated(
    QList<Element *> selectedElements) {
    while (formLayout->rowCount() > 0) {
        formLayout->removeRow(0);
    }

    if (selectedElements.size() == 0) {
        QLabel *lbl = new QLabel("No elements selected", this);
        lbl->setDisabled(true);
        lbl->setWordWrap(true);
        formLayout->addWidget(lbl);
        return;
    }

    if (selectedElements.size() > 1) {
        QLabel *lbl = new QLabel(
            "Multiple elements cannot be edited at the same time", this);
        lbl->setDisabled(true);
        lbl->setWordWrap(true);
        formLayout->addWidget(lbl);
        return;
    }

    Element *element = selectedElements.first();

    QLabel *idLabel = new QLabel(element->id.first(8), this);
    idLabel->setDisabled(true);
    idLabel->setToolTip(element->id);
    formLayout->addRow("ID", idLabel);

    QString name = element->objectName();
    QLineEdit *nameEdit = new QLineEdit(name, this);
    connect(
        nameEdit, &QLineEdit::textEdited, this,
        [element](const QString &newText) { element->setObjectName(newText); });
    formLayout->addRow("Name", nameEdit);

    for (auto property : element->properties) {
        PropertyEdit *propertyEdit = new PropertyEdit(property, scene, this);
        formLayout->addRow(property->getDisplayName(), propertyEdit);
    }
}
