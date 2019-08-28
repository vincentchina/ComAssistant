#include "DlgOptions.h"
#include "ui_options.h"
#include "HexConvert.h"

#include <QSettings>
#include <QApplication>
#include <QFileDialog>
#include <QFontDialog>

DlgOptions::DlgOptions( QWidget* parent ) : QDialog( parent )
{
    m_ui = new Ui_DialogOptions;
    m_ui->setupUi( this );

    QString path = qApp->applicationDirPath() + "/optinos.ini";
    m_settings = new QSettings( path, QSettings::IniFormat );
    m_settings->setIniCodec( "UTF-8" );

    loadSettings();

    connect( this, SIGNAL(finished(int)), this, SLOT(slot_finished(int)) );
}

DlgOptions::~DlgOptions()
{
    delete m_ui;
    delete m_settings;
}

DlgOptions::CheckSumType DlgOptions::checkSumType()
{
    if( !m_ui->radioButton_noneEnding->isChecked() )
        return eNoCheck;
    return m_checkSum;
}

void DlgOptions::loadSettings()
{
    QString txt = m_settings->value( "/Send/UserEnding" ).toString();
    m_ui->lineEdit_userEnding->setText( txt );
    QString mode = m_settings->value( "/Send/EndingMode", "None" ).toString();
    m_lineEnding.clear();
    if( mode == "None" )
    {
        m_ui->radioButton_noneEnding->setChecked( true );
    }else if( mode == "CR" )
    {
        m_ui->radioButton_CREnding->setChecked( true );
        m_lineEnding.append( '\r' );
    }else if( mode == "NLCR" )
    {
        m_ui->radioButton_NL_CREnding->setChecked( true );
        m_lineEnding.append( '\n' );
        m_lineEnding.append( '\r' );
    }else if( mode == "CRNL" )
    {
        m_ui->radioButton_CR_NLEnding->setChecked( true );
        m_lineEnding.append( '\r' );
        m_lineEnding.append( '\n' );
    }else if( mode == "User" )
    {
        m_ui->radioButton_userEnding->setChecked( true );
        HexConvert::TextToHex( txt, m_lineEnding );
    }else
    {
        m_ui->radioButton_noneEnding->setChecked( true );
        m_lineEnding.clear();
    }
    m_ui->lineEdit_userEnding->setEnabled( m_ui->radioButton_userEnding->isChecked() );
    QString type = m_settings->value( "/Send/CheckSum", "None" ).toString();
    if( type == "None" )
        m_checkSum = eNoCheck;
    else if( type == "Sum8" )
        m_checkSum = eSum8;
    else if( type == "Sum16" )
        m_checkSum = eSum16;
    else if( type == "CRC16" )
        m_checkSum = eCRC16;
    else if( type == "CRC32" )
        m_checkSum = eCRC32;
    else
    {
        type = "None";
        m_checkSum = eNoCheck;
    }
    setDefaultValue( m_ui->comboBox_checkSumType, type );
    type = m_settings->value( "/Send/Endian", "LittleEndian" ).toString();
    if( type == "BigEndian" )
    {
        m_endian = eBigEndian;
        m_ui->radioButton_bigEndian->setChecked( true );
    }else
    {
        m_endian = eLittleEndian;
        m_ui->radioButton_LittleEndian->setChecked( true );
    }

    m_saveLog = m_settings->value( "/Log/SaveLog", false ).toBool();
    m_logFileName = m_settings->value( "/Log/FileName" ).toString();
    m_maxLogSize = m_settings->value( "/Log/MaxSize", 0 ).toInt();
    m_ui->checkBox_saveLog->setChecked( m_saveLog );
    m_ui->lineEdit_logFileName->setText( m_logFileName );
    m_ui->spinBox_maxLogSize->setValue( m_maxLogSize );

    m_buffSize = m_settings->value( "/Display/BufferSize", 125 ).toInt();
    m_ui->spinBox_buffSize->setValue( m_buffSize );
    QString defaultFont = this->font().toString();
    m_font = m_settings->value( "/Display/Font", defaultFont ).toString();
    m_ui->lineEdit_font->setText( m_font );
    m_encode = m_settings->value( "/Display/Encode", "UTF-8" ).toString();
    setDefaultValue( m_ui->comboBox_encode, m_encode );
}

void DlgOptions::saveSettings()
{
    QString mode;
    if( m_ui->radioButton_noneEnding->isChecked() )
        mode = "None";
    else if( m_ui->radioButton_CREnding->isChecked() )
         mode = "CR";
    else if( m_ui->radioButton_NL_CREnding->isChecked() )
        mode = "NLCR";
    else if( m_ui->radioButton_CR_NLEnding->isChecked() )
        mode = "CRNL";
    else if( m_ui->radioButton_userEnding->isChecked() )
    {
        mode = "User";
        QString txt = m_ui->lineEdit_userEnding->text();
        m_lineEnding.clear();
        HexConvert::TextToHex( txt, m_lineEnding );
        txt.clear();
        HexConvert::HexToText( m_lineEnding, txt, 0 );
        m_settings->setValue( "/Send/UserEnding", txt );
    }else 
        mode = "None";
    m_settings->setValue( "/Send/EndingMode", mode );
    QString type = m_ui->comboBox_checkSumType->currentText();
    m_settings->setValue( "/Send/CheckSum", type );
    if( m_ui->radioButton_LittleEndian->isChecked() )
        type = "LittleEndian";
    else
        type = "BigEndian";
    m_settings->setValue( "/Send/Endian", type );

    m_saveLog = m_ui->checkBox_saveLog->isChecked();
    m_logFileName = m_ui->lineEdit_logFileName->text();
    m_maxLogSize = m_ui->spinBox_maxLogSize->value();
    m_settings->setValue( "/Log/SaveLog", m_saveLog );
    m_settings->setValue( "/Log/FileName", m_logFileName );
    m_settings->setValue( "/Log/MaxSize", m_maxLogSize );

    m_buffSize = m_ui->spinBox_buffSize->value();
    m_settings->setValue( "/Display/BufferSize", m_buffSize );
    m_font = m_ui->lineEdit_font->text();
    m_settings->setValue( "/Display/Font", m_font );
    m_encode = m_ui->comboBox_encode->currentText();
    m_settings->setValue( "/Display/Encode", m_encode );

    m_settings->sync();
    loadSettings();
}

void DlgOptions::showEvent(QShowEvent *e)
{
    loadSettings();
    return QDialog::showEvent( e );
}

void DlgOptions::slot_finished( int result )
{
    if( result == QDialog::Accepted )
    {
        saveSettings();
        emit optionsChanged();
    }
}

void DlgOptions::on_pushButton_open_clicked()
{
    QFileDialog dlg( this );
    dlg.setWindowTitle( tr("Select a File") );
    dlg.setAcceptMode( QFileDialog::AcceptOpen );
    if( dlg.exec() == QDialog::Accepted )
    {
        QString path = dlg.selectedFiles().at(0);
        m_ui->lineEdit_logFileName->setText( path );
    }
}
void DlgOptions::on_pushButton_font_clicked()
{
    QFont oldFont;
    oldFont.fromString( m_font );
    bool ok;
    QFont font = QFontDialog::getFont( &ok, oldFont, this );
    if (ok) {
        m_font = font.toString();
        m_ui->lineEdit_font->setText( m_font );
    }
}
