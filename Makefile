CONTIKI_PROJECT = ctk_sch
all: $(CONTIKI_PROJECT)

#UIP_CONF_IPV6=1

CONTIKI = /home/user/contiki-2.7
include $(CONTIKI)/Makefile.include
