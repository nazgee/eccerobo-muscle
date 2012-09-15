/*
 * types.h
 *
 *  Created on: Sep 15, 2012
 *      Author: nazgee
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <stddef.h>

/**
 * This is required by some of the list macros. Does nothing on AVR
 */
#define prefetch(x) __builtin_prefetch(x)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})


#define LIST_POISON1  ((void *) 0xFFFFDEAD)
#define LIST_POISON2  ((void *) 0xFFFFDEAD)

struct list_head {
	struct list_head *next, *prev;
};

//#define ENTER_CRITICAL_SECTION( )   do { cli(); LED_Off(LED_IRQ_ENABLED); _delay_us(100);} while(0)
//#define EXIT_CRITICAL_SECTION( )    do { LED_On(LED_IRQ_ENABLED); sei(); _delay_us(100);} while(0)
#define ENTER_CRITICAL_SECTION( )   do { cli(); } while(0)
#define EXIT_CRITICAL_SECTION( )    do { sei(); } while(0)

#endif /* TYPES_H_ */
