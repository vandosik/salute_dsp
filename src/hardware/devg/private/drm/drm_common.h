#ifndef _DRM_COMMON_H_
#define _DRM_COMMON_H_

#include <graphics/display.h>

#include <stdint.h>
#ifdef __QNXNTO__
#include <stdint.h>
#include <stdbool.h>
#else
#include <qnx4_helpers.h>
#ifndef bool
typedef unsigned char           bool;
#define true                    (1)
#define false                   (0)
#endif

#define inline

#define EPROTO                  (71)
#endif
#include <errno.h>

/* linux int types */
#if 0	// conflicts with linux headers
#ifndef s8
#define s8 int8_t
#endif
#ifndef s16
#define s16 int16_t
#endif
#ifndef s32
#define s32 int32_t
#endif
#ifndef s64
#define s64 int64_t
#endif
#ifndef u8
#define u8 uint8_t
#endif
#ifndef u16
#define u16 uint16_t
#endif
#ifndef u32
#define u32 uint32_t
#endif
#ifndef u64
#define u64 uint64_t
#endif
#endif

struct i2c_msg {
	uint16_t addr;     /* slave address                        */
	uint16_t flags;
#define I2C_M_TEN               0x0010  /* this is a ten bit chip address */
#define I2C_M_RD                0x0001  /* read data, from slave to master */
#define I2C_M_STOP              0x8000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NOSTART           0x4000  /* if I2C_FUNC_NOSTART */
#define I2C_M_REV_DIR_ADDR      0x2000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK        0x1000  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NO_RD_ACK         0x0800  /* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_RECV_LEN          0x0400  /* length will be first received byte */
	uint16_t len;              /* msg length                           */
	uint8_t *buf;              /* pointer to msg data                  */
};

struct i2c_adapter {
	int (*xfer)(struct i2c_adapter *i2c, struct i2c_msg *msgs, int num);
	void *algo_data;
	
	int timeout;
	int retries;
	disp_adapter_t *dev;
	void *dev_data;
};

int i2c_transfer(struct i2c_adapter *i2c, struct i2c_msg *msgs, int num);

#endif
