#ifndef __DLG_OPTIONS_H__
#define __DLG_OPTIONS_H__

#include <QDialog>
#include <QComboBox>

class Ui_DialogOptions;
class QSettings;
class QShowEvent;

inline void setDefaultValue( QComboBox* combo, const QString& value )
{
    combo->setCurrentIndex( combo->findText( value ) );
}

class DlgOptions : public QDialog
{
    Q_OBJECT
public:
    DlgOptions( QWidget* parent = 0 );
    ~DlgOptions();
    enum CheckSumType{ eNoCheck, eSum8, eSum16, eCRC16, eCRC32 };
    enum EndianType{ eLittleEndian, eBigEndian };
    inline const QByteArray& lineEnding()
    {
        return m_lineEnding;
    }
    inline bool isSaveLog() const
    {
        return m_saveLog;
    }
    inline QString logFileName() const
    {
        return m_logFileName;
    }
    inline unsigned int maxLogSize() const
    {
        return m_maxLogSize;
    }
    inline unsigned int displayBufferSize() const
    {
        return m_buffSize;
    }
    inline const QString fontName()
    {
        return m_font;
    }
    inline const QString textEncode()
    {
        return m_encode;
    }
    CheckSumType checkSumType();
    inline EndianType endianType()
    {
        return m_endian;
    }
    inline QSettings* settings()
    {
        return m_settings;
    }

protected:
    virtual void showEvent(QShowEvent *);

    void loadSettings();
    void saveSettings();

signals:
    void optionsChanged();

protected slots:
    void slot_finished(int result);
    void on_pushButton_open_clicked();
    void on_pushButton_font_clicked();

protected:
    Ui_DialogOptions*   m_ui;
    QSettings*          m_settings;
    
    QByteArray          m_lineEnding;
    CheckSumType        m_checkSum;
    EndianType          m_endian;

    bool                m_saveLog;
    QString             m_logFileName;
    unsigned int        m_maxLogSize;

    unsigned int        m_buffSize;
    QString             m_font;
    QString             m_encode;
    bool                m_fisplayColor;
    QString             m_receiveColor;
    QString             m_sendColor;
};
#endif
