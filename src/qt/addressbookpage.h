#ifndef ADDRESSBOOKPAGE_H
#define ADDRESSBOOKPAGE_H

#include <QDialog>

namespace Ui {
    class AddressBookPage;
}
class AddressTableModel;

QT_BEGIN_NAMESPACE
class QTableView;
class QItemSelection;
class QSortFilterProxyModel;
QT_END_NAMESPACE

/** Widget that shows a list of sending or receiving addresses.
  */
class AddressBookPage : public QDialog
{
    Q_OBJECT

public:
    enum Tabs {
        SendingTab = 0,
        ReceivingTab = 1
    };

    enum Mode {
        ForSending, /**< Open address book to pick address for sending */
        ForEditing  /**< Open address book for editing */
    };

    explicit AddressBookPage(Mode mode, Tabs tab, QWidget *parent = 0);
    ~AddressBookPage();

    void setModel(AddressTableModel *model);
    const QString &getReturnValue() const { return returnValue; }

public slots:
    void done(int retval);
    void exportClicked();

private:
    Ui::AddressBookPage *ui;
    AddressTableModel *model;
    Mode mode;
    Tabs tab;
    QString returnValue;
    QSortFilterProxyModel *proxyModel;

private slots:
    void on_deleteButton_clicked();
    void on_newAddressButton_clicked();
    void on_copyToClipboard_clicked();
    void selectionChanged();
    void on_showQRCode_clicked();
};

#endif // ADDRESSBOOKDIALOG_H
