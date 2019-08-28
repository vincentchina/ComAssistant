#include "UartDriver.h"
#include <Windows.h>
#include <assert.h>

#define READ_TIMEOUT_UNIT   1000

UartDriver::UartDriver() :
	m_uart( INVALID_HANDLE_VALUE )
	//m_config;
	//m_portName[64];
{
	m_uart = INVALID_HANDLE_VALUE;
	m_portName[0] = '\\';
	m_portName[1] = '\\';
	m_portName[2] = '.';
	m_portName[3] = '\\';
	m_config.baudRate = BAUD9600;
	m_config.dataBits = DATA_8;
	m_config.flowControl = FLOW_OFF;
	m_config.parity = PAR_NONE;
	m_config.stopBits = STOP_2;
	m_config.timeout = 0;
}

UartDriver::~UartDriver()
{
	close();
}

bool UartDriver::isOpen()
{
	return ( m_uart != INVALID_HANDLE_VALUE );
}

bool UartDriver::open()
{
	if ( isOpen() )
		return true;

	m_uart = CreateFileA( m_portName,
	                      GENERIC_READ | GENERIC_WRITE,
	                      0,
	                      NULL,
	                      OPEN_EXISTING,
	                      FILE_ATTRIBUTE_NORMAL,
	                      NULL );
	if ( m_uart == INVALID_HANDLE_VALUE )
		return false;

	COMMCONFIG commConfig;
	DWORD confSize = sizeof( COMMCONFIG );
	commConfig.dwSize = confSize;
	GetCommConfig( m_uart, &commConfig, &confSize );
	GetCommState( m_uart, &( commConfig.dcb ) );
	/*set up parameters*/
	commConfig.dcb.fBinary = TRUE;
	commConfig.dcb.fInX = FALSE;
	commConfig.dcb.fOutX = FALSE;
	commConfig.dcb.fAbortOnError = FALSE;
	commConfig.dcb.fNull = FALSE;
	/* Dtr default to true. See Issue 122*/
	commConfig.dcb.fDtrControl = TRUE;

	//fill struct : COMMCONFIG
	commConfig.dcb.BaudRate = m_config.baudRate;
	commConfig.dcb.Parity = ( BYTE )m_config.parity;
	commConfig.dcb.fParity = ( m_config.parity == PAR_NONE ) ? FALSE : TRUE;
	commConfig.dcb.ByteSize = ( BYTE )m_config.dataBits;
	switch ( m_config.stopBits )
	{
	case STOP_1:
		commConfig.dcb.StopBits = ONESTOPBIT;
		break;
	case STOP_1_5:
		commConfig.dcb.StopBits = ONE5STOPBITS;
		break;
	case STOP_2:
		commConfig.dcb.StopBits = TWOSTOPBITS;
		break;
	}
	switch ( m_config.flowControl )
	{
	case FLOW_OFF:
		commConfig.dcb.fOutxCtsFlow = FALSE;
		commConfig.dcb.fRtsControl = RTS_CONTROL_DISABLE;
		commConfig.dcb.fInX = FALSE;
		commConfig.dcb.fOutX = FALSE;
		break;
		/*software (XON/XOFF) flow control*/
	case FLOW_XONXOFF:
		commConfig.dcb.fOutxCtsFlow = FALSE;
		commConfig.dcb.fRtsControl = RTS_CONTROL_DISABLE;
		commConfig.dcb.fInX = TRUE;
		commConfig.dcb.fOutX = TRUE;
		break;
		/*hardware flow control*/
	case FLOW_HARDWARE:
		commConfig.dcb.fOutxCtsFlow = TRUE;
		commConfig.dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
		commConfig.dcb.fInX = FALSE;
		commConfig.dcb.fOutX = FALSE;
		break;
	}

	setTimeout( m_config.timeout );
	SetCommConfig( m_uart, &commConfig, sizeof( COMMCONFIG ) );
	PurgeComm( m_uart, PURGE_TXCLEAR | PURGE_RXCLEAR );
	return true;
}

