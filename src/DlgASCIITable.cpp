#include "DlgASCIITable.h"
#include "ui_ASCII.h"

#include <QStandardItemModel>
#include <QStringList>

struct ASCIIData{
    QString Abbreviation;
    QString Describe;
};
DlgASCIITable::DlgASCIITable( QWidget* parent ) : QDialog( parent )
{
    m_ui = new Ui_DialogASCIITable;
    m_ui->setupUi( this );

    m_asciiModel = new QStandardItemModel();
    m_ui->tableView->setModel( m_asciiModel );
    resetUI();    
    m_ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);      
    m_ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ui->tableView->setColumnWidth(0,80); 
    m_ui->tableView->setColumnWidth(1,50); 
    m_ui->tableView->setColumnWidth(2,50); 
    m_ui->tableView->setColumnWidth(3,200); 
}

DlgASCIITable::~DlgASCIITable()
{
    delete m_ui;
}

void DlgASCIITable::resetUI()
{
    QStringList asciiTable;
    asciiTable 
        << tr( "NUL (null)" )
        << tr( "SOH (start of headline)" )
        << tr( "STX (start of text)" )
        << tr( "ETX (end of text)" )
        << tr( "EOT (end of transmission)" )
        << tr( "ENQ (enquiry)" )
        << tr( "ACK (acknowledge)" )
        << tr( "BEL (bell)" )
        << tr( "BS (backspace)" )
        << tr( "HT (horizontal tab)" )
        << tr( "LF (NL line feed, new line)" )
        << tr( "VT (vertical tab)" )
        << tr( "FF (NP form feed, new page)" )
        << tr( "CR (carriage return)" )
        << tr( "SO (shift out)" )
        << tr( "SI (shift in)" )
        << tr( "DLE (data link escape)" )
        << tr( "DC1 (device control 1)" )
        << tr( "DC2 (device control 2)" )
        << tr( "DC3 (device control 3)" )
        << tr( "DC4 (device control 4)" )
        << tr( "NAK (negative acknowledge)" )
        << tr( "SYN (synchronous idle)" )
        << tr( "ETB (end of trans. block)" )
        << tr( "CAN (cancel)" )
        << tr( "EM (end of medium)" )
        << tr( "SUB (substitute))" )
        << tr( "ESC (escape)" )
        << tr( "FS (file separator)" )
        << tr( "GS (group separator)" )
        << tr( "RS (record separator)" )
        << tr( "US (unit separator)" )
        << tr( "(space)" )
        << "!"
        << "\""
        << "#"
        << "$"
        << "%"
        << "&"
        << "'"
        << "("
        << ")"
        << "*"
        << "+"
        << ","
        << "-"
        << "."
        << "/"
        << "0"
        << "1"
        << "2"
        << "3"
        << "4"
        << "5"
        << "6"
        << "7"
        << "8"
        << "9"
        << ":"
        << ";"
        << "<"
        << "="
        << ">"
        << "?"
        << "@"
        << "A"
        << "B"
        << "C"
        << "D"
        << "E"
        << "F"
        << "G"
        << "H"
        << "I"
        << "J"
        << "K"
        << "L"
        << "M"
        << "N"
        << "O"
        << "P"
        << "Q"
        << "R"
        << "S"
        << "T"
        << "U"
        << "V"
        << "W"
        << "X"
        << "Y"
        << "Z"
        << "["
        << "\\"
        << "]"
        << "^"
        << "_"
        << "`"
        << "a"
        << "b"
        << "c"
        << "d"
        << "e"
        << "f"
        << "g"
        << "h"
        << "i"
        << "j"
        << "k"
        << "l"
        << "m"
        << "n"
        << "o"
        << "p"
        << "q"
        << "r"
        << "s"
        << "t"
        << "u"
        << "v"
        << "w"
        << "x"
        << "y"
        << "z"
        << "{"
        << "|"
        << "}"
        << "~"
        << tr( "DEL (delete)" );

    if( m_asciiModel->columnCount() <= 0 )
    {
        m_asciiModel->setHorizontalHeaderItem(0, new QStandardItem());
        m_asciiModel->setHorizontalHeaderItem(1, new QStandardItem());
        m_asciiModel->setHorizontalHeaderItem(2, new QStandardItem());
        m_asciiModel->setHorizontalHeaderItem(3, new QStandardItem());
    }
    m_asciiModel->setHeaderData( 0, Qt::Horizontal, tr( "Bin" ) );
    m_asciiModel->setHeaderData( 1, Qt::Horizontal, tr( "Oct" ) );
    m_asciiModel->setHeaderData( 2, Qt::Horizontal, tr( "Hex" ) );
    m_asciiModel->setHeaderData( 3, Qt::Horizontal, tr( "Describe" ) );

    if( m_asciiModel->rowCount() <= 0 )
    {
        for ( int i = 0; i < asciiTable.size(); ++i )
        {
            m_asciiModel->setItem( i, 0, new QStandardItem( QString("%1").arg( i, 8, 2, QChar('0') ) ) );
            m_asciiModel->setItem( i, 1, new QStandardItem( QString("%1").arg( i, 3, 8, QChar('0') ) ) );
            m_asciiModel->setItem( i, 2, new QStandardItem( QString("%1").arg( i, 2, 16, QChar('0') ) ) );
            m_asciiModel->setItem( i, 3, new QStandardItem( asciiTable.at( i ) ) );
            m_asciiModel->setHeaderData( i, Qt::Vertical, QString::number( i ) );
        }
    }else
    {
        for ( int i = 0; i < asciiTable.size(); ++i )
            m_asciiModel->setData( m_asciiModel->index( i, 3 ), asciiTable.at( i ) );
    }
}

