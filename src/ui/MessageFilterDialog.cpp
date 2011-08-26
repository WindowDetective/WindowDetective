/////////////////////////////////////////////////////////////////////
// File: MessageFilterDialog.cpp                                   //
// Date: 23/6/11                                                   //
// Desc: Dialog window used to manage message filters for a        //
//   messages window. The filters can be inclusions/exclusions or  //
//   highlighting of particular messages.                          //
/////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010-2011 XTAL256

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#include "MessageFilterDialog.h"
using namespace inspector;

MessageFilterDialog::MessageFilterDialog(QWidget* parent) :
    QDialog(parent),
    allMessagesModel() {
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setupUi(this);
    highlightsTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    highlightsTable->horizontalHeader()->setResizeMode(1, QHeaderView::Fixed);
    highlightsTable->horizontalHeader()->setResizeMode(2, QHeaderView::Fixed);
    highlightsTable->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);

    connect(excludeButton, SIGNAL(clicked()), this, SLOT(excludeButtonClicked()));
    connect(includeButton, SIGNAL(clicked()), this, SLOT(includeButtonClicked()));
    connect(addHighlightButton, SIGNAL(clicked()), this, SLOT(addHighlightButtonClicked()));
    connect(removeHighlightButton, SIGNAL(clicked()), this, SLOT(removeHighlightButtonClicked()));
    connect(highlightsTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(highlightCellDoubleClicked(int,int)));
    connect(highlightsTable, SIGNAL(itemSelectionChanged()), this, SLOT(highlightItemSelectionChanged()));

    removeHighlightButton->setEnabled(false);   // Nothing selected initially
    tabWidget->setCurrentIndex(0);
}

/*------------------------------------------------------------------+
| Sets the list of all applicable messages, as well as the          |
| included and excluded message lists.                              |
+------------------------------------------------------------------*/
void MessageFilterDialog::setMessages(QStringList all, QStringList excluded) {
    allMessagesModel.setStringList(all);

    QStringList included;
    QStringList::const_iterator i;
    for (i = all.constBegin(); i != all.constEnd(); i++) {
        if (!excluded.contains(*i)) {
            included.append(*i);
        }
    }

    QAbstractItemModel* oldIncludeModel = includeListView->model();
    if (oldIncludeModel) delete oldIncludeModel;
    QAbstractItemModel* oldExcludeModel = excludeListView->model();
    if (oldExcludeModel) delete oldExcludeModel;

    QStringListModel* includeModel = new QStringListModel(included);
    includeListView->setModel(includeModel);
    QStringListModel* excludeModel = new QStringListModel(excluded);
    excludeListView->setModel(excludeModel);
    includeModel->sort(0);
    excludeModel->sort(0);
}

/*------------------------------------------------------------------+
| Sets the list of highlighted messages, and builds the UI.         |
+------------------------------------------------------------------*/
void MessageFilterDialog::setHighlightedMessages(QList<MessageHighlight> highlights) {
    QList<MessageHighlight>::const_iterator i;
    for (i = highlights.constBegin(); i != highlights.constEnd(); i++) {
        addHighlight(i->name, i->foregroundColour, i->backgroundColour);
    }
}

QStringListModel* MessageFilterDialog::getIncludedMessagesModel() {
    QStringListModel* model = dynamic_cast<QStringListModel*>(includeListView->model());
    if (!model) {
        Logger::debug("MessageFilterDialog::getIncludedMessagesModel - Unable to cast model to a QStringListModel");
        return NULL;
    }
    return model;
}

QStringListModel* MessageFilterDialog::getExcludedMessagesModel() {
    QStringListModel* model = dynamic_cast<QStringListModel*>(excludeListView->model());
    if (!model) {
        Logger::debug("MessageFilterDialog::getExcludedMessagesModel - Unable to cast model to a QStringListModel");
        return NULL;
    }
    return model;
}

/*------------------------------------------------------------------+
| Returns the string list of included messages.                     |
+------------------------------------------------------------------*/
QStringList MessageFilterDialog::getIncludedMessages() {
    QStringListModel* model = getIncludedMessagesModel();
    return model ? model->stringList() : QStringList();
}

/*------------------------------------------------------------------+
| Returns the string list of excluded messages.                     |
+------------------------------------------------------------------*/
QStringList MessageFilterDialog::getExcludedMessages() {
    QStringListModel* model = getExcludedMessagesModel();
    return model ? model->stringList() : QStringList();
}

/*------------------------------------------------------------------+
| Gets the highlighted messages from the UI table and returns them  |
| as a list of MessageHighlight objects.                            |
+------------------------------------------------------------------*/
QList<MessageHighlight> MessageFilterDialog::getHighlightedMessages() {
    QList<MessageHighlight> list;

    for (int i = 0; i < highlightsTable->rowCount(); i++) {
        MessageHighlight m;
        QComboBox* comboBox = dynamic_cast<QComboBox*>(highlightsTable->cellWidget(i, 0));
        if (!comboBox) {
            Logger::debug("MessageFilterDialog::getHighlightedMessages - Unable to get combo box from first column");
            return QList<MessageHighlight>();
        }
        m.name = comboBox->currentText();
        m.foregroundColour = highlightsTable->item(i, 1)->background().color();
        m.backgroundColour = highlightsTable->item(i, 2)->background().color();
        list.append(m);
    }

    return list;
}

