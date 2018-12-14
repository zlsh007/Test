#ifndef __DEV_IOCTROL_H__
#define __DEV_IOCTROL_H__

//#define WHITE
//#define SEC
//#define USEC

#define GPIOA0  0
#define GPIOA1  1
#define GPIOA2  2
#define GPIOA3  3
#define GPIOA4  4
#define GPIOA5  5
#define GPIOA6  6
#define GPIOA7  7
#define GPIOA8  8
#define GPIOA9  9
#define GPIOA10  10
#define GPIOA11  11
#define GPIOA12  12
#define GPIOA13  13
#define GPIOA14  14
#define GPIOA15  15
#define GPIOA16  16
#define GPIOA17  17
#define GPIOA18  18
#define GPIOA19  19
#define GPIOA20  20
#define GPIOA21  21
#define GPIOA22  22
#define GPIOA23  23
#define GPIOA24  24
#define GPIOA25  25
#define GPIOA26  26
#define GPIOA27  27
#define GPIOA28  28
#define GPIOA29  29
#define GPIOA30  30
#define GPIOA31  31
#define GPIOB0  0
#define GPIOB1  1
#define GPIOB2  2
#define GPIOB3  3
#define GPIOB4  4
#define GPIOB5  5
#define GPIOB6  6
#define GPIOB7  7
#define GPIOB8  8
#define GPIOB9  9
#define GPIOB10  10
#define GPIOB11  11
#define GPIOB12  12
#define GPIOB13  13
#define GPIOB14  14
#define GPIOB15  15
#define GPIOB16  16
#define GPIOB17  17


#ifdef  WHITE
#undef SEC
#undef USEC
//#define PORTLED1   GPIOA27
#define PORTLED2   GPIOB12
#define PORTLED3   GPIOA11
#define PORTLED4   GPIOB16
#define PORTLED5   GPIOB5
//#define PORTLED6   GPIOB12
#define PORTBEEP   GPIOA28
//#define PORTAC4RESET   GPIOB13
//#define PORTUARTMODE GPIOA10
//#define PORTKILLKEY GPIOB17
//#define PORTAC4RESET GPIOB13
#endif

#ifdef SEC
#undef WHITE
#undef USEC
#define PORTLED1   GPIOA7
#define PORTLED2   GPIOA6
#define PORTLED3   GPIOA8

#define PORTBEEP        GPIOB5
#define PORTUARTMODE    GPIOA26
#define PORTKILLKEY     GPIOB12
#define PORTAC4RESET    GPIOB13
#define PORTNAS7825IO0  GPIOA5
#define PORTNAS7825IO1  GPIOA11
#define PORTOPENLOCK    GPIOB16
#define PORTCFGDONE   GPIOB6
#endif

#ifdef USEC
#undef SEC
#undef WHITE
#define PORTLED1   GPIOA7
#define PORTLED2   GPIOA6
#define PORTLED3   GPIOA8
#define PORTNAS7825IO0  GPIOA11
#define PORTNAS7825IO1  GPIOA5
#endif 

#define GPIO_IOC_MAGIC  'i'
#define GPIO_IOC_RESET					_IOWR(GPIO_IOC_MAGIC, 0, int)
#define GPIO_IOC_SET					_IOWR(GPIO_IOC_MAGIC, 1, int)
#define GPIO_IOC_CLEAR					_IOWR(GPIO_IOC_MAGIC, 2, int)
#define GPIO_IOC_GET					_IOWR(GPIO_IOC_MAGIC,3, int)
#define GPIO_ATTR_GET_IOMODE			_IOWR(GPIO_IOC_MAGIC, 4, int)
#define GPIO_ATTR_IN					_IOWR(GPIO_IOC_MAGIC, 5, int)
#define GPIO_ATTR_OUT					_IOWR(GPIO_IOC_MAGIC, 6, int)
#define GPIO_ATTR_PULLUP				_IOWR(GPIO_IOC_MAGIC, 7, int)
#define GPIO_ATTR_PULLDOWN			_IOWR(GPIO_IOC_MAGIC, 8, int)
#define GPIO_ATTR_PULLNONE			_IOWR(GPIO_IOC_MAGIC, 9, int)
#define GPIO_ATTR_INTERRUPT			_IOWR(GPIO_IOC_MAGIC, 10, int)
#define GPIO_ATTR_RISING_EDGE_IRQ		_IOWR(GPIO_IOC_MAGIC, 11, int)
#define GPIO_ATTR_FALLING_EDGE_IRQ	_IOWR(GPIO_IOC_MAGIC, 12, int)
#define GPIO_READREG					_IOWR(GPIO_IOC_MAGIC, 13, int)
#define GPIO_WRITEREG					_IOWR(GPIO_IOC_MAGIC, 14, int)


#endif
