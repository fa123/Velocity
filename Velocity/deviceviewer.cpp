#include "deviceviewer.h"
#include "ui_deviceviewer.h"

DeviceViewer::DeviceViewer(QWidget *parent) : QDialog(parent), ui(new Ui::DeviceViewer)
{
    ui->setupUi(this);
    currentDrive = NULL;

    // setup the context menus
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showRemoveContextMenu(QPoint)));
}

DeviceViewer::~DeviceViewer()
{
    if (currentDrive)
        delete currentDrive;

    delete ui;
}

void DeviceViewer::on_pushButton_clicked()
{
    // clear all the items
    ui->treeWidget->clear();

    if (currentDrive)
        currentDrive->Close();

    // open the drive
    currentDrive = new FatxDrive(ui->lineEdit->text().toStdWString());

    // load partitions
    std::vector<Partition*> parts = currentDrive->GetPartitions();
    for (int i = 0; i < parts.size(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

        item->setText(0, QString::fromStdString(parts.at(i)->name));
        item->setData(0, Qt::UserRole, QVariant::fromValue(parts.at(i)));
    }
}

void DeviceViewer::on_treeWidget_expanded(const QModelIndex &index)
{
    // get the item
    QTreeWidgetItem *item = (QTreeWidgetItem*)index.internalPointer();

    // remove all current child items
    qDeleteAll(item->takeChildren());

    // set the current parent
    FatxFileEntry *currentParent;
    if (!item->parent())
    {
        Partition *part = item->data(0, Qt::UserRole).value<Partition*>();
        currentParent = &part->root;
    }
    else
        currentParent = item->data(0, Qt::UserRole).value<FatxFileEntry*>();

    currentDrive->GetChildFileEntries(currentParent);

    for (int i = 0; i < currentParent->cachedFiles.size(); i++)
    {
        // get the entry
        FatxFileEntry *entry = &currentParent->cachedFiles.at(i);

        // don't show if it's deleted
        if (entry->nameLen == FATX_ENTRY_DELETED)
            continue;

        // setup the tree widget item
        QTreeWidgetItem *entryItem = new QTreeWidgetItem(item);
        entryItem->setData(0, Qt::UserRole, QVariant::fromValue(entry));

        // show the indicator if it's a directory
        if (entry->fileAttributes & FatxDirectory)
            entryItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

        // setup the text
        entryItem->setText(0, QString::fromStdString(entry->name));
        entryItem->setText(1, QString::fromStdString(ByteSizeToString(entry->fileSize)));
        entryItem->setText(2, "0x" + QString::number(entry->startingCluster, 16));

        // add it to the tree widget
        item->addChild(entryItem);
    }
}

void DeviceViewer::showRemoveContextMenu(QPoint point)
{
    QPoint globalPos = ui->treeWidget->mapToGlobal(point);
    QMenu contextMenu;

    contextMenu.addAction(QPixmap(":/Images/properties.png"), "Extract Selected");

    QAction *selectedItem = contextMenu.exec(globalPos);
    if(selectedItem == NULL)
        return;

    QList <QTreeWidgetItem*> items = ui->treeWidget->selectedItems();

    if (selectedItem->text() == "Extract Selected")
    {
        if (items.size() < 1)
            return;

        // get the save path
        QString path = QFileDialog::getSaveFileName(this, "Save Location", QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));

        if (path.isEmpty())
            return;

        // save the file
        FatxFileEntry *entry = items.at(0)->data(0, Qt::UserRole).value<FatxFileEntry*>();
        FatxIO io = currentDrive->GetFatxIO(entry);
        io.SaveFile(path.toStdString());
    }
}
