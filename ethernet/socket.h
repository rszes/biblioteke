#ifndef SOCKET_H
#define SOCKET_H

// hardware specs
#define SPI_PORT        PORTB	/* target-specific port containing the SPI lines */
#define SPI_DDR         DDRB	/* target-specific DDR for the SPI port lines */

#define CS_DDR          DDRB	/* target-specific DDR for chip-select */
#define CS_PORT         PORTB	/* target-specific port used as chip-select */
#define CS_BIT          2	/* target-specific port line used as chip-select */

#define RESET_DDR       DDRD	/* target-specific DDR for reset */
#define RESET_PORT      PORTD	/* target-specific port used for reset */
#define RESET_BIT       3	/* target-specific port line used as reset */

#define W51_ENABLE      CS_PORT&=~(1<<CS_BIT)
#define W51_DISABLE     CS_PORT|=(1<<CS_BIT)

typedef enum {IDLE,
			  CLOSED,
			  CONNECT_REQ,
			  DATA_RECEIVED,
			  DISCONNECT_REQ} client_actions;

// block flags
#define BLOCKING 0
#define NON_BLOCKING 1

// define protocols
#define SOCK_STREAM W5100_SKT_MR_TCP
#define SOCK_DGRAM W5100_SKT_MR_UDP

// shield init function
void EthernetInit();

// Both TCP and UDP
uint8_t Socket(uint8_t eth_protocol, uint16_t port);
uint8_t Bind(ADDRESS_CFG *bind_address);
void Close(uint8_t sock);

// TCP functions
// helper functions
uint8_t ConnectBasic(uint8_t sock, uint8_t *remote_ip,uint16_t remote_port);
uint8_t SendBasic(uint8_t  sock, const uint8_t  *buf, uint16_t  buflen);
uint16_t ReceiveBasic(uint8_t  sock, uint8_t  *buf, uint16_t  buflen);
// user functions
void Disconnect(uint8_t sock);
uint8_t Listen(uint8_t sock);
uint8_t Accept(uint16_t port);
uint8_t Connect(uint8_t sock, uint8_t *remote_ip,uint16_t remote_port);
uint8_t Send(uint8_t  sock, const uint8_t  *buf, uint16_t  buflen);
uint16_t Receive(uint8_t  sock, uint8_t  *buf, uint16_t  buflen, uint8_t blocking_flag);

// depracated
uint8_t Accept_old(uint8_t sock, uint8_t eth_protocol, uint16_t port);

//UDP functions
// helper functions
uint8_t SendToBasic(uint8_t sock, const uint8_t *buf, uint16_t buflen, uint8_t *remote_ip, uint16_t remote_port);
uint16_t ReceiveFromBasic(uint8_t sock, uint8_t *buf, uint16_t buflen, uint8_t *remote_ip, uint16_t *remote_port);
// user functions
uint8_t SendTo(uint8_t sock, const uint8_t *buf, uint16_t buflen, uint8_t *remote_ip, uint16_t remote_port);
uint16_t ReceiveFrom(uint8_t sock, uint8_t *buf, uint16_t buflen, uint8_t *remote_ip, uint16_t *remote_port, uint8_t blocking_flag);

// for multiclient server communication
uint8_t Select(uint8_t *socket_set, uint8_t eth_protocol, uint16_t port);

// utility functions
uint16_t ReceivedSize(uint8_t sock);
void printStatus(uint8_t sock);

void my_select(void);
void my_deselect(void);
uint8_t my_xchg(uint8_t  val);
void my_reset(void);

#endif //SOCKET_H
