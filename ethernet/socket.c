#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include "w5100.h"
#include "socket.h"

#define MAX_BUF 256 /* largest buffer we can read from chip */

W5100_CALLBACKS callbacks;

void EthernetInit()
{
	CS_PORT |= (1 << CS_BIT);	// pull CS pin high
	CS_DDR |= (1 << CS_BIT);	// now make it an output

	SPI_PORT = SPI_PORT | (1 << PORTB2);	// make sure SS is high
	SPI_DDR = (1 << PORTB3) | (1 << PORTB5) | (1 << PORTB2);	// set MOSI, SCK and SS as output, others as input
	SPCR = (1 << SPE) | (1 << MSTR);	// enable SPI, master mode 0
	SPSR |= (1 << SPI2X);	// set the clock rate fck/2

	callbacks._select = &my_select;	// callback for selecting the W5100
	callbacks._xchg = &my_xchg;	// callback for exchanging data
	callbacks._deselect = &my_deselect;	// callback for deselecting the W5100
	callbacks._reset = &my_reset;	// callback for hardware-reset of the W5100

	W51_register(&callbacks);	// register our target-specific W5100 routines with the W5100 library
	W51_init();		// now initialize the W5100
}

uint8_t Socket(uint8_t eth_protocol, uint16_t port)
{
	int16_t iter;
	uint8_t retval;
	uint16_t sockaddr;
	uint8_t sock = W5100_NUM_SOCKETS;

	retval = W5100_FAIL;

	for (iter = 0; iter < W5100_NUM_SOCKETS; iter++)
	{
		sockaddr = W5100_SKT_BASE(iter);
		uint8_t status = W51_read(sockaddr + W5100_SR_OFFSET);
		if (W51_read(sockaddr + W5100_SR_OFFSET) == W5100_SKT_SR_CLOSED)
		{
			sock = iter;
			break;
		}
		if (status == W5100_SKT_SR_FIN_WAIT)
		{
			Close(iter);
			sock = iter;
			break;
		}
		else if (status == W5100_SKT_SR_CLOSING)
		{
			Close(iter);
			sock = iter;
			break;
		}
		else if (status == W5100_SKT_SR_TIME_WAIT)
		{
			Close(iter);
			sock = iter;
			break;
		}
		else if (status == W5100_SKT_SR_LAST_ACK)
		{
			Close(iter);
			sock = iter;
			break;
		}
	}
	
	if (sock == W5100_NUM_SOCKETS)
		return retval;
	
	// found our available socket
	sockaddr = W5100_SKT_BASE(sock);	// calc base addr for this socket

	if (W51_read(sockaddr + W5100_SR_OFFSET) != W5100_SKT_SR_CLOSED)	// Make sure we close the socket first
	{
		Close(sock);
	}

	W51_write(sockaddr + W5100_MR_OFFSET, eth_protocol);	// set protocol for this socket
	W51_write(sockaddr + W5100_PORT_OFFSET, ((port & 0xFF00) >> 8));	// set port for this socket (MSB)
	W51_write(sockaddr + W5100_PORT_OFFSET + 1, (port & 0x00FF));	// set port for this socket (LSB)
	
	W51_write(sockaddr + W5100_CR_OFFSET, W5100_SKT_CR_OPEN);	// open the socket

	while (W51_read(sockaddr + W5100_CR_OFFSET)) ;	// loop until device reports socket is open (blocks!!)

	if (eth_protocol == W5100_SKT_MR_TCP)
	{
		if (W51_read(sockaddr + W5100_SR_OFFSET) == W5100_SKT_SR_INIT)
			retval = sock;	// if success, return socket number
		else
			Close(sock);	// if failed, close socket immediately
	}
	else if (eth_protocol == W5100_SKT_MR_UDP)
	{
		if (W51_read(sockaddr + W5100_SR_OFFSET) == W5100_SKT_SR_UDP)
			retval = sock;	// if success, return socket number
		else
			Close(sock);	// if failed, close socket immediately
	}
	else
	{
		Close(sock);	// if failed, close socket immediately
	}
	
	return retval;
}

uint8_t Bind(ADDRESS_CFG *bind_address)
{
	return W51_config(bind_address);
}

