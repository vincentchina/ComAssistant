#include "MainWindow.h"
#include "ui_main.h"
#include "DevDriver/UartDriver.h"
#include "HexConvert.h"
#include "DlgASCIITable.h"
#include "DataChecker.h"

#include <QTextStream>
#include <QPainter>
#include <QToolBar>
#include <QTime>
#include <QTextDocumentFragment>
#include <QTextCodec>
#include <QSettings>
#include <QScrollBar>

#define KBYTE_SIZE  1024
#define MBYTE_SIZE  ( KBYTE_SIZE * 1024 )
#define GBYTE_SIZE  ( KBYTE_SIZE * 1024 )

#define MAX_HISTORY_TXT_LENGTH  128

MainWindow::StatusBar::StatusBar(  QStatusBar* bar, QWidget* parent )
{
    this->bar = bar;

    labelComInfo = new QLabel( parent );
    bar->addPermanentWidget( labelComInfo );

    labelRx = new QLabel( parent );
    bar->addPermanentWidget( labelRx );

    labelTx = new QLabel( parent );
    bar->addPermanentWidget( labelTx );

    bar->showMessage( MainWindow::tr( "ready" ), 2000 );

    clearRxTx();
    //setComInfo( MainWindow::tr("COM not open") );
}

MainWindow::StatusBar::~StatusBar()
{

}

void MainWindow::StatusBar::showMessage( const QString& txt, int timeout )
{
    return bar->showMessage( txt, timeout );
}

void MainWindow::StatusBar::setComInfo( const QString& txt )
{
    labelComInfo->setText( txt );
}

void MainWindow::StatusBar::increase( QLabel* label, const QString& prefix, unsigned int& x, const unsigned int count )
{
    x += count;
    double value;
    bool isInterger = false;
    const char* unit = "Bits";
    if( x > GBYTE_SIZE )
    {
        value = x / double( GBYTE_SIZE );
        unit = "GBits";
    }else if( x > MBYTE_SIZE )
    {
        value = x / double( MBYTE_SIZE );
        unit = "MBits";
    }else if( x > KBYTE_SIZE )
    {
        value = x / double( 1024 );
        unit = "KBits";
    }else
    {
        value = x;
        unit = "Bits";
        isInterger = true;
    }

    QString txt;
    if( isInterger )
        txt = QString( "%1%2%3" ).arg( prefix ).arg( x ).arg( unit );
    else
        txt = QString( "%1%2%3" ).arg( prefix ).arg( value, 0, 'f', 2 ).arg( unit );
    label->setText( txt );
}

void MainWindow::StatusBar::increaseRx( const unsigned int count )
{
    increase( labelRx, MainWindow::tr("Rx: "), rx, count );
}

void MainWindow::StatusBar::increaseTx( const unsigned int count )
{
    increase( labelTx, MainWindow::tr("Tx: "), tx, count );
}

void MainWindow::StatusBar::clearRxTx()
{
    tx = 0;
    rx = 0;
    increaseRx( 0 );
    increaseTx( 0 );
}

