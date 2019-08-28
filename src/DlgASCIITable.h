#ifndef __DLG_ASCII_H__
#define __DLG_ASCII_H__

#include <QDialog>

class Ui_DialogASCIITable;
class QSettings;
class QShowEvent;
class QStandardItemModel;

class DlgASCIITable : public QDialog
{
    Q_OBJECT
public:
    DlgASCIITable( QWidget* parent = 0 );
    ~DlgASCIITable();

protected:
    void resetUI();

protected:
    Ui_DialogASCIITable*    m_ui;
    QStandardItemModel*     m_asciiModel;
};
#endif