void Close(uint8_t sock)
{
	uint16_t sockaddr;

	if (sock > W5100_NUM_SOCKETS)
		return;		// if illegal socket number, ignore request
	sockaddr = W5100_SKT_BASE(sock);	// calc base addr for this socket

	W51_write(sockaddr + W5100_CR_OFFSET, W5100_SKT_CR_CLOSE);	// tell chip to close the socket
	while (W51_read(sockaddr + W5100_CR_OFFSET)) ;	// loop until socket is closed (blocks!!)
}

void Disconnect(uint8_t sock)
{
	uint16_t sockaddr;

	if (sock > W5100_NUM_SOCKETS)
		return;		// if illegal socket number, ignore request
	sockaddr = W5100_SKT_BASE(sock);	// calc base addr for this socket

	W51_write(sockaddr + W5100_CR_OFFSET, W5100_SKT_CR_DISCON);	// disconnect the socket
	while (W51_read(sockaddr + W5100_CR_OFFSET)) ;	// loop until socket is closed (blocks!!)
}

uint8_t Listen(uint8_t sock)
{
	uint8_t retval;
	uint16_t sockaddr;

	retval = W5100_FAIL;	// assume this fails
	if (sock > W5100_NUM_SOCKETS)
		return retval;	// if illegal socket number, ignore request

	sockaddr = W5100_SKT_BASE(sock);	// calc base addr for this socket
	if (W51_read(sockaddr + W5100_SR_OFFSET) == W5100_SKT_SR_INIT)	// if socket is in initialized state...
	{
		W51_write(sockaddr + W5100_CR_OFFSET, W5100_SKT_CR_LISTEN);	// put socket in listen state
		while (W51_read(sockaddr + W5100_CR_OFFSET)) ;	// block until command is accepted

		if (W51_read(sockaddr + W5100_SR_OFFSET) == W5100_SKT_SR_LISTEN)
			retval = W5100_OK;	// if socket state changed, show success
		else
			Close(sock);	// not in listen mode, close and show an error occurred
	}
	return retval;
}

// deprecated
uint8_t Accept_old(uint8_t sock, uint8_t eth_protocol, uint16_t port)
{
	uint8_t retval;
	uint16_t sockaddr;

	retval = W5100_FAIL;
	if (sock >= W5100_NUM_SOCKETS)
		return retval;

	sockaddr = W5100_SKT_BASE(sock);
	
	while(1)
	{
		switch(W51_read(sockaddr + W5100_SR_OFFSET))
		{
			case W5100_SKT_SR_CLOSED:
				/*if (Socket(eth_protocol, port) == sock)
				{
					Listen(sock);
					_delay_ms(1);
				}*/
			break;
			case W5100_SKT_SR_ESTABLISHED: return 1;
			case W5100_SKT_SR_FIN_WAIT:
			case W5100_SKT_SR_CLOSING:
			case W5100_SKT_SR_TIME_WAIT:
			case W5100_SKT_SR_CLOSE_WAIT:
			case W5100_SKT_SR_LAST_ACK:
				Close(sock);
				break;
		}
	}
}

uint8_t Accept(uint16_t port)
{
	int16_t iter;
	uint8_t sock;
	uint16_t sockaddr;
	uint8_t listening = 0;
	
	// check for established sockets
	for (iter = 0; iter < W5100_NUM_SOCKETS; iter++)
	{
		sockaddr = W5100_SKT_BASE(iter);
		uint8_t status = W51_read(sockaddr + W5100_SR_OFFSET);
		if (status == W5100_SKT_SR_ESTABLISHED || status == W5100_SKT_SR_CLOSE_WAIT)
		{
			sock = iter;
			return sock;
		}
	}
	
	// check for listening sockets
	for (iter = 0; iter < W5100_NUM_SOCKETS; iter++)
	{
		sockaddr = W5100_SKT_BASE(iter);
		uint8_t status = W51_read(sockaddr + W5100_SR_OFFSET);
		if (status == W5100_SKT_SR_LISTEN)
		{
			listening = 1;
			sock = iter;
		}
	}
	
	// if no listening sockets -> all are closed, open new one
	if (listening == 0)
	{
		sock = Socket(W5100_SKT_MR_TCP, port);
	}
	
	// wait for socket to be established
	sockaddr = W5100_SKT_BASE(sock);
	while(1)
	{
		switch(W51_read(sockaddr + W5100_SR_OFFSET))
		{
			case W5100_SKT_SR_CLOSED:
			break;
			case W5100_SKT_SR_ESTABLISHED: return sock;
			case W5100_SKT_SR_FIN_WAIT:
			case W5100_SKT_SR_CLOSING:
			case W5100_SKT_SR_TIME_WAIT:
			case W5100_SKT_SR_CLOSE_WAIT:
			case W5100_SKT_SR_LAST_ACK:
				Close(sock);
				return W5100_FAIL;
			break;
		}
	}
	
	//return sock;
}