MainWindow::MainWindow( QWidget* parent ) : QMainWindow( parent )
{
    m_ui = new Ui_MainWindow();
    m_ui->setupUi( this );
    
    m_ui->toolBar_editor->addAction( m_ui->actionScan );
    m_ui->toolBar_editor->addSeparator();
    m_ui->toolBar_editor->addAction( m_ui->actionStart );
    m_ui->toolBar_editor->addAction( m_ui->actionPause );
    m_ui->toolBar_editor->addAction( m_ui->actionStop );
    m_ui->toolBar_editor->addSeparator();
    m_ui->toolBar_editor->addAction( m_ui->actionOptions );
    m_ui->toolBar_editor->addSeparator();
    m_ui->toolBar_editor->addAction( m_ui->actionClear );

    m_ui->actionPause->setCheckable( true );

    m_ui->plainTextEdit_send->setMaximumBlockCount( 2 );
    m_ui->spinBox_loopTime->setRange( 10, 360000 );

    m_ui->radioButton_receiveText->setChecked( true );
    m_ui->radioButton_sendText->setChecked( true );

#ifdef Q_OS_WIN
    on_actionScan_triggered();
#endif

    updateUI( false );

    m_uart = new UartDriver;
    m_jobId = JobThread::InvalidID;
    m_buffIsBusy = false;
    m_pause = false;
    m_currentLength = 0;

    m_statusBar = new StatusBar( m_ui->statusbar, this );
    m_dlgOptions = new DlgOptions( this );
    m_dlgASCIITable = new DlgASCIITable( this );

    slot_optionsChanged();

    m_loopTimer.setSingleShot( false );
    connect( &m_loopTimer, SIGNAL(timeout()), this, SLOT( slot_sendLoopTimeout() ) );
    connect( m_dlgOptions, SIGNAL(optionsChanged()), this, SLOT( slot_optionsChanged() ) );

    QSettings* settings = m_dlgOptions->settings();
    setDefaultValue( m_ui->comboBox_baudrate, settings->value( "/MainPort/BaudRate", "115200" ).toString()  );
    setDefaultValue( m_ui->comboBox_dataBits, settings->value( "/MainPort/DataBits", "8" ).toString() );
    setDefaultValue( m_ui->comboBox_parity, settings->value( "/MainPort/Parity", "None" ).toString() );
    setDefaultValue( m_ui->comboBox_stopBits, settings->value( "/MainPort/StopBits", "1" ).toString() );
    setDefaultValue( m_ui->comboBox_flowType, settings->value( "/MainPort/FlowType", "None" ).toString() );

    QString value = settings->value( "/MainReceive/Mode", "Text" ).toString();
    bool check = ( value == "Text" );
    m_ui->radioButton_receiveText->setChecked( check );
    m_ui->radioButton_receiveHex->setChecked( !check );
    check = settings->value( "/MainReceive/AutoFeedLine", false ).toBool();
    m_ui->checkBox_feedLine->setChecked( check );
    check = settings->value( "/MainReceive/DisplaySend", false ).toBool();
    m_ui->checkBox_displaySend->setChecked( check );
    check = settings->value( "/MainReceive/DisplayTime", false ).toBool();
    m_ui->checkBox_distplayTime->setChecked( check );

    value = settings->value( "/MainSend/Mode", "Text" ).toString();
    check = ( value == "Text" );
    m_ui->radioButton_sendText->setChecked( check );
    m_ui->radioButton_sendHex->setChecked( !check );
    m_ui->spinBox_loopTime->setValue( settings->value("/MainSend/LoopTime", 10 ).toInt() ); //must before checkBox_sendLoop
    check = settings->value( "/MainSend/Loop", false ).toBool();
    m_ui->checkBox_sendLoop->setChecked( check );
    check = settings->value( "/MainSend/LoopPerLine", false ).toBool();
    m_ui->checkBox_loopPerLine->setChecked( check );

    m_currentHistoryIndex = settings->value( "/MainSend/CurrentHistoryIndex", 0 ).toInt();
    if( m_currentHistoryIndex < 0 || m_currentHistoryIndex > m_ui->comboBox_history->count() )
        m_currentHistoryIndex = 0;
    m_ui->comboBox_history->setCurrentIndex( m_currentHistoryIndex );
    QString key = QString( "/MainSend/History_%1" ).arg( m_currentHistoryIndex );
    value = settings->value( key ).toString();
    m_ui->plainTextEdit_send->setPlainText( value );
}

