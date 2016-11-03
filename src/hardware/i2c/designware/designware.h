/* designware.h
 *
 *  Created on: 14.10.2016
 *      Author: SWD Embedded Systems Ltd. */


#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/neutrino.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <hw/inout.h>
#include <hw/i2c.h>
#include <stdarg.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>


#include "designware-registers.h"


#ifndef _DESIGNWARE_H_
#define _DESIGNWARE_H_


/* EEPROM chips that implement "address overflow" are ones like Catalyst 24WC04/08/16 which has 9/10/11 bits of address and the extra bits
 * end up in the "chip address" bit slots. This makes a 24WC08 (1Kbyte) chip look like four 256 byte chips.
 *
 * Note that we consider the length of the address field to still be one byte because the extra address bits are hidden in the chip address. */
//#define I2C_EEPROM_ADDR_OVERFLOW                ??

#define DESIGNWARE_I2C_REGLEN                   0x1000

#define IC_CLK                                  144000 /* KHz */

#define NANO_TO_MICRO                           1000L
#define NANO_TO_MILLI                           1000000L
#define NANO_TO_SEC                             1000000000L

/* Worst case timeouts in ms */
#define I2C_BYTE_TO                             (16)
#define I2C_STOPDET_TO                          (16)
#define I2C_BYTE_TO_BB                          (64)

/* Registers access flag */
#define ACCESS_SWAP                             0x00000001
#define ACCESS_16BIT                            0x00000002  /* ignored */


typedef struct designware_i2c_dev
{
    /* Device */
    uint32_t            frequency;

    /* Registers */
    designware_i2c_regs_t   *registers;
    uint32_t            physbase;
    uint32_t            accessor_flags;                     /* Registers access flag */

    /* FIFO */
    uint32_t            tx_fifo_depth;
    uint32_t            rx_fifo_depth;

    uint32_t            speed;
    i2c_addr_t          slave_addr;

    pthread_mutex_t     lock;
} designware_i2c_dev_t;


/* i2c-designware.c */
int i2c_printf( int, const char *, ... );

/* designware.c */
uint32_t designware_i2c_write( designware_i2c_dev_t *dev, uint8_t slave_address, uint32_t offset, int alen, uint8_t *buffer, int len );
uint32_t designware_i2c_read( designware_i2c_dev_t *dev, uint8_t slave_address, uint32_t offset, int alen, uint8_t *buffer, int len );
uint32_t designware_i2c_set_bus_speed( designware_i2c_dev_t *dev, i2c_speed_mode_t speed );
void designware_i2c_reset( designware_i2c_dev_t *dev );
void designware_i2c_init( designware_i2c_dev_t *dev, i2c_speed_mode_t speed, uint32_t slaveaddr );
int synopsys_i2c_init( designware_i2c_dev_t *dev );


#endif /* _DESIGNWARE_H_ */
