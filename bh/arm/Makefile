
#OBJ_S = head.o
obj-y = head.o

obj-y += arm_heap.o exception.o

obj-$(CONFIG_IRQ_SUPPORT) += udelay.o

dir-$(CONFIG_S3C2410) = s3c24x0
dir-$(CONFIG_S3C2440) = s3c24x0
dir-$(CONFIG_S3C6410) = s3c6410
dir-$(CONFIG_AT91SAM9261) = at91sam926x
dir-$(CONFIG_AT91SAM9263) = at91sam926x

dir-y += lib
