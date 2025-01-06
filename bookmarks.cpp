#include "bookmarks.h"
#include "ui_bookmarks.h"

Bookmarks::Bookmarks(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Bookmarks)
{
    ui->setupUi(this);
}

Bookmarks::~Bookmarks()
{
    delete ui;
}

void Bookmarks::on_cancelButton_clicked()
{

}


void Bookmarks::on_deleteButton_clicked()
{

}

