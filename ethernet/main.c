#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include "w5100.h"
#include "socket.h"
#include "usart.h"

#define MAX_BUF 256

#define HTTP_PORT 80
#define TCP_PORT  1234
#define UDP_PORT  8888

uint8_t server_ip[] = {192,168,0,12};
//uint16_t sockaddr;
	
ADDRESS_CFG my_cfg = {
	{0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED},	// MAC address
	{192, 168, 0, 177},						// IP address
	{255, 255, 255, 0},						// Subnet mask
	{192, 168, 0, 1}                        // Gateway
};


uint8_t buf[MAX_BUF];

int16_t main ()
{	
	uint8_t mysocket;
	uint16_t rsize;
	char str[32];

	// init board and usart
	usartInit(9600);
	EthernetInit();
	
	
	// test more clients on tcp server via select
	uint8_t socket_set[4] = {IDLE, IDLE, IDLE, IDLE};
	while(1)
	{
		uint8_t sock = Socket(SOCK_STREAM, TCP_PORT);
		if (sock >= W5100_NUM_SOCKETS)
		{
			usartPutString("Failed to open socket\r\n");
			Close(sock);
		}
		
		Bind(&my_cfg);
		Listen(sock);
		
		while(1)
		{
			Select(socket_set, SOCK_STREAM, TCP_PORT);
			
			int16_t iter;
			for (iter = 0; iter < W5100_NUM_SOCKETS; iter++)
			{
				switch(socket_set[iter])
				{
					case DATA_RECEIVED:
						Receive(iter, buf, MAX_BUF, NON_BLOCKING);
						sprintf(buf, "hello client %d", iter);
						Send(iter, buf, strlen((char *)buf));
					break;
					
					case CONNECT_REQ:
					break;
					
					case DISCONNECT_REQ:
						Disconnect(iter);
					break;
				}
			}
		}
	}
	
	
	// test more clients on udp server via select
	/*uint8_t socket_set[4] = {IDLE, IDLE, IDLE, IDLE};
	while(1)
	{
		uint8_t remote_ip[4];
		uint16_t remote_port;
		uint8_t sock = Socket(SOCK_DGRAM, UDP_PORT);
		if (sock >= W5100_NUM_SOCKETS)
		{
			usartPutString("Failed to open socket\r\n");
			Close(sock);
		}
		Bind(&my_cfg);
		
		while(1)
		{
			Select(socket_set, SOCK_DGRAM, UDP_PORT);
			
			int16_t iter;
			for (iter = 0; iter < W5100_NUM_SOCKETS; iter++)
			{
				switch(socket_set[iter])
				{
					case DATA_RECEIVED:
					ReceiveFrom(iter, buf, MAX_BUF, remote_ip, &remote_port, NON_BLOCKING);
					//strcpy_P((char *)buf,PSTR("Hello world!.\r\n"));
					sprintf(buf, "hello client %d", iter);
					SendTo(iter, buf, strlen((char *)buf), remote_ip, remote_port);
					break;
					case CONNECT_REQ:
					break;
					case DISCONNECT_REQ:
					Close(iter);
					break;
				}
			}
		}
	}*/
	
	// Simple chat 1-1 UDP Client side
	/*while(1)
	{
		uint8_t remote_ip[4] = {192,168,0,12};
		uint16_t remote_port = 8885;
		
		usartPutString("Begin chat\r\n");
		int16_t sock = Socket(SOCK_DGRAM, UDP_PORT);
		
		if (sock >= W5100_NUM_SOCKETS)
		{
			usartPutString("Failed to open socket\r\n");
			Close(sock);
		}
		
		Bind(&my_cfg);

		while(1)
		{
			usartPutString("Input:\r\n");
			while(!usartAvailable());
			_delay_ms(100);

			usartGetString(buf);

			usartPutString("client:");
			usartPutString(buf);
			usartPutString("\r\n");
			
			if (SendTo(sock, buf, strlen((char *)buf), remote_ip, remote_port) == W5100_FAIL)
			{
				usartPutString("fail\r\n");
				Close(sock);
			}
			
			ReceiveFrom(sock, buf, MAX_BUF, remote_ip, &remote_port, BLOCKING);
			usartPutString("server:");
			usartPutString(buf + 8); // 8B header
			usartPutString("\r\n");
		}
		
		Close(sock);
		
	}*/
	
	// Simple chat 1-1 TCP Client side
	/*while(1)
	{
		usartPutString("Begin chat\r\n");
		int16_t sock = Socket(SOCK_STREAM, TCP_PORT);
		
		if (sock >= W5100_NUM_SOCKETS)
		{
			usartPutString("Failed to open socket\r\n");
			Close(sock);
		}
		
		Bind(&my_cfg);
		
		Connect(sock, server_ip, 12345);
		
		while(1)
		{
			usartPutString("Input:\r\n");
			while(!usartAvailable());
			_delay_ms(100);
	
			usartGetString(buf);

			usartPutString("client:");
			usartPutString(buf);
			usartPutString("\r\n");

			if (Send(sock, buf, strlen((char *)buf)) == W5100_FAIL)
			{
				usartPutString("fail\r\n");
				Close(sock);
			}
			
			if (Receive(sock, buf, MAX_BUF, BLOCKING) == W5100_FAIL)
				Close(sock);
			
			usartPutString("server:");
			usartPutString(buf);
			usartPutString("\r\n");
		}
		
		Disconnect(sock);
		Close(sock);
	}*/
	
	
	
	/*uint8_t socket_set[4] = {IDLE, IDLE, IDLE, IDLE};
	
	// Test Select
	while(1)
	{
		uint8_t remote_ip[4];
		uint16_t remote_port;
		uint8_t sock = Socket(SOCK_STREAM, TCP_PORT);
		if (sock >= W5100_NUM_SOCKETS)
		{
			usartPutString("Failed to open socket\r\n");
			Close(sock);
		}
		Bind(&my_cfg);
		Listen(sock);
		
		while(1)
		{
			Select(socket_set, SOCK_STREAM, TCP_PORT);
			
			#define TCP
			// obrada statusa za svakog klijenta
			// ovo sad moze na razne nacine
			// bitno proverati u kom se stanju sock nalazi
			// i na osnovu toga reagovati
			int16_t iter;
			for (iter = 0; iter < W5100_NUM_SOCKETS; iter++)
			{
				#ifdef TCP
				switch(socket_set[iter])
				{
					case DATA_RECEIVED:
					Receive(iter, buf, MAX_BUF, BLOCKING);
					strcpy_P((char *)buf,PSTR("Hello world!.\r\n"));
					Send(iter, buf, strlen((char *)buf));
					break;
					case CONNECT_REQ:
					break;
					case DISCONNECT_REQ:
					Disconnect(iter);
					break;
				}
				#else
				switch(socket_set[iter])
				{
					case DATA_RECEIVED:
					ReceiveFrom(iter, buf, MAX_BUF, remote_ip, &remote_port, BLOCKING);
					strcpy_P((char *)buf,PSTR("Hello world!\r\n"));
					SendTo(iter, buf, strlen((char *)buf), remote_ip, remote_port);
					break;
					case CONNECT_REQ:
					break;
					case DISCONNECT_REQ:
					Close(iter);
					break;
				}
				#endif
			}
		}
	}*/
	
	
	/*// Test interrupt
	while(1)
	{
		if (W51_read(sockaddr + W5100_IR_OFFSET) & W5100_IR_S0_INT)
			usartPutString("test interrupt 0\n\r");
		
		if (W51_read(W5100_IR) & W5100_IR_S0_INT)
			usartPutString("test interrupt 0 IR\n\r");
	}*/
	
	
	// SERVER TCP
	/*while (1)
	{
		uint8_t sock = Socket(SOCK_STREAM, TCP_PORT);
		
		if (sock >= W5100_NUM_SOCKETS)
		{
			usartPutString("Failed to open socket\r\n");
			Close(sock);
		}
		
		Bind(&my_cfg);
		
		Listen(sock);
	
		//Accept_old(sock, SOCK_STREAM, TCP_PORT);
		sock = Accept(TCP_PORT);
		
		uint16_t sockaddr = W5100_SKT_BASE(sock);
		usartPutString("begin");
	
		if (Receive(sock, buf, MAX_BUF, BLOCKING) == W5100_FAIL)
		{
			usartPutString("test");
			//Close(sock);
		}
		strcpy_P((char *)buf,PSTR("Hello world!\r\n"));
		if (Send(sock, buf, strlen((char *)buf)) == W5100_FAIL)
		{
			usartPutString("fail\r\n");
			//Close(sock);
		}

		/*while(1)
		{
			switch(W51_read(sockaddr + W5100_SR_OFFSET))
			{
				case W5100_SKT_SR_CLOSED:
				break;
				case W5100_SKT_SR_ESTABLISHED:

				Disconnect(sock);
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
		Disconnect(sock);
		
		while(1);
	}*/
	
	
	// TCP client
	/*while(1)
	{
		int16_t sock = Socket(SOCK_STREAM, TCP_PORT);
		
		if (sock >= W5100_NUM_SOCKETS)
		{
			usartPutString("Failed to open socket\r\n");
			Close(sock);
		}
		
		Bind(&my_cfg);
		
		Connect(sock, server_ip, 1234);
		
		if (Receive(sock, buf, MAX_BUF, BLOCKING) == W5100_FAIL)
			Close(sock);
		
		usartPutString(buf);
		
		strcpy_P((char *)buf,PSTR("Hello world!\r\n"));
		if (Send(sock, buf, strlen((char *)buf)) == W5100_FAIL)
		{
			usartPutString("fail\r\n");
			Close(sock);
		}
		
		Disconnect(sock);
		Close(sock);
		
		while(1);
	}*/
	
	
	// UDP SERVER
	/*while(1)
	{
		uint8_t remote_ip[4];
		uint16_t remote_port;
		
		int16_t sock = Socket(SOCK_DGRAM, UDP_PORT);
		
		if (sock >= W5100_NUM_SOCKETS)
		{
			usartPutString("Failed to open socket\r\n");
			Close(sock);
		}
		
		Bind(&my_cfg);
		
		ReceiveFrom(sock, buf, MAX_BUF, remote_ip, &remote_port, BLOCKING);
		
		strcpy_P((char *)buf,PSTR("Hello world!\r\n"));
		
		if (SendTo(sock, buf, strlen((char *)buf), remote_ip, remote_port) == W5100_FAIL)
		{
			usartPutString("fail\r\n");
			Close(sock);
		}
		
		Close(sock);
		while(1);
	}*/

	// UDP CLIENT
	/*while(1)
	{
		uint8_t remote_ip[4] = {192,168,0,12};
		uint16_t remote_port = 8885;
		
		int16_t sock = Socket(SOCK_DGRAM, UDP_PORT);
		
		if (sock >= W5100_NUM_SOCKETS)
		{
			usartPutString("Failed to open socket\r\n");
			Close(sock);
		}
		
		Bind(&my_cfg);
		
		strcpy_P((char *)buf,PSTR("Hello world!\r\n"));
		
		if (SendTo(sock, buf, strlen((char *)buf), remote_ip, remote_port) == W5100_FAIL)
		{
			usartPutString("fail\r\n");
			Close(sock);
		}
		
		ReceiveFrom(sock, buf, MAX_BUF, remote_ip, &remote_port, BLOCKING);
		
		Close(sock);
		while(1);
	}*/

	return 0;
}
