// Copyright (c) 2009-2012 *coin developers
// where * = (Bit, Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
// Previously distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QStackedWidget;
class QListWidget;
class QListWidgetItem;
class QPushButton;
QT_END_NAMESPACE
class OptionsModel;
class MainOptionsPage;
class DisplayOptionsPage;
class MonitoredDataMapper;

/** Preferences dialog. */
class OptionsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit OptionsDialog(QWidget *parent=0);

    void setModel(OptionsModel *model);

signals:

public slots:
    /** Change the current page to \a index. */
    void changePage(int index);

private slots:
    void okClicked();
    void cancelClicked();
    void applyClicked();
    void enableApply();
    void disableApply();

private:
    QListWidget *contents_widget;
    QStackedWidget *pages_widget;
    OptionsModel *model;
    MonitoredDataMapper *mapper;
    QPushButton *apply_button;

    // Pages
    MainOptionsPage *main_page;
    DisplayOptionsPage *display_page;

    void setupMainPage();
};

#endif // OPTIONSDIALOG_H
