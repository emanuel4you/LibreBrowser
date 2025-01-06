#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class Bookmarks;
}
QT_END_NAMESPACE

class Bookmarks : public QDialog
{
    Q_OBJECT

public:
    Bookmarks(QWidget *parent = nullptr);
    ~Bookmarks();

private slots:
    void on_cancelButton_clicked();

    void on_deleteButton_clicked();

private:
    Ui::Bookmarks *ui;
};
#endif // BOOKMARKS_H
