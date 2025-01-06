#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QDialog>

namespace Ui {
class Bookmark;
}

class Bookmark : public QDialog
{
    Q_OBJECT

public:
    explicit Bookmark(QWidget *parent = nullptr);
    ~Bookmark();

private slots:
    void on_deleteButton_2_clicked();
    void on_acceptButton_clicked();

private:
    Ui::Bookmark *ui;
};

#endif // BOOKMARK_H