/*------------------------------------------------------------------+
| Moves the selected items from the first view to the second.       |
+------------------------------------------------------------------*/
void MessageFilterDialog::moveSelectedItems(QListView* fromView, QListView* toView) {
    fromView->setUpdatesEnabled(false);
    toView->setUpdatesEnabled(false);

    QAbstractItemModel* fromModel = fromView->model();
    QAbstractItemModel* toModel = toView->model();

    QModelIndexList selectedIndexes = fromView->selectionModel()->selectedRows();
    qSort(selectedIndexes.begin(), selectedIndexes.end());

    // Add the selected items to the second list
    int rowCount;
    foreach (QModelIndex index, selectedIndexes) {
        rowCount = toModel->rowCount();
        toModel->insertRow(rowCount);
        toModel->setData(toModel->index(rowCount, 0), index.data());
    }

    // Then remove them from the first list
    for (int i = selectedIndexes.size(); i > 0; --i) {
        fromModel->removeRow(selectedIndexes.at(i - 1).row());
    }

    fromModel->sort(0);
    toModel->sort(0);

    fromView->setUpdatesEnabled(true);
    toView->setUpdatesEnabled(true);
}

/*------------------------------------------------------------------+
| Adds a new row in the highlights table.                           |
+------------------------------------------------------------------*/
void MessageFilterDialog::addHighlight(String msgName, QColor foreColour, QColor backColour) {
    const int lastRow = highlightsTable->rowCount();
    highlightsTable->setRowCount(lastRow+1);

    QComboBox* comboBox = new QComboBox();
    comboBox->view()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    comboBox->setModel(&allMessagesModel);
    int nameIndex = comboBox->findData(msgName, Qt::DisplayRole);
    if (nameIndex != -1) {
        comboBox->setCurrentIndex(nameIndex);
    }
    highlightsTable->setCellWidget(lastRow, 0, comboBox);

    QTableWidgetItem* foregroundColourItem = new QTableWidgetItem();
    foregroundColourItem->setBackgroundColor(foreColour);
    highlightsTable->setItem(lastRow, 1, foregroundColourItem);

    QTableWidgetItem* backgroundColourItem = new QTableWidgetItem();
    backgroundColourItem->setBackgroundColor(backColour);
    highlightsTable->setItem(lastRow, 2, backgroundColourItem);
}

/*------------------------------------------------------------------+
| The "move right" button was pressed. Move the selected items in   |
| the include list to the exclude list.                             |
+------------------------------------------------------------------*/
void MessageFilterDialog::excludeButtonClicked() {
    moveSelectedItems(includeListView, excludeListView);
}

/*------------------------------------------------------------------+
| The "move left" button was pressed. Move the selected items in    |
| the exclude list to the include list.                             |
+------------------------------------------------------------------*/
void MessageFilterDialog::includeButtonClicked() {
    moveSelectedItems(excludeListView, includeListView);
}

/*------------------------------------------------------------------+
| Add a new row in the highlights table.                            |
+------------------------------------------------------------------*/
void MessageFilterDialog::addHighlightButtonClicked() {
    QPalette palette = QApplication::palette();
    addHighlight("WM_NULL", palette.text().color(), palette.base().color());
}

/*------------------------------------------------------------------+
| Remove the selected rows (if any) in the highlights table.        |
+------------------------------------------------------------------*/
void MessageFilterDialog::removeHighlightButtonClicked() {
    QModelIndexList selectedIndexes = highlightsTable->selectionModel()->selectedRows();
    qSort(selectedIndexes.begin(), selectedIndexes.end());

    for (int i = selectedIndexes.size(); i > 0; --i) {
        highlightsTable->removeRow(selectedIndexes.at(i - 1).row());
    }
}

/*------------------------------------------------------------------+
| Open the colour chooser to change the value of the cell that was  |
| double-clicked. This cell must be in the foreground or background |
| colour columns.                                                   |
+------------------------------------------------------------------*/
void MessageFilterDialog::highlightCellDoubleClicked(int row, int column) {
    if (column == 0) return;     // Colour cells are in the 2nd and 3nd column

    QTableWidgetItem* colourItem = highlightsTable->item(row, column);
    QColor chosen = QColorDialog::getColor(colourItem->backgroundColor(), this, tr("Select colour"));
    if (chosen.isValid()) {
        colourItem->setBackgroundColor(chosen);
    }
}

/*------------------------------------------------------------------+
| Disables the "Remove" button if no items are selected.            |
+------------------------------------------------------------------*/
void MessageFilterDialog::highlightItemSelectionChanged() {
    removeHighlightButton->setEnabled(!highlightsTable->selectedItems().isEmpty());
}
