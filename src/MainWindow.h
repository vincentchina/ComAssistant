#ifndef __MAIN_WINDOWS_H__
#define __MAIN_WINDOWS_H__

#include <QMainWindow>
#include <QFile>
#include <QTimer>
#include "JobThread.h"
#include "DlgOptions.h"

class QLabel;
class Ui_MainWindow;
class UartDriver;
class QStatusBar;
class DlgASCIITable;

class MainWindow : public QMainWindow{
    Q_OBJECT
public:
    MainWindow( QWidget* parent = 0 );
    ~MainWindow();

public slots:
    void slot_scanResult( const QString& lst );
    void slot_receiveData( int count );
    void slot_sendLoopTimeout();
    void slot_optionsChanged();

    void on_actionStart_triggered();
    void on_actionScan_triggered();
    void on_actionClear_triggered();
    void on_actionPause_triggered();
    void on_actionStop_triggered();
    void on_actionOptions_triggered();
    void on_actionASCIITable_triggered();
    void on_pushButton_send_clicked();
    void on_checkBox_sendLoop_toggled(bool);
    void on_comboBox_history_currentIndexChanged( int );

protected:
    static void job_scan( void* data );
    static void job_receive( void* data );

    virtual void closeEvent( QCloseEvent * );

    void appendReceiveText( const QString& txt );
    void updateUI( bool devIsOpen );
    void sendData();
    void formatDisplayData( unsigned char* buff, int count, bool isSend );

protected:
    enum { BUFF_SIZE = 1024 };

    class StatusBar{
    public:
        StatusBar( QStatusBar* bar, QWidget* parent = 0 );
        ~StatusBar();
        void showMessage( const QString& txt, int timeout );
        void setComInfo( const QString& txt );
        void increaseRx( const unsigned int count );
        void increaseTx( const unsigned int count );
        void clearRxTx();

    private:
        static void increase( QLabel* label, const QString& prefix, unsigned int& x, const unsigned int count );

    protected:
        QLabel* labelComInfo;
        QLabel* labelRx;
        QLabel* labelTx;
        QStatusBar* bar;
        unsigned int rx;
        unsigned int tx;
    };
    StatusBar*      m_statusBar;
    JobThread       m_jobThread;
    Ui_MainWindow*  m_ui;
    UartDriver*     m_uart;
    int             m_jobId;
    unsigned char   m_receiveBuff[BUFF_SIZE];
    bool            m_buffIsBusy;
    bool            m_pause;
    QFile           m_logFile;
    QTimer          m_loopTimer;
    DlgOptions*     m_dlgOptions;
    DlgASCIITable*  m_dlgASCIITable;
    unsigned int    m_currentLength;
    int             m_currentHistoryIndex;
};
#endif
