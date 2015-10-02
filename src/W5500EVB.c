/*
===============================================================================
 Name        : W5500EVB.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <string.h>
#include "spiHandler.h"
#include "w5500_init.h"
#include "common.h"
#include "loopback.h"

#include <cr_section_macros.h>

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
///////////////////////////////////////
// Debugging Message Printout enable //
///////////////////////////////////////
#define _MAIN_DEBUG_

///////////////////////////
// Demo Firmware Version //
///////////////////////////
#define VER_H		1
#define VER_L		01

//////////////////////////////////////////////////
// Socket & Port number definition for Examples //
//////////////////////////////////////////////////
#define SOCK_TCPS       0
#define SOCK_UDPS       1
#define PORT_TCPS		5000
#define PORT_UDPS       3000

////////////////////////////////////////////////
// Shared Buffer Definition for LOOPBACK TEST //
////////////////////////////////////////////////

uint8_t gDATABUF[DATA_BUF_SIZE];

///////////////////////////
// Network Configuration //
///////////////////////////
wiz_NetInfo gWIZNETINFO = { .mac = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xff},
                            .ip = {192, 168, 0, 81},
                            .sn = {255, 255, 255, 0},
                            .gw = {192, 168, 0, 1},
                            .dns = {8, 8, 8, 8},
                            .dhcp = NETINFO_STATIC };

// For TCP client loopback examples; destination network info
uint8_t destip[4] = {192, 168, 0, 2};
uint16_t destport = 5001;

#if LOOPBACK_MODE == LOOPBACK_MAIN_NOBLOCK
#if defined(TEST_TCPS)
extern int32_t loopback_tcps_bsd(uint8_t sn, uint8_t* buf, uint16_t port);
#elif defined(TEST_UDPS)
extern int32_t loopback_udps_bsd(uint8_t sn, uint8_t* buf, uint16_t port);
#elif defined(TEST_TCPC)
extern int32_t loopback_tcpc_bsd(uint8_t sn, uint8_t* buf, struct sockaddr *sa);
#endif
#elif LOOPBACK_MODE == LOOPBACK_MAIN_SAMPLE
#if defined(TEST_TCPS)
extern int32_t loopback_tcps_bsd(uint8_t* buf);
#elif defined(TEST_TCPC)
extern int32_t loopback_tcpc_bsd(uint8_t* buf);
#endif
#endif

int main(void) {
#if LOOPBACK_MODE == LOOPBACK_MAIN_NOBLOCK
    int ret;
    uint8_t sn;
#if defined(TEST_TCPC)
    int32_t loopback_ret;
    struct sockaddr_in sa;
    uint32_t s_addr;
#endif
#endif

#if defined (__USE_LPCOPEN)
#if !defined(NO_BOARD_LIB)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "On"
    Board_LED_Set(0, true);
    Board_LED_Set(1, true);
    Board_LED_Set(2, true);
#endif
#endif

#if 0
    // TODO: insert code here

    // Force the counter to be placed into memory
    volatile static int i = 0 ;
    // Enter an infinite loop, just incrementing a counter
    while(1) {
        i++ ;
    }
#else
	SPI_Init();
	W5500_Init();
	Net_Conf(gWIZNETINFO);

#ifdef _MAIN_DEBUG_
	uint8_t tmpstr[6] = {0,};

	ctlwizchip(CW_GET_ID,(void*)tmpstr);

    printf("\r\n=======================================\r\n");
	printf(" WIZnet %s EVB Demos v%d.%.2d\r\n", tmpstr, VER_H, VER_L);
	printf("=======================================\r\n");
	printf(">> W5500 based Loopback Example\r\n");
	printf("=======================================\r\n");

	Display_Net_Conf(); // Print out the network information to serial terminal
#endif

#if LOOPBACK_MODE == LOOPBACK_MAIN_NOBLOCK
#if defined(TEST_TCPS) || defined(TEST_TCPC)
	ret = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#elif defined(TEST_UDPS)
    ret = socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);
#endif
#if defined(TEST_TCPC)
    memset(&sa, 0, sizeof(sa));
    sa.sin_len = (uint16_t)sizeof(sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(destport);
    s_addr = (((((destip[0] << 8) | destip[1]) << 8) | destip[2]) << 8) | destip[3];
    sa.sin_addr.s_addr = htonl(s_addr);
#endif

    if(ret == -1)
    {
#ifdef _LOOPBACK_DEBUG_
        printf("%d:Socket Error\r\n", ret);
#endif
        return ret;
    }
    sn = ret;
#endif

	while(1)
	{
		/* Loopback Test: TCP Server and UDP */
		// Test for Ethernet data transfer validation
#if LOOPBACK_MODE == LOOPBACK_MAIN_NOBLOCK
#if defined(TEST_TCPS)
			loopback_tcps_bsd(sn, gDATABUF, TEST_PORT);
#elif defined(TEST_UDPS)
			loopback_udps_bsd(sn, gDATABUF, TEST_PORT);
#elif defined(TEST_TCPC)
			loopback_ret = loopback_tcpc_bsd(sn, gDATABUF, (struct sockaddr*)&sa);
			if(loopback_ret < 0) printf("loopback ret: %ld\r\n", loopback_ret); // TCP Socket Error code
#endif
#elif LOOPBACK_MODE == LOOPBACK_MAIN_SAMPLE
#if defined(TEST_TCPS)
			loopback_tcps_bsd(gDATABUF);
#elif defined(TEST_TCPC)
			loopback_tcpc_bsd(gDATABUF);
#endif
#endif
	}
#endif

    return 0 ;
}