MainWindow::~MainWindow()
{
    m_uart->close();
    m_jobThread.stop();
    delete m_ui;
    delete m_uart;
    delete m_statusBar;
}
void MainWindow::closeEvent( QCloseEvent * event )
{
    QSettings* settings = m_dlgOptions->settings();
    settings->setValue( "/MainPort/BaudRate", m_ui->comboBox_baudrate->currentText() );
    settings->setValue( "/MainPort/DataBits", m_ui->comboBox_dataBits->currentText() );
    settings->setValue( "/MainPort/Parity", m_ui->comboBox_parity->currentText() );
    settings->setValue( "/MainPort/StopBits", m_ui->comboBox_stopBits->currentText() );
    settings->setValue( "/MainPort/FlowType", m_ui->comboBox_flowType->currentText() );

    bool check = m_ui->radioButton_receiveText->isChecked();
    if( check )
        settings->setValue( "/MainReceive/Mode", "Text" );
    else
        settings->setValue( "/MainReceive/Mode", "Hex" );
    settings->setValue( "/MainReceive/AutoFeedLine", m_ui->checkBox_feedLine->isChecked() );
    settings->setValue( "/MainReceive/DisplaySend", m_ui->checkBox_displaySend->isChecked() );
    settings->value( "/MainReceive/DisplayTime", m_ui->checkBox_distplayTime->isChecked() );

    check = m_ui->radioButton_sendText->isChecked();
    if( check )
        settings->setValue( "/MainSend/Mode", "Text" );
    else
        settings->setValue( "/MainSend/Mode", "Hex" );
    settings->setValue( "/MainSend/Loop", m_ui->checkBox_sendLoop->isChecked() );
    settings->setValue( "/MainSend/LoopPerLine", m_ui->checkBox_loopPerLine->isChecked() );
    settings->setValue("/MainSend/LoopTime", m_ui->spinBox_loopTime->value() );
    QString txt = m_ui->plainTextEdit_send->toPlainText();
    QString key = QString( "/MainSend/History_%1" ).arg( m_currentHistoryIndex );
    settings->setValue( key, txt.left( MAX_HISTORY_TXT_LENGTH ) );
    settings->setValue( "/MainSend/CurrentHistoryIndex", m_currentHistoryIndex );
    return QMainWindow::closeEvent( event );
}

void MainWindow::appendReceiveText( const QString& txt )
{
    QTextCursor cursorOld = m_ui->plainTextEdit_receive->textCursor();
    unsigned int maxSize = m_dlgOptions->displayBufferSize() * KBYTE_SIZE;
    if( m_currentLength + txt.length() > maxSize )
    {
        unsigned int reservedSize = maxSize / 2;
        QTextCursor cursor = m_ui->plainTextEdit_receive->textCursor();
        cursor.clearSelection();
        cursor.setPosition( 0 );
        cursor.setPosition( reservedSize, QTextCursor::KeepAnchor );

        if( m_dlgOptions->isSaveLog() )
        {
            QString name = m_dlgOptions->logFileName();
            if( name != m_logFile.fileName() )
            {
                m_logFile.close();
                m_logFile.setFileName( name );
            }
            if( !m_logFile.isOpen() )
                m_logFile.open( QFile::Append );

            if( !m_logFile.isOpen() )
                m_statusBar->showMessage( tr( "Log file open ERROR!" ), 2000 );
            else if( m_logFile.size() < m_dlgOptions->maxLogSize() * MBYTE_SIZE )
            {
                QString text = cursor.selection().toPlainText();
                QTextStream stream( &m_logFile );
                stream << text;
                //m_logFile.flush();
            }
        }
        
        cursor.removeSelectedText();
        m_currentLength -= reservedSize;
    }
    m_currentLength += txt.length();
    QTextCursor cursor = m_ui->plainTextEdit_receive->textCursor();
    cursor.clearSelection();
    cursor.movePosition(QTextCursor::End);
    m_ui->plainTextEdit_receive->setTextCursor( cursor );
    m_ui->plainTextEdit_receive->insertPlainText( txt );
    m_ui->plainTextEdit_receive->setTextCursor( cursorOld );
    QScrollBar* vBar = m_ui->plainTextEdit_receive->verticalScrollBar();
    vBar->setSliderPosition( vBar->maximum() );
}

void MainWindow::updateUI( bool devIsOpen )
{
    m_ui->actionStart->setDisabled( devIsOpen );
    m_ui->actionStop->setEnabled( devIsOpen );
    m_ui->actionPause->setEnabled( devIsOpen );
#ifdef Q_OS_WIN
    m_ui->actionScan->setDisabled( devIsOpen );
#else
    m_ui->actionScan->setDisabled( true );
    m_ui->comboBox_port->setEditable( true );
#endif
    m_ui->groupBoxPort->setDisabled( devIsOpen );
    m_ui->pushButton_send->setEnabled( devIsOpen );
}

void MainWindow::slot_scanResult( const QString& result )
{
    if( !m_ui->comboBox_port->isEnabled() )
        return;

    QStringList lst = result.split(',');
    m_ui->comboBox_port->clear();
    foreach( QString name, lst )
    {
        if( !name.isEmpty() )
            m_ui->comboBox_port->addItem( name );
    }
}

