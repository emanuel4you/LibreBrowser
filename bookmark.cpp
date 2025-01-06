#include "bookmark.h"
#include "ui_bookmark.h"
#include <QSettings>

Bookmark::Bookmark(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Bookmark)
{
    ui->setupUi(this);
    QSettings settings("LibreBrowser", "LibreBrowser");
    settings.beginGroup("Bookmark");
    QStringList recentBookmark = settings.value("Bookmarks").toStringList();
    ui->listWidget->addItems(recentBookmark);
    settings.endGroup();
}

Bookmark::~Bookmark()
{
    delete ui;
}

void Bookmark::on_deleteButton_2_clicked()
{
    QStringList list;
    //ui->listWidget->removeItemWidget(ui->listWidget->currentItem());

    qDeleteAll(ui->listWidget->selectedItems());

    for(int row = 0; row < ui->listWidget->count(); row++)
    {
        QListWidgetItem *item = ui->listWidget->item(row);
        list.push_back(item->text());
    }
    QSettings settings("LibreBrowser", "LibreBrowser");
    settings.beginGroup("Bookmark");
    settings.setValue("Bookmarks", list);
    settings.endGroup();
}


void Bookmark::on_acceptButton_clicked()
{
    accept();
}