uint8_t ConnectBasic(uint8_t sock, uint8_t *remote_ip, uint16_t remote_port)
{
	uint8_t retval;
	uint16_t sockaddr;

	retval = W5100_FAIL;
	if (sock >= W5100_NUM_SOCKETS)
		return retval;

	sockaddr = W5100_SKT_BASE(sock);
	
	if (W51_read(sockaddr + W5100_SR_OFFSET) == W5100_SKT_SR_INIT)	// if socket is in initialized state...
	{
		W51_write(sockaddr + W5100_DIPR_OFFSET, remote_ip[0]);
		W51_write(sockaddr + W5100_DIPR_OFFSET + 1, remote_ip[1]);
		W51_write(sockaddr + W5100_DIPR_OFFSET + 2, remote_ip[2]);
		W51_write(sockaddr + W5100_DIPR_OFFSET + 3, remote_ip[3]);
	
		W51_write(sockaddr + W5100_DPORT_OFFSET, ((remote_port & 0xFF00) >> 8));
		W51_write(sockaddr + W5100_DPORT_OFFSET + 1, (remote_port & 0x00FF));
	
		W51_write(sockaddr + W5100_CR_OFFSET, W5100_SKT_CR_CONNECT);
		while (W51_read(sockaddr + W5100_CR_OFFSET));
		
		retval = W5100_OK;
	}
	
	return retval;
}

uint8_t Connect(uint8_t sock, uint8_t *remote_ip, uint16_t remote_port)
{
	uint16_t sockaddr;

	if (sock >= W5100_NUM_SOCKETS)
		return W5100_FAIL;	// ignore illegal requests
	
	sockaddr = W5100_SKT_BASE(sock);
	
	if (ConnectBasic(sock, remote_ip, remote_port) == W5100_FAIL)
		return W5100_FAIL;
	
	while(1)
	{
		switch(W51_read(sockaddr + W5100_SR_OFFSET))
		{
			case W5100_SKT_SR_CLOSED:
			break;
			case W5100_SKT_SR_ESTABLISHED: return W5100_OK;
			case W5100_SKT_SR_FIN_WAIT:
			case W5100_SKT_SR_CLOSING:
			case W5100_SKT_SR_TIME_WAIT:
			case W5100_SKT_SR_CLOSE_WAIT:
			case W5100_SKT_SR_LAST_ACK:
				Close(sock);
				return W5100_FAIL;
			break;
		}
	}
}


uint8_t SendBasic(uint8_t sock, const uint8_t *buf, uint16_t buflen)
{
	uint16_t ptr;
	uint16_t offaddr;
	uint16_t realaddr;
	uint16_t txsize;
	uint16_t timeout;
	uint16_t sockaddr;

	if (buflen == 0 || sock >= W5100_NUM_SOCKETS)
		return W5100_FAIL;	// ignore illegal requests
	sockaddr = W5100_SKT_BASE(sock);	// calc base addr for this socket
	// Make sure the TX Free Size Register is available
	txsize = W51_read(sockaddr + W5100_TX_FSR_OFFSET);	// make sure the TX free-size reg is available
	txsize = (((txsize & 0x00FF) << 8) + W51_read(sockaddr + W5100_TX_FSR_OFFSET + 1));

	timeout = 0;
	while (txsize < buflen)
	{
		_delay_ms(1);

		txsize = W51_read(sockaddr + W5100_TX_FSR_OFFSET);	// make sure the TX free-size reg is available
		txsize =
		    (((txsize & 0x00FF) << 8) +
		     W51_read(sockaddr + W5100_TX_FSR_OFFSET + 1));

		if (timeout++ > 1000)	// if max delay has passed...
		{
			Disconnect(sock);	// can't connect, close it down
			return W5100_FAIL;	// show failure
		}
	}

	// Read the Tx Write Pointer
	ptr = W51_read(sockaddr + W5100_TX_WR_OFFSET);
	offaddr = (((ptr & 0x00FF) << 8) + W51_read(sockaddr + W5100_TX_WR_OFFSET + 1));

	while (buflen)
	{
		buflen--;
		realaddr = (W5100_TXBUFADDR + (0x0800 * sock)) + (offaddr & W5100_TX_BUF_MASK);	// calc W5100 physical buffer addr for this socket

		W51_write(realaddr, *buf);	// send a byte of application data to TX buffer
		offaddr++;	// next TX buffer addr
		buf++;		// next input buffer addr
	}
	
	W51_write(sockaddr + W5100_TX_WR_OFFSET, (offaddr & 0xFF00) >> 8);	// send MSB of new write-pointer addr
	W51_write(sockaddr + W5100_TX_WR_OFFSET + 1, (offaddr & 0x00FF));	// send LSB

	W51_write(sockaddr + W5100_CR_OFFSET, W5100_SKT_CR_SEND);	// start the send on its way
	while (W51_read(sockaddr + W5100_CR_OFFSET));
	//_delay_ms(5);
	return W5100_OK;
}

