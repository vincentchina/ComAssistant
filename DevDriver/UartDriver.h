#ifndef __UART_DRIVER_H__
#define __UART_DRIVER_H__

#ifdef WIN32
#include <Windows.h>
typedef HANDLE UARTHANDLE;
#else
typedef int UARTHANDLE;
#endif

class UartDriver
{
	struct PrivateData;
public:
	enum BaudRateType
	{
	        BAUD110 = 110,
	        BAUD300 = 300,
	        BAUD600 = 600,
	        BAUD1200 = 1200,
	        BAUD2400 = 2400,
	        BAUD4800 = 4800,
	        BAUD9600 = 9600,
	        BAUD19200 = 19200,
	        BAUD38400 = 38400,
	        BAUD57600 = 57600,
	        BAUD115200 = 115200
	};
	enum DataBitsType
	{
	        DATA_5 = 5,
	        DATA_6 = 6,
	        DATA_7 = 7,
	        DATA_8 = 8
	};
	enum ParityType
	{
	        PAR_NONE,
	        PAR_ODD,
	        PAR_EVEN,
	        PAR_MARK,               //WINDOWS ONLY
	        PAR_SPACE
	};
	enum StopBitsType
	{
	        STOP_1,
	        STOP_1_5,               //WINDOWS ONLY
	        STOP_2
	};
	enum FlowType
	{
	        FLOW_OFF,
	        FLOW_HARDWARE,
	        FLOW_XONXOFF
	};
	enum Mod
	{
	        ModErr = -1,
	        ModRS232,
	        ModRS485,
	        ModRS422
	};
	typedef struct tagConfigInfo
	{
		unsigned long   timeout;
		BaudRateType    baudRate;
		ParityType      parity;
		DataBitsType    dataBits;
		StopBitsType    stopBits;
		FlowType        flowControl;
	} ConfigInfo;

    enum FlushMod { flushRead = 1, flushWrite, flushAll };
public:
	UartDriver();
	~UartDriver();
	bool isOpen();
	bool open();
	void close();
#ifdef _WIN32
	bool reOpen();
#endif
	int write( const void *data, int cb );
	int read( void *data, int cb );
    void flush( FlushMod mod );

	bool setTimeout( int msec );

	void setPortName( const char *port );
	void config( const ConfigInfo *conf );
	ConfigInfo getConfig()
	{
		return m_config;
	}

	int changeMod( Mod mod );
	Mod getMod();

protected:
	UARTHANDLE  m_uart;
	ConfigInfo  m_config;
	char        m_portName[64];
};

const char *getPortName( unsigned char port );

#endif