void MainWindow::slot_receiveData( int count )
{
    if( count <= 0 )
    {
        m_buffIsBusy = false;
        if( count < 0 )
        {
            if( m_ui->actionStop->isEnabled() )
            {
                m_statusBar->showMessage( tr("COM Read ERROR!"), 5000 );
                on_actionStop_triggered();
            }
        }
        return;
    }

    m_statusBar->increaseRx( count );
    if( m_pause )
    {
        m_buffIsBusy = false;
        return;
    }

    formatDisplayData( m_receiveBuff, count, false );
    
    m_buffIsBusy = false;
}

void MainWindow::formatDisplayData( unsigned char* buff, int count, bool isSend )
{
    QString txt;
    bool isDiaplayTime = m_ui->checkBox_distplayTime->isChecked(); 
    if( isDiaplayTime )
        txt = QTime::currentTime().toString("[hh:mm:ss.zzz] ");

    bool isDisplayRTx = m_ui->checkBox_displaySend->isChecked();
    if( isDisplayRTx )
    {
        if( isSend )
            txt += "Tx: ";
        else
            txt += "Rx: ";
    }

    if( m_ui->radioButton_receiveText->isChecked() )
    {
        QTextCodec *codec = QTextCodec::codecForName( m_dlgOptions->textEncode().toLatin1() );
        if( codec )
            txt += codec->toUnicode( (char*)buff, count );
        else
            txt += QString::fromUtf16( (ushort*)buff, count / 2 );
    }else 
    {
        if( txt.isEmpty() )
            txt = ' ';
        HexConvert::HexToText( buff, count, txt );
    }

    if( isDisplayRTx || isDiaplayTime || m_ui->checkBox_feedLine->isChecked() )
        txt += '\n';

    appendReceiveText( txt );
}

void MainWindow::slot_sendLoopTimeout()
{
    if( !m_ui->actionStart->isEnabled() )
        sendData();
}

void MainWindow::slot_optionsChanged()
{
    QFont font;
    if( font.fromString ( m_dlgOptions->fontName() ) )
    {
        m_ui->plainTextEdit_receive->setFont( font );
        m_ui->plainTextEdit_send->setFont( font );
    }
}

void MainWindow::job_scan( void* data )
{
    MainWindow* pThis = static_cast<MainWindow*>( data );
    QString result;
    for ( int i = 0; i < 50; ++i )
    {
        UartDriver uart;
        QString portName = QString( "COM%1" ).arg( i );
        uart.setPortName( portName.toLatin1() );
        if( uart.open() )
            result.append( portName + "," );
    }
    if( !result.isEmpty() )
        QMetaObject::invokeMethod( pThis, "slot_scanResult", Q_ARG( QString, result ) );
}

void MainWindow::job_receive( void* data )
{
    MainWindow* pThis = static_cast<MainWindow*>( data );
    if( pThis->m_buffIsBusy )
    {
        //pThis->m_jobThread.msleep( 10 );
        return;
    }
    int ret = pThis->m_uart->read( pThis->m_receiveBuff, BUFF_SIZE );
    pThis->m_buffIsBusy = true;
    QMetaObject::invokeMethod( pThis, "slot_receiveData",  Q_ARG( int, ret ) );
}

void MainWindow::on_actionStart_triggered()
{
    QString portName = m_ui->comboBox_port->currentText();
    if ( portName.isEmpty() )
        return;

    m_uart->setPortName( portName.toLocal8Bit() );
    UartDriver::ConfigInfo conf;
    conf.baudRate = (UartDriver::BaudRateType)m_ui->comboBox_baudrate->currentText().toUInt();
    conf.dataBits = (UartDriver::DataBitsType)m_ui->comboBox_dataBits->currentText().toUInt();
    conf.flowControl = UartDriver::FLOW_OFF;
    conf.parity = (UartDriver::ParityType)m_ui->comboBox_parity->currentIndex();
    conf.stopBits = (UartDriver::StopBitsType)m_ui->comboBox_stopBits->currentIndex();
    conf.timeout = 10;
    m_uart->config( &conf );
    bool ok = m_uart->open();
    updateUI( ok );
    if( ok )
    {
        m_uart->setTimeout( conf.timeout );
        m_jobId = m_jobThread.startLoopJob( job_receive, this );
    }
}