uint8_t Send(uint8_t sock, const uint8_t *buf, uint16_t buflen)
{
	uint16_t sockaddr;

	if (buflen == 0 || sock >= W5100_NUM_SOCKETS)
		return W5100_FAIL;	// ignore illegal requests
	
	sockaddr = W5100_SKT_BASE(sock);
	
	if (W51_read(sockaddr + W5100_SR_OFFSET) != W5100_SKT_SR_ESTABLISHED)
		return W5100_FAIL;
	
	return SendBasic(sock, buf, buflen);
}


uint8_t SendTo(uint8_t sock, const uint8_t *buf, uint16_t buflen, uint8_t *remote_ip, uint16_t remote_port)
{
	uint16_t ptr;
	uint16_t offaddr;
	uint16_t realaddr;
	uint16_t txsize;
	uint16_t timeout;
	uint16_t sockaddr;

	if (buflen == 0 || sock >= W5100_NUM_SOCKETS)
		return W5100_FAIL;	// ignore illegal requests
	
	sockaddr = W5100_SKT_BASE(sock);	// calc base addr for this socket

	// set destination ip address and port
	W51_write(sockaddr + W5100_DIPR_OFFSET, remote_ip[0]);
	W51_write(sockaddr + W5100_DIPR_OFFSET + 1, remote_ip[1]);
	W51_write(sockaddr + W5100_DIPR_OFFSET + 2, remote_ip[2]);
	W51_write(sockaddr + W5100_DIPR_OFFSET + 3, remote_ip[3]);

	W51_write(sockaddr + W5100_DPORT_OFFSET, ((remote_port & 0xFF00) >> 8));
	W51_write(sockaddr + W5100_DPORT_OFFSET + 1, (remote_port & 0x00FF));

	// Make sure the TX Free Size Register is available
	txsize = W51_read(sockaddr + W5100_TX_FSR_OFFSET);	// make sure the TX free-size reg is available
	txsize = (((txsize & 0x00FF) << 8) + W51_read(sockaddr + W5100_TX_FSR_OFFSET + 1));

	timeout = 0;
	while (txsize < buflen)
	{
		_delay_ms(1);

		txsize = W51_read(sockaddr + W5100_TX_FSR_OFFSET);	// make sure the TX free-size reg is available
		txsize = (((txsize & 0x00FF) << 8) + W51_read(sockaddr + W5100_TX_FSR_OFFSET + 1));

		if (timeout++ > 1000)	// if max delay has passed...
		{
			//Close(sock);	// can't connect, close it down
			return W5100_FAIL;	// show failure
		}
	}

	// Read the Tx Write Pointer
	ptr = W51_read(sockaddr + W5100_TX_WR_OFFSET);
	offaddr = (((ptr & 0x00FF) << 8) + W51_read(sockaddr + W5100_TX_WR_OFFSET + 1));

	while (buflen)
	{
		buflen--;
		realaddr = (W5100_TXBUFADDR + (0x0800 * sock)) + (offaddr & W5100_TX_BUF_MASK);	// calc W5100 physical buffer addr for this socket

		W51_write(realaddr, *buf);	// send a byte of application data to TX buffer
		offaddr++;	// next TX buffer addr
		buf++;		// next input buffer addr
		_delay_ms(2);
	}

	W51_write(sockaddr + W5100_TX_WR_OFFSET, (offaddr & 0xFF00) >> 8);	// send MSB of new write-pointer addr
	W51_write(sockaddr + W5100_TX_WR_OFFSET + 1, (offaddr & 0x00FF));	// send LSB

	W51_write(sockaddr + W5100_CR_OFFSET, W5100_SKT_CR_SEND);	// start the send on its way
	_delay_ms(2);
	while (W51_read(sockaddr + W5100_CR_OFFSET)) ;	// block until socket starts the send

	return W5100_OK;
}


