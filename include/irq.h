#pragma once

#include <types.h>

#define IRQ_NONE       0
#define IRQ_HANDLED    1

#define IRQ_TYPE_NONE         0x00000000
#define IRQ_TYPE_RISING       0x00000001
#define IRQ_TYPE_FALLING      0x00000002
#define IRQ_TYPE_BOTH         (IRQ_TYPE_FALLING | IRQ_TYPE_RISING)
#define IRQ_TYPE_HIGH         0x00000004
#define IRQ_TYPE_LOW          0x00000008


struct int_pin;
struct irq_dev;

typedef	void (*IRQ_PIN_HANDLER)(struct int_pin *, u32);
typedef int  (*IRQ_DEV_HANDLER)(u32, void *);


struct int_ctrl
{
	void (*ack)(u32);
	void (*mask)(u32);
	void (*mack)(u32);
	void (*umask)(u32);
	int	 (*set_trigger)(u32, u32);
};


struct int_pin
{
	IRQ_PIN_HANDLER	 irq_handle;
	struct int_ctrl	 *intctrl;
	struct irq_dev	 *dev_list;
};


struct irq_dev
{
	void             *device;
	IRQ_DEV_HANDLER  dev_isr;
	struct irq_dev   *next;
};


void irq_handle(u32 irq);

void irq_handle_level(struct int_pin *ipin, u32 irq);

void irq_handle_edge(struct int_pin *ipin, u32 irq);

void irq_handle_simple(struct int_pin *ipin, u32 irq);

void irq_set_handler(u32 irq, IRQ_PIN_HANDLER irq_handle, int isChained);

int  irq_set_trigger(u32 irq, u32 type);

int  irq_register_isr(u32 irq, IRQ_DEV_HANDLER dev_isr, void *dev);

int irq_assoc_intctl(u32 irq, struct int_ctrl *intctrl);