void MainWindow::on_actionScan_triggered()
{
    m_ui->comboBox_port->clear();
    m_jobThread.startOneJob( job_scan, this );
}

void MainWindow::on_actionClear_triggered()
{
    m_ui->plainTextEdit_receive->clear();
    m_statusBar->clearRxTx();
}

void MainWindow::on_actionPause_triggered()
{
    m_pause = !m_pause;
    m_ui->actionPause->setChecked( m_pause );
}

void MainWindow::on_actionStop_triggered()
{
    on_checkBox_sendLoop_toggled( false );
    m_jobThread.stopJob( m_jobId );
    m_uart->close();
    m_logFile.close();
    updateUI( false );
}

void MainWindow::on_actionOptions_triggered()
{
    m_dlgOptions->exec();
}

void MainWindow::on_actionASCIITable_triggered()
{
    if( !m_dlgASCIITable->isVisible() )
        m_dlgASCIITable->show();
}

void MainWindow::on_pushButton_send_clicked()
{
    on_checkBox_sendLoop_toggled( m_ui->checkBox_sendLoop->isChecked() );
    sendData();
}

void MainWindow::on_checkBox_sendLoop_toggled( bool checked )
{
    if( checked )
    {
        int loopTime = m_ui->spinBox_loopTime->text().toInt();
        if( loopTime <= 0 )
        {
            m_ui->checkBox_sendLoop->setChecked( false );
            return;
        }
        if( !m_loopTimer.isActive() )
            m_loopTimer.start( loopTime );
    }else
        m_loopTimer.stop();
    m_ui->spinBox_loopTime->setDisabled( checked );
}

void MainWindow::on_comboBox_history_currentIndexChanged( int index )
{
    if( m_currentHistoryIndex == index )
        return;

    QSettings* settings = m_dlgOptions->settings();
    QString txt = m_ui->plainTextEdit_send->toPlainText();
    QString key = QString( "/MainSend/History_%1" ).arg( m_currentHistoryIndex );
    settings->setValue( key, txt.left( MAX_HISTORY_TXT_LENGTH ) );
    key = QString( "/MainSend/History_%1" ).arg( index );
    txt = settings->value( key ).toString();
    m_ui->plainTextEdit_send->setPlainText( txt );
    m_currentHistoryIndex = index;
}

void MainWindow::sendData()
{
    QString txt = m_ui->plainTextEdit_send->toPlainText();
    QByteArray ba;
    if( m_ui->radioButton_sendText->isChecked() )
    {
        QTextCodec *codec = QTextCodec::codecForName( m_dlgOptions->textEncode().toLatin1() );
        if( codec )
            ba = codec->fromUnicode( txt );
        else
            ba.setRawData( (char*)txt.unicode(), txt.length() * 2 );
    }else
    {
        HexConvert::TextToHex( txt, ba );
    }
    DlgOptions::CheckSumType checkSum = m_dlgOptions->checkSumType();
    if( checkSum == DlgOptions::eNoCheck )
        ba.append( m_dlgOptions->lineEnding() );
    else
    {
        int checkSize = 0;
        unsigned int reslut = 0;
        if( checkSum == DlgOptions::eSum8 )
        {
            checkSize = 1;
            reslut = DataChecker::sum8( ba.data(), ba.size() );
        }else if( checkSum == DlgOptions::eSum16 )
        {
            checkSize = 2;
            reslut = DataChecker::sum16( ba.data(), ba.size() );
        }else if( checkSum == DlgOptions::eCRC16 )
        {
            checkSize = 2;
            reslut = DataChecker::crc16( ba.data(), ba.size() );
        }else if( checkSum == DlgOptions::eCRC32 )
        {
            checkSize = 4;
            reslut = DataChecker::crc32( ba.data(), ba.size() );
        }
        if( m_dlgOptions->endianType() == DlgOptions::eBigEndian )
            DataChecker::toBigEndian( &reslut, checkSize );
        ba.append( (char*)&reslut, checkSize );
    }
    int ret = m_uart->write( ba.data(), ba.size() );
    if( ret > 0 )
    {
        if( m_ui->checkBox_displaySend->isChecked() )
            formatDisplayData( (unsigned char*)ba.data(), ba.size(), true );
        m_statusBar->increaseTx( ret );
    }
}