uint16_t ReceiveBasic(uint8_t sock, uint8_t *buf, uint16_t buflen)
{
	uint16_t ptr;
	uint16_t offaddr;
	uint16_t realaddr;
	uint16_t sockaddr;

	if (buflen == 0 || sock >= W5100_NUM_SOCKETS)
		return W5100_FAIL;	// ignore illegal conditions

	if (buflen > (MAX_BUF - 2))
		buflen = MAX_BUF - 2;	// requests that exceed the max are truncated

	sockaddr = W5100_SKT_BASE(sock);	// calc base addr for this socket
	ptr = W51_read(sockaddr + W5100_RX_RD_OFFSET);	// get the RX read pointer (MSB)
	offaddr = (((ptr & 0x00FF) << 8) + W51_read(sockaddr + W5100_RX_RD_OFFSET + 1));	// get LSB and calc offset addr

	while (buflen)
	{
		buflen--;
		realaddr = (W5100_RXBUFADDR + (0x0800 * sock)) + (offaddr & W5100_RX_BUF_MASK);
		*buf = W51_read(realaddr);
		offaddr++;
		buf++;
	}
	*buf = '\0';		// buffer read is complete, terminate the str

	// Increase the S0_RX_RD value, so it point to the next receive
	W51_write(sockaddr + W5100_RX_RD_OFFSET, (offaddr & 0xFF00) >> 8);	// update RX read offset (MSB)
	W51_write(sockaddr + W5100_RX_RD_OFFSET + 1, (offaddr & 0x00FF));	// update LSB

	// Now Send the RECV command
	W51_write(sockaddr + W5100_CR_OFFSET, W5100_SKT_CR_RECV);	// issue the receive command
	_delay_us(5);		// wait for receive to start

	return W5100_OK;
}


uint16_t Receive(uint8_t sock, uint8_t *buf, uint16_t buflen, uint8_t blocking_flag)
{
	uint16_t sockaddr;
	int16_t rsize = 0;

	if (buflen == 0 || sock >= W5100_NUM_SOCKETS)
		return W5100_FAIL;	// ignore illegal requests
	
	sockaddr = W5100_SKT_BASE(sock);
	
	if (W51_read(sockaddr + W5100_SR_OFFSET) != W5100_SKT_SR_ESTABLISHED)
		return W5100_FAIL;
	
	if (blocking_flag == BLOCKING)
	{
		do {
			rsize = ReceivedSize(sock);
			_delay_us(10);
		} while (rsize <= 0);
	}
	else
	{
		rsize = ReceivedSize(sock);
	}
	
	if (rsize > 0)
	{
		if (ReceiveBasic(sock, buf, rsize) == W5100_OK)
			return rsize;
		else
			return W5100_FAIL;
	}	
	
	return rsize;
}


