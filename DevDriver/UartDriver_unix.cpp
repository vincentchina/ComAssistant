#include "UartDriver.h"
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/io.h>
#include <termios.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>

#define INVALID_HANDLE_VALUE -1
#define READ_TIMEOUT_UNIT   1000

UartDriver::UartDriver()
{
	m_uart = INVALID_HANDLE_VALUE;
	m_portName[0] = 0;
	m_config.baudRate = BAUD9600;
	m_config.dataBits = DATA_8;
	m_config.flowControl = FLOW_OFF;
	m_config.parity = PAR_NONE;
	m_config.stopBits = STOP_2;
	m_config.timeout = 0;
}

UartDriver::~UartDriver()
{
}

bool UartDriver::isOpen()
{
	return ( m_uart != INVALID_HANDLE_VALUE );
}

bool UartDriver::open()
{
	if ( !isOpen() )
	{
		unsigned long baud = B9600;
		struct termios currentTermios;
		m_uart = ::open( m_portName, O_RDWR | O_NOCTTY );
		if ( m_uart == INVALID_HANDLE_VALUE )
			return false;
		::tcgetattr( m_uart, &currentTermios );   // Save the old termios
		::cfmakeraw( &currentTermios ); // Enable raw access

		/*set up other port settings*/
		currentTermios.c_cflag |= CREAD | CLOCAL;
		currentTermios.c_lflag &= ( ~( ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG ) );
		currentTermios.c_iflag &= ( ~( INPCK | IGNPAR | PARMRK | ISTRIP | ICRNL | IXANY ) );
		currentTermios.c_oflag &= ( ~OPOST );
		currentTermios.c_cc[VMIN] = 0;
		currentTermios.c_cc[VTIME] = 1;

		const long vdisable = ::fpathconf( m_uart, _PC_VDISABLE );
		currentTermios.c_cc[VINTR] = vdisable;
		currentTermios.c_cc[VQUIT] = vdisable;
		currentTermios.c_cc[VSTART] = vdisable;
		currentTermios.c_cc[VSTOP] = vdisable;
		currentTermios.c_cc[VSUSP] = vdisable;

		switch ( m_config.baudRate )
		{
		case BAUD110:
			baud = B110;
			break;
		case BAUD300:
			baud = B300;
			break;
		case BAUD600:
			baud = B600;
			break;
		case BAUD1200:
			baud = B1200;
			break;
		case BAUD2400:
			baud = B2400;
			break;
		case BAUD4800:
			baud = B4800;
			break;
		case BAUD9600:
			baud = B9600;
			break;
		case BAUD19200:
			baud = B19200;
			break;
		case BAUD38400:
			baud = B38400;
			break;
		case BAUD57600:
			baud = B57600;
			break;
		case BAUD115200:
			baud = B115200;
			break;
		default:
			break;
		}
		::cfsetispeed( &currentTermios, baud );
		::cfsetospeed( &currentTermios, baud );

		switch ( m_config.parity )
		{
		case PAR_NONE:
			currentTermios.c_cflag &= ( ~PARENB );
			break;
		case PAR_EVEN:
			currentTermios.c_cflag &= ( ~PARODD );
			currentTermios.c_cflag |= PARENB;
			break;
		case PAR_ODD:
			currentTermios.c_cflag |= ( PARENB | PARODD );
			break;
		default:
			break;
		}
		currentTermios.c_cflag &= ~CSIZE;
		switch ( m_config.dataBits )
		{
		case DATA_5:
			currentTermios.c_cflag |= CS5;
			break;
		case DATA_6:
			currentTermios.c_cflag |= CS6;
			break;
		case DATA_7:
			currentTermios.c_cflag |= CS7;
			break;
		case DATA_8:
			currentTermios.c_cflag |= CS8;
			break;
		}

		switch ( m_config.stopBits )
		{
		case STOP_1:
			currentTermios.c_cflag &= ( ~CSTOPB );
			break;
		case STOP_2:
			currentTermios.c_cflag |= CSTOPB;
			break;
		default:
			break;
		}

		switch ( m_config.flowControl )
		{
		case FLOW_OFF:
			currentTermios.c_cflag &= ( ~CRTSCTS );
			currentTermios.c_iflag &= ( ~( IXON | IXOFF | IXANY ) );
			break;
		case FLOW_XONXOFF:
            currentTermios.c_cflag &= ( ~CRTSCTS );
			currentTermios.c_iflag |= ( IXON | IXOFF | IXANY );
			break;
		case FLOW_HARDWARE:
			currentTermios.c_cflag |= CRTSCTS;
			currentTermios.c_iflag &= ( ~( IXON | IXOFF | IXANY ) );
			break;
		}
        ::tcsetattr( m_uart, TCSAFLUSH, &currentTermios );
	}
	return isOpen();
}

void UartDriver::close()
{
	UARTHANDLE tmp = m_uart;
    m_uart = INVALID_HANDLE_VALUE;
	if ( tmp != INVALID_HANDLE_VALUE )
	{
		::tcflush( tmp, TCIOFLUSH );
		::close( tmp );
	}
}

int UartDriver::write( const void *data, int cb )
{
	if ( !isOpen() )
		return -1;
	return ::write( m_uart, data, cb );
}

int UartDriver::read( void *data, int cb )
{
	int ret;
	struct timeval tv;
	fd_set rd;

	if ( !isOpen() )
		return -1;

	FD_ZERO( &rd );
	FD_SET( m_uart, &rd );

	tv.tv_sec = m_config.timeout / 1000;
	tv.tv_usec = ( m_config.timeout % 1000 ) * 1000;

	ret = select( m_uart + 1, &rd, 0, 0, &tv );
	if ( ret == 0 ) //timeout
		return 0;
	else if ( ret > 0 )
	{
		ret = ::read( m_uart, data, cb );
		return ( ( ret == 0 ) ? -1 : ret );
	}
	else
		return -1;
}

void UartDriver::flush( FlushMod mod )
{
	unsigned int flag = 0;
	if ( !isOpen() )
		return;
	if ( mod == flushRead )
		flag = TCIFLUSH;
	else if ( mod == flushWrite )
		flag = TCOFLUSH;
	else
		flag = TCIOFLUSH;
	::tcflush( m_uart, flag );
}

bool UartDriver::setTimeout( int msec )
{
    m_config.timeout = msec;
	return true;
}

void UartDriver::setPortName( const char *port )
{
	unsigned int len = strlen( port );
	assert( len < sizeof( m_portName ) );
	memcpy( m_portName, port, len + 1 );
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
		m_config.parity = conf->parity;
		break;
	default:
		break;
	};
	switch ( conf->stopBits )
	{
	case STOP_1:
	case STOP_2:
		m_config.stopBits = conf->stopBits;
		break;
	default:
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