bool UartDriver::reOpen()
{
	unsigned long timeout = m_config.timeout;
	while ( true )
	{
		close();
		Sleep( READ_TIMEOUT_UNIT );
		if ( open() )
			return true;
		if ( timeout > READ_TIMEOUT_UNIT )
			timeout -= READ_TIMEOUT_UNIT;
		else
			return false;
	}
	return false;
}

void UartDriver::close()
{
	UARTHANDLE tmp = m_uart;
	m_uart = INVALID_HANDLE_VALUE; //多线程，需要先置为INVALID_HANDLE_VALUE
	if ( tmp != INVALID_HANDLE_VALUE )
		CloseHandle( tmp );
}

int UartDriver::write( const void *data, int cb )
{
	DWORD dwRealSend = 0;
	if ( !isOpen() || !WriteFile( m_uart, data, cb, &dwRealSend, 0 ) )
	{
		close();
		return -1;
	}
	return dwRealSend;
}

int UartDriver::read( void *data, int cb )
{
	DWORD dwRealRead = 0;
	unsigned long tryCount = ( m_config.timeout + READ_TIMEOUT_UNIT - 1 ) / READ_TIMEOUT_UNIT ;

	while ( isOpen() && tryCount && dwRealRead == 0 )
	{
		if ( !ReadFile( m_uart, data, cb, &dwRealRead, 0 ) )
		{
			close();
			return -1;
		}
		--tryCount;
	}
	return ( !isOpen() ? -1 : dwRealRead );
}

bool UartDriver::setTimeout( int msec )
{
	m_config.timeout = msec;
	if ( isOpen() )
	{
		COMMTIMEOUTS commTimeouts;
		COMMTIMEOUTS oldCommTimeouts;

		if ( m_config.timeout == 0 )
		{
			commTimeouts.ReadIntervalTimeout = MAXDWORD;
			commTimeouts.ReadTotalTimeoutConstant = 0;
		}
		else
		{
			commTimeouts.ReadIntervalTimeout = 100;

			if ( m_config.timeout < READ_TIMEOUT_UNIT )
				commTimeouts.ReadTotalTimeoutConstant = m_config.timeout;
			else
				commTimeouts.ReadTotalTimeoutConstant = READ_TIMEOUT_UNIT;
		}
		commTimeouts.ReadTotalTimeoutMultiplier = 0;
		commTimeouts.WriteTotalTimeoutMultiplier = m_config.timeout;
		commTimeouts.WriteTotalTimeoutConstant = 0;

		GetCommTimeouts( m_uart, &oldCommTimeouts );
		if ( memcmp( &oldCommTimeouts, &commTimeouts, sizeof( commTimeouts ) ) )
			SetCommTimeouts( m_uart, &commTimeouts );
	}
	return true;
}

void UartDriver::setPortName( const char *port )
{
	int len = strlen( port ) + 5;
	assert( len < sizeof( m_portName ) );
	memcpy( m_portName + 4, port, len + 1 );
}

void UartDriver::config( const ConfigInfo *conf )
{
	m_config.timeout = conf->timeout;
	switch ( conf->baudRate )
	{
	case BAUD110:
	case BAUD300:
	case BAUD600:
	case BAUD1200:
	case BAUD2400:
	case BAUD4800:
	case BAUD9600:
	case BAUD19200:
	case BAUD38400:
	case BAUD57600:
	case BAUD115200:
		m_config.baudRate = conf->baudRate;
		break;
	}
	switch ( conf->dataBits )
	{
	case DATA_5:
	case DATA_6:
	case DATA_7:
	case DATA_8:
		m_config.dataBits = conf->dataBits;
		break;
	}
	switch ( conf->parity )
	{
	case PAR_NONE:
	case PAR_ODD:
	case PAR_EVEN:
	case PAR_MARK:
	case PAR_SPACE:
		m_config.parity = conf->parity;
		break;
	};
	switch ( conf->stopBits )
	{
	case STOP_1:
	case STOP_1_5:
	case STOP_2:
		m_config.stopBits = conf->stopBits;
		break;
	};
	switch ( conf->flowControl )
	{
	case FLOW_OFF:
	case FLOW_HARDWARE:
	case FLOW_XONXOFF:
		m_config.flowControl = conf->flowControl;
		break;
	}
}