uint16_t ReceiveFromBasic(uint8_t sock, uint8_t *buf, uint16_t buflen, uint8_t *remote_ip, uint16_t *remote_port)
{
	uint16_t ptr;
	uint16_t offaddr;
	uint16_t realaddr;
	uint16_t sockaddr;

	if (buflen == 0 || sock >= W5100_NUM_SOCKETS)
		return W5100_FAIL;	// ignore illegal conditions

	if (buflen > (MAX_BUF - 2))
		buflen = MAX_BUF - 2;	// requests that exceed the max are truncated

	sockaddr = W5100_SKT_BASE(sock);	// calc base addr for this socket
	ptr = W51_read(sockaddr + W5100_RX_RD_OFFSET);	// get the RX read pointer (MSB)
	offaddr = (((ptr & 0x00FF) << 8) + W51_read(sockaddr + W5100_RX_RD_OFFSET + 1));	// get LSB and calc offset addr

	uint8_t *tmp = buf;

	while (buflen)
	{
		buflen--;
		realaddr = (W5100_RXBUFADDR + (0x0800 * sock)) + (offaddr & W5100_RX_BUF_MASK);
		*buf = W51_read(realaddr);
		offaddr++;
		buf++;
	}
	*buf = '\0';		// buffer read is complete, terminate the str

	remote_ip[0] = tmp[0];
	remote_ip[1] = tmp[1];
	remote_ip[2] = tmp[2];
	remote_ip[3] = tmp[3];

	*remote_port = (tmp[4] << 8) | tmp[5];
	
	// Increase the S0_RX_RD value, so it point to the next receive
	W51_write(sockaddr + W5100_RX_RD_OFFSET, (offaddr & 0xFF00) >> 8);	// update RX read offset (MSB)
	W51_write(sockaddr + W5100_RX_RD_OFFSET + 1, (offaddr & 0x00FF));	// update LSB

	// Now Send the RECV command
	W51_write(sockaddr + W5100_CR_OFFSET, W5100_SKT_CR_RECV);	// issue the receive command
	_delay_us(5);		// wait for receive to start

	return W5100_OK;		
	
}

uint16_t ReceiveFrom(uint8_t sock, uint8_t *buf, uint16_t buflen, uint8_t *remote_ip, uint16_t *remote_port, uint8_t blocking_flag)
{
	char str[32];
	uint16_t sockaddr;
	uint16_t rsize = 0;
	
	sockaddr = W5100_SKT_BASE(sock);
	
	while(1)
	{
		switch (W51_read(sockaddr + W5100_SR_OFFSET))
		{
			case W5100_SKT_SR_CLOSED:
			break;
		
			case W5100_SKT_SR_UDP:
			rsize = ReceivedSize(sock);
			if (rsize > 0)
			{
				if (ReceiveFromBasic(sock, buf, rsize, remote_ip, remote_port) == W5100_OK)
					return rsize;
				else
					return W5100_FAIL;
			}
			else
			{
				_delay_ms(10);
			}
			
			if (blocking_flag == NON_BLOCKING)
				return rsize;
			
			break;
			
			case W5100_SKT_SR_FIN_WAIT:
			case W5100_SKT_SR_CLOSING:
			case W5100_SKT_SR_TIME_WAIT:
			case W5100_SKT_SR_CLOSE_WAIT:
			case W5100_SKT_SR_LAST_ACK:
				Close(sock);
			break;
		}
	}	
}

uint16_t ReceivedSize(uint8_t sock)
{
	uint16_t val;
	uint16_t sockaddr;

	if (sock >= W5100_NUM_SOCKETS)
		return 0;
	sockaddr = W5100_SKT_BASE(sock);	// calc base addr for this socket
	val = W51_read(sockaddr + W5100_RX_RSR_OFFSET) & 0xff;
	val = (val << 8) + W51_read(sockaddr + W5100_RX_RSR_OFFSET + 1);
	return val;
}

void printStatus(uint8_t sock)
{
	uint16_t sockaddr = W5100_SKT_BASE(sock);
	switch(W51_read(sockaddr + W5100_SR_OFFSET))
	{
		case W5100_SKT_SR_CLOSED:
			usartPutString("W5100_SKT_SR_CLOSED\r\n");
		break;
		case W5100_SKT_SR_ESTABLISHED:
			usartPutString("W5100_SKT_SR_ESTABLISHED\r\n");
		break;
		case W5100_SKT_SR_UDP:
			usartPutString("W5100_SKT_SR_UDP\r\n");
		break;
		case W5100_SKT_SR_FIN_WAIT:
			usartPutString("W5100_SKT_SR_FIN_WAIT\r\n");
		break;
		case W5100_SKT_SR_LISTEN:
			usartPutString("W5100_SKT_SR_LISTEN\r\n");
		break;
		case W5100_SKT_SR_CLOSING:
			usartPutString("W5100_SKT_SR_CLOSING\r\n");
		break;
		case W5100_SKT_SR_TIME_WAIT:
			usartPutString("W5100_SKT_SR_TIME_WAIT\r\n");
		break;
		case W5100_SKT_SR_CLOSE_WAIT:
			usartPutString("W5100_SKT_SR_CLOSE_WAIT\r\n");
		break;
		case W5100_SKT_SR_LAST_ACK:
			usartPutString("W5100_SKT_SR_LAST_ACK\r\n");
		break;
	}	
}


