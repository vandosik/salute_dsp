#include <graphics/display.h>
#include <graphics/disputil.h>
#include <errno.h>

#include "drm/drm_common.h"
#include <linux/i2c-algo-bit.h>

#define udelay(us) disp_usecspin(us)

#define I2C_SMBUS_BLOCK_MAX     32

/* ----- global defines ----------------------------------------------- */

#ifdef DEBUG
#define bit_dbg(level, dev, format, args...) \
	do { \
		if (i2c_debug >= level) \
			disp_printf(dev, format, ##args); \
	} while (0)
#else
#ifndef __WATCOMC__
#define bit_dbg(level, dev, format, args...) \
	do {} while (0)
#else
#define bit_dbg
#endif
#endif /* DEBUG */

/* --- setting states on the bus with the right timing: ---------------	*/

#define setsda(adap, val)	adap->setsda(adap->data, val)
#define setscl(adap, val)	adap->setscl(adap->data, val)
#define getsda(adap)		adap->getsda(adap->data)
#define getscl(adap)		adap->getscl(adap->data)

static inline void sdalo(struct i2c_algo_bit_data *adap)
{
	setsda(adap, 0);
	udelay((adap->udelay + 1) / 2);
}

static inline void sdahi(struct i2c_algo_bit_data *adap)
{
	setsda(adap, 1);
	udelay((adap->udelay + 1) / 2);
}

static inline void scllo(struct i2c_algo_bit_data *adap)
{
	setscl(adap, 0);
	udelay(adap->udelay / 2);
}

/*
 * Raise scl line, and do checking for delays. This is necessary for slower
 * devices.
 */
static int sclhi(struct i2c_algo_bit_data *adap)
{
	//unsigned long start;
	unsigned long timeout;

	setscl(adap, 1);

	/* Not all adapters have scl sense line... */
	if (!adap->getscl)
		goto done;

	//start = jiffies;
	timeout = adap->timeout;
	while (!getscl(adap)) {
		/* This hw knows how to read the clock line, so we wait
		 * until it actually gets high.  This is safer as some
		 * chips may hold it low ("clock stretching") while they
		 * are processing data internally.
		 */\
#if 0
		if (time_after(jiffies, start + adap->timeout)) {
			/* Test one last time, as we may have been preempted
			 * between last check and timeout test.
			 */
			if (getscl(adap))
				break;
			return -ETIMEDOUT;
		}
		cpu_relax();
#else
		if (timeout == 0)
		{
			if (getscl(adap))
				break;
			return -ETIMEDOUT;
		}
		udelay(1);
		timeout --;
#endif
	}
#ifdef DEBUG
	/*if (jiffies != start && i2c_debug >= 3)
		pr_debug("i2c-algo-bit: needed %ld jiffies for SCL to go "
			 "high", jiffies - start);*/
#endif

done:
	udelay(adap->udelay);
	return 0;
}


/* --- other auxiliary functions --------------------------------------	*/
static void i2c_start(struct i2c_algo_bit_data *adap)
{
	/* assert: scl, sda are high */
	setsda(adap, 0);
	udelay(adap->udelay);
	scllo(adap);
}

static void i2c_repstart(struct i2c_algo_bit_data *adap)
{
	/* assert: scl is low */
	sdahi(adap);
	sclhi(adap);
	setsda(adap, 0);
	udelay(adap->udelay);
	scllo(adap);
}


static void i2c_stop(struct i2c_algo_bit_data *adap)
{
	/* assert: scl is low */
	sdalo(adap);
	sclhi(adap);
	setsda(adap, 1);
	udelay(adap->udelay);
}

/* send a byte without start cond., look for arbitration,
   check ackn. from slave */
/* returns:
 * 1 if the device acknowledged
 * 0 if the device did not ack
 * -ETIMEDOUT if an error occurred (while raising the scl line)
 */
static int i2c_outb(struct i2c_adapter *i2c_adap, unsigned char c)
{
	int i;
	int sb;
	int ack;
	struct i2c_algo_bit_data *adap = i2c_adap->algo_data;

	/* assert: scl is low */
	for (i = 7; i >= 0; i--) {
		sb = (c >> i) & 1;
		setsda(adap, sb);
		udelay((adap->udelay + 1) / 2);
		if (sclhi(adap) < 0) { /* timed out */
			bit_dbg(1, &i2c_adap->dev, "i2c_outb: 0x%02x, "
				"timeout at bit #%d", (int)c, i);
			return -ETIMEDOUT;
		}
		/* FIXME do arbitration here:
		 * if (sb && !getsda(adap)) -> ouch! Get out of here.
		 *
		 * Report a unique code, so higher level code can retry
		 * the whole (combined) message and *NOT* issue STOP.
		 */
		scllo(adap);
	}
	sdahi(adap);
	if (sclhi(adap) < 0) { /* timeout */
		bit_dbg(1, &i2c_adap->dev, "i2c_outb: 0x%02x, "
			"timeout at ack", (int)c);
		return -ETIMEDOUT;
	}

	/* read ack: SDA should be pulled down by slave, or it may
	 * NAK (usually to report problems with the data we wrote).
	 */
	ack = !getsda(adap);    /* ack: sda is pulled low -> success */
	bit_dbg(2, &i2c_adap->dev, "i2c_outb: 0x%02x %s", (int)c,
		ack ? "A" : "NA");

	scllo(adap);
	return ack;
	/* assert: scl is low (sda undef) */
}


static int i2c_inb(struct i2c_adapter *i2c_adap)
{
	/* read byte via i2c port, without start/stop sequence	*/
	/* acknowledge is sent in i2c_read.			*/
	int i;
	unsigned char indata = 0;
	struct i2c_algo_bit_data *adap = i2c_adap->algo_data;

	/* assert: scl is low */
	sdahi(adap);
	for (i = 0; i < 8; i++) {
		if (sclhi(adap) < 0) { /* timeout */
			bit_dbg(1, &i2c_adap->dev, "i2c_inb: timeout at bit "
				"#%d", 7 - i);
			return -ETIMEDOUT;
		}
		indata *= 2;
		if (getsda(adap))
			indata |= 0x01;
		setscl(adap, 0);
		udelay(i == 7 ? adap->udelay / 2 : adap->udelay);
	}
	/* assert: scl is low */
	return indata;
}

/* ----- Utility functions
 */

/* try_address tries to contact a chip for a number of
 * times before it gives up.
 * return values:
 * 1 chip answered
 * 0 chip did not answer
 * -x transmission error
 */
static int try_address(struct i2c_adapter *i2c_adap,
		       unsigned char addr, int retries)
{
	struct i2c_algo_bit_data *adap = i2c_adap->algo_data;
	int i, ret = 0;

	for (i = 0; i <= retries; i++) {
		ret = i2c_outb(i2c_adap, addr);
		if (ret == 1 || i == retries)
			break;
		bit_dbg(3, &i2c_adap->dev, "emitting stop condition");
		i2c_stop(adap);
		udelay(adap->udelay);
		sched_yield();
		bit_dbg(3, &i2c_adap->dev, "emitting start condition");
		i2c_start(adap);
	}
	if (i && ret)
		bit_dbg(1, &i2c_adap->dev, "Used %d tries to %s client at "
			"0x%02x: %s", i + 1,
			addr & 1 ? "read from" : "write to", addr >> 1,
			ret == 1 ? "success" : "failed, timeout?");
	return ret;
}

static int sendbytes(struct i2c_adapter *i2c_adap, struct i2c_msg *msg)
{
	const unsigned char *temp = msg->buf;
	int count = msg->len;
	unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;
	int retval;
	int wrcount = 0;

	while (count > 0) {
		retval = i2c_outb(i2c_adap, *temp);

		/* OK/ACK; or ignored NAK */
		if ((retval > 0) || (nak_ok && (retval == 0))) {
			count--;
			temp++;
			wrcount++;

		/* A slave NAKing the master means the slave didn't like
		 * something about the data it saw.  For example, maybe
		 * the SMBus PEC was wrong.
		 */
		} else if (retval == 0) {
			disp_printf(i2c_adap->dev, "sendbytes: NAK bailout.");
			return -EIO;

		/* Timeout; or (someday) lost arbitration
		 *
		 * FIXME Lost ARB implies retrying the transaction from
		 * the first message, after the "winning" master issues
		 * its STOP.  As a rule, upper layer code has no reason
		 * to know or care about this ... it is *NOT* an error.
		 */
		} else {
			disp_printf(i2c_adap->dev, "sendbytes: error %d",
					retval);
			return retval;
		}
	}
	return wrcount;
}

static int acknak(struct i2c_adapter *i2c_adap, int is_ack)
{
	struct i2c_algo_bit_data *adap = i2c_adap->algo_data;

	/* assert: sda is high */
	if (is_ack)		/* send ack */
		setsda(adap, 0);
	udelay((adap->udelay + 1) / 2);
	if (sclhi(adap) < 0) {	/* timeout */
		disp_printf(i2c_adap->dev, "readbytes: ack/nak timeout");
		return -ETIMEDOUT;
	}
	scllo(adap);
	return 0;
}

static int readbytes(struct i2c_adapter *i2c_adap, struct i2c_msg *msg)
{
	int inval;
	int rdcount = 0;	/* counts bytes read */
	unsigned char *temp = msg->buf;
	int count = msg->len;
	const unsigned flags = msg->flags;

	while (count > 0) {
		inval = i2c_inb(i2c_adap);
		if (inval >= 0) {
			*temp = inval;
			rdcount++;
		} else {   /* read timed out */
			break;
		}

		temp++;
		count--;

		/* Some SMBus transactions require that we receive the
		   transaction length as the first read byte. */
		if (rdcount == 1 && (flags & I2C_M_RECV_LEN)) {
			if (inval <= 0 || inval > I2C_SMBUS_BLOCK_MAX) {
				if (!(flags & I2C_M_NO_RD_ACK))
					acknak(i2c_adap, 0);
				disp_printf(i2c_adap->dev, "readbytes: invalid "
					"block length (%d)", inval);
				return -EPROTO;
			}
			/* The original count value accounts for the extra
			   bytes, that is, either 1 for a regular transaction,
			   or 2 for a PEC transaction. */
			count += inval;
			msg->len += inval;
		}

		bit_dbg(2, &i2c_adap->dev, "readbytes: 0x%02x %s",
			inval,
			(flags & I2C_M_NO_RD_ACK)
				? "(no ack/nak)"
				: (count ? "A" : "NA"));

		if (!(flags & I2C_M_NO_RD_ACK)) {
			inval = acknak(i2c_adap, count);
			if (inval < 0)
				return inval;
		}
	}
	return rdcount;
}

/* doAddress initiates the transfer by generating the start condition (in
 * try_address) and transmits the address in the necessary format to handle
 * reads, writes as well as 10bit-addresses.
 * returns:
 *  0 everything went okay, the chip ack'ed, or IGNORE_NAK flag was set
 * -x an error occurred (like: -ENXIO if the device did not answer, or
 *	-ETIMEDOUT, for example if the lines are stuck...)
 */
static int bit_doAddress(struct i2c_adapter *i2c_adap, struct i2c_msg *msg)
{
	unsigned short flags = msg->flags;
	unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;
	struct i2c_algo_bit_data *adap = i2c_adap->algo_data;

	unsigned char addr;
	int ret, retries;

	retries = nak_ok ? 0 : i2c_adap->retries;

	if (flags & I2C_M_TEN) {
		/* a ten bit address */
		addr = 0xf0 | ((msg->addr >> 7) & 0x06);
		bit_dbg(2, &i2c_adap->dev, "addr0: %d", addr);
		/* try extended address code...*/
		ret = try_address(i2c_adap, addr, retries);
		if ((ret != 1) && !nak_ok)  {
			disp_printf(i2c_adap->dev,
				"died at extended address code");
			return -ENXIO;
		}
		/* the remaining 8 bit address */
		ret = i2c_outb(i2c_adap, msg->addr & 0xff);
		if ((ret != 1) && !nak_ok) {
			/* the chip did not ack / xmission error occurred */
			disp_printf(i2c_adap->dev, "died at 2nd address code");
			return -ENXIO;
		}
		if (flags & I2C_M_RD) {
			bit_dbg(3, &i2c_adap->dev, "emitting repeated "
				"start condition");
			i2c_repstart(adap);
			/* okay, now switch into reading mode */
			addr |= 0x01;
			ret = try_address(i2c_adap, addr, retries);
			if ((ret != 1) && !nak_ok) {
				disp_printf(i2c_adap->dev,
					"died at repeated address code");
				return -EIO;
			}
		}
	} else {		/* normal 7bit address	*/
		addr = msg->addr << 1;
		if (flags & I2C_M_RD)
			addr |= 1;
		if (flags & I2C_M_REV_DIR_ADDR)
			addr ^= 1;
		ret = try_address(i2c_adap, addr, retries);
		if ((ret != 1) && !nak_ok)
			return -ENXIO;
	}

	return 0;
}

int i2c_bit_xfer(struct i2c_adapter *i2c_adap,
		    struct i2c_msg msgs[], int num)
{
	struct i2c_msg *pmsg;
	struct i2c_algo_bit_data *adap = i2c_adap->algo_data;
	int i, ret;
	unsigned short nak_ok;

	if (adap->pre_xfer) {
		ret = adap->pre_xfer(i2c_adap);
		if (ret < 0)
			return ret;
	}

	bit_dbg(3, &i2c_adap->dev, "emitting start condition");
	i2c_start(adap);
	for (i = 0; i < num; i++) {
		pmsg = &msgs[i];
		nak_ok = pmsg->flags & I2C_M_IGNORE_NAK;
		if (!(pmsg->flags & I2C_M_NOSTART)) {
			if (i) {
				bit_dbg(3, &i2c_adap->dev, "emitting "
					"repeated start condition");
				i2c_repstart(adap);
			}
			ret = bit_doAddress(i2c_adap, pmsg);
			if ((ret != 0) && !nak_ok) {
				bit_dbg(1, &i2c_adap->dev, "NAK from "
					"device addr 0x%02x msg #%d",
					msgs[i].addr, i);
				goto bailout;
			}
		}
		if (pmsg->flags & I2C_M_RD) {
			/* read bytes into buffer*/
			ret = readbytes(i2c_adap, pmsg);
			if (ret >= 1)
				bit_dbg(2, &i2c_adap->dev, "read %d byte%s",
					ret, ret == 1 ? "" : "s");
			if (ret < pmsg->len) {
				if (ret >= 0)
					ret = -EIO;
				goto bailout;
			}
		} else {
			/* write bytes from buffer */
			ret = sendbytes(i2c_adap, pmsg);
			if (ret >= 1)
				bit_dbg(2, &i2c_adap->dev, "wrote %d byte%s",
					ret, ret == 1 ? "" : "s");
			if (ret < pmsg->len) {
				if (ret >= 0)
					ret = -EIO;
				goto bailout;
			}
		}
	}
	ret = i;

bailout:
	bit_dbg(3, &i2c_adap->dev, "emitting stop condition");
	i2c_stop(adap);

	if (adap->post_xfer)
		adap->post_xfer(i2c_adap);
	return ret;
}
