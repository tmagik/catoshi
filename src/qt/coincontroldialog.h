// Copyright (c) 2009-2012 *coin developers
// where * = (Bit, Lite, PP, Peerunmity, Blu, Cat, Solar, URO, ...)
// Previously distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef COINCONTROLDIALOG_H
#define COINCONTROLDIALOG_H

#include "amount.h"

#include <QAbstractButton>
#include <QAction>
#include <QDialog>
#include <QList>
#include <QMenu>
#include <QPoint>
#include <QString>
#include <QTreeWidgetItem>

class WalletModel;

class CCoinControl;
class CTxMemPool;

namespace Ui {
    class CoinControlDialog;
}

class CoinControlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CoinControlDialog(QWidget *parent = 0);
    ~CoinControlDialog();

    void setModel(WalletModel *model);
    
    // static because also called from sendcoinsdialog
    static void updateLabels(WalletModel*, QDialog*);
    static QString getPriorityLabel(double dPriority, double mempoolEstimatePriority);
    
    static QList<CAmount> payAmounts;
    static CCoinControl *coinControl;

private:
    Ui::CoinControlDialog *ui;
    WalletModel *model;
    int sortColumn;
    Qt::SortOrder sortOrder;
    
    QMenu *contextMenu;
    QTreeWidgetItem *contextMenuItem;
    QAction *copyTransactionHashAction;
    QAction *lockAction;
    QAction *unlockAction;
    
    QString strPad(QString, int, QString);
    void sortView(int, Qt::SortOrder);
    void updateView();
    
    enum
    {
        COLUMN_CHECKBOX,
        COLUMN_AMOUNT,
        COLUMN_LABEL,
        COLUMN_ADDRESS,
        COLUMN_DATE,
        COLUMN_CONFIRMATIONS,
        COLUMN_PRIORITY,
        COLUMN_TXHASH,
        COLUMN_VOUT_INDEX,
        COLUMN_AMOUNT_INT64,
        COLUMN_PRIORITY_INT64,
        COLUMN_DATE_INT64
    };
    
    // some columns have a hidden column containing the value used for sorting
    int getMappedColumn(int column, bool fVisibleColumn = true)
    {
        if (fVisibleColumn)
        {
            if (column == COLUMN_AMOUNT_INT64)
                return COLUMN_AMOUNT;
            else if (column == COLUMN_PRIORITY_INT64)
                return COLUMN_PRIORITY;
            else if (column == COLUMN_DATE_INT64)
                return COLUMN_DATE;
        }
        else
        {
            if (column == COLUMN_AMOUNT)
                return COLUMN_AMOUNT_INT64;
            else if (column == COLUMN_PRIORITY)
                return COLUMN_PRIORITY_INT64;
            else if (column == COLUMN_DATE)
                return COLUMN_DATE_INT64;
        }

        return column;
    }

private slots:
    void showMenu(const QPoint &);
    void copyAmount();
    void copyLabel();
    void copyAddress();
    void copyTransactionHash();
    void lockCoin();
    void unlockCoin();
    void clipboardQuantity();
    void clipboardAmount();
    void clipboardFee();
    void clipboardAfterFee();
    void clipboardBytes();
    void clipboardPriority();
    void clipboardLowOutput();
    void clipboardChange();
    void radioTreeMode(bool);
    void radioListMode(bool);
    void viewItemChanged(QTreeWidgetItem*, int);
    void headerSectionClicked(int);
    void buttonBoxClicked(QAbstractButton*);
    void buttonSelectAllClicked();
    void updateLabelLocked();
};

#endif // _COINCONTROLDIALOG_H