uint8_t Select(uint8_t *socket_set, uint8_t eth_protocol, uint16_t port)
{
	int16_t iter;
	uint16_t sockaddr;
	uint8_t any_ready = 0;
	uint8_t already_connected = 0;
	
	// ukoliko nijedan nije u stanju listeninga (i ima dovoljno socketa) - postavi ga u to stanje
	for (iter = 0; iter < W5100_NUM_SOCKETS; iter++)
	{
		sockaddr = W5100_SKT_BASE(iter);
		uint8_t status = W51_read(sockaddr + W5100_SR_OFFSET);
		if (status == W5100_SKT_SR_LISTEN || status == W5100_SKT_SR_UDP)
		{
			any_ready = 1;
		}
		if (status == W5100_SKT_SR_ESTABLISHED)
		{
			already_connected++;
		}
	}
	
	if (!any_ready && already_connected != W5100_NUM_SOCKETS)
	{
		uint8_t tmp_sock = Socket(eth_protocol, port);
		if (tmp_sock == W5100_FAIL)
		{
			return W5100_FAIL;
		}
		
		if (eth_protocol == W5100_SKT_MR_TCP)
		{
			if (Listen(tmp_sock) == W5100_FAIL)
				return W5100_FAIL;
		}
		
	}
	
	for (iter = 0; iter < W5100_NUM_SOCKETS; iter++)
	{
		sockaddr = W5100_SKT_BASE(iter);
		uint8_t status = W51_read(sockaddr + W5100_SR_OFFSET);
		switch(status)
		{
			case W5100_SKT_SR_CLOSED:
				socket_set[iter] = DISCONNECT_REQ;
			break;
			
			case W5100_SKT_SR_ESTABLISHED:
				if (ReceivedSize(iter) > 0)
					socket_set[iter] = DATA_RECEIVED;
				else	
					socket_set[iter] = CONNECT_REQ;
			break;
			
			case W5100_SKT_SR_LISTEN:
				socket_set[iter] = IDLE;
			break;
			
			case W5100_SKT_SR_UDP:
				if (ReceivedSize(iter) > 0)
					socket_set[iter] = DATA_RECEIVED;
				else	
					socket_set[iter] = CONNECT_REQ;
			break;
			
			case W5100_SKT_SR_FIN_WAIT:
			break;
			
			case W5100_SKT_SR_CLOSING:
			break;
			
			case W5100_SKT_SR_TIME_WAIT:
			break;
			
			case W5100_SKT_SR_CLOSE_WAIT:
				socket_set[iter] = DISCONNECT_REQ;
			break;
			
			case W5100_SKT_SR_LAST_ACK:
			break;
		}	
	}
	
	return W5100_OK;
}



/*
 *  Simple wrapper function for selecting the W5100 device.  This function
 *  allows the library code to invoke a target-specific function for enabling
 *  the W5100 chip.
 */
void my_select(void)
{
	W51_ENABLE;
}

/*
 *  Simple wrapper function for deselecting the W5100 device.  This function
 *  allows the library code to invoke a target-specific function for disabling
 *  the W5100 chip.
 */
void my_deselect(void)
{
	W51_DISABLE;
}

/*
 *  my_xchg      callback function; exchanges a byte with W5100 chip
 */
uint8_t my_xchg(uint8_t val)
{
	SPDR = val;
	while (!(SPSR & (1 << SPIF))) ;
	return SPDR;
}

/*
 *  my_reset      callback function; force a hardware reset of the W5100 device
 */
void my_reset(void)
{
	RESET_PORT |= (1 << RESET_BIT);	// pull reset line high
	RESET_DDR |= (1 << RESET_BIT);	// now make it an output
	RESET_PORT &= ~(1 << RESET_BIT);	// pull the line low
	_delay_ms(5);		// let the device reset
	RESET_PORT |= (1 << RESET_BIT);	// done with reset, pull the line high
	_delay_ms(10);		// let the chip wake up
}