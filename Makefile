INSTALLGRP = root
INSTALLUSR = root
INSTALL = $(shell which install)
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
BASENAME = vmebus
SUBDIR = vmebridge/driver
INSTALLDIR = /lib/modules/$(shell uname -r)/kernel/drivers/vme
UDEVDIR = /etc/udev/rules.d

LIBSOURCES = vmebridge/lib/libvmebus.c
LIBOBJECTS = $(LIBSOURCES:.c=.o)
LIBINSTALLDIR = /usr/local/universe
FLAGS = -g -Wall -Ivmebridge/driver -Ivmebridge/lib
LIB = libvmebus.a

APISOURCES = $(wildcard universe_api/*.cc)
APIHEADERS = $(wildcard universe_api/*.hh)
APIOBJECTS = $(APISOURCES:.cc=.o)
APIFLAGS = -g -Wall -I$(LIBINSTALLDIR)/include
APILIB = libuniverse_api.a


%.o: %.c
	@echo compiling $*
	gcc -c $(FLAGS) -o $@ $<

%.o: %.cc
	@echo compiling $*
	gcc -c $(APIFLAGS) -o $@ $<

all: check

$(LIB): $(LIBOBJECTS)
	$(AR) -r $(LIB) $(LIBOBJECTS)

$(APILIB): $(APIOBJECTS)
#	$(AR) -r $(APILIB) $(APIOBJECTS)
	$(AR) -r $(APILIB) $(APIOBJECTS) $(LIBOBJECTS)

$(BASENAME).ko default:
	        $(MAKE) obj-m=$(BASENAME).o $(BASENAME)-y="vme_bridge.o tsi148.o vme_dma.o vme_irq.o vme_misc.o vme_window.o vme_cesif.o" -C $(KDIR) M=$(PWD)/$(SUBDIR) modules

install_driver: $(BASENAME).ko 
	        @echo
	        @echo "Beginning driver installation..."
	        @$(INSTALL) -o $(INSTALLUSR) -g $(INSTALLGRP) -d $(INSTALLDIR) || exit $$?
		@$(INSTALL) -o $(INSTALLUSR) -g $(INSTALLGRP) -m 644 40-vme.rules $(UDEVDIR) || exit $$?
		@$(INSTALL) -o $(INSTALLUSR) -g $(INSTALLGRP) -m 644 $(SUBDIR)/$(BASENAME).ko $(INSTALLDIR) || exit $$?
		@if lsmod | grep -q $(BASENAME); then rmmod $(BASENAME).ko || exit $$?; fi
		@depmod || exit $$?
		@modprobe $(BASENAME) || exit $$?
		@if lsmod | grep -q $(BASENAME); then echo "modprobe ok." || exit $$?; fi
	        @echo "kernel driver install complete."
	        @echo

uninstall_driver:
		@if lsmod | grep -q $(BASENAME); then rmmod $(BASENAME).ko || exit $$?; fi
	        @rm -f $(INSTALLDIR)/$(BASENAME).ko
	        @rm -f $(UDEVDIR)/40-vme.rules
	        @echo "kernel driver uninstall complete."
	        @echo

load_driver:
	        @echo "Loading kernel driver..."
		@if lsmod | grep -q $(BASENAME); echo "driver loaded already, unload_driver first." && exit $$?; fi
		@modprobe $(BASENAME) || exit $$?
		@if lsmod | grep -q $(BASENAME); then echo "modprobe ok." || exit $$?; fi
	        @echo "driver loaded."
	        @echo

unload_driver:
	        @echo "Unloading kernel driver..."
		@if lsmod | grep -q $(BASENAME); then rmmod $(BASENAME).ko || exit $$?; fi
	        @echo "driver unloaded."
	        @echo

install_lib: $(LIB)
	        @echo "Installing libvmebus..."
	        $(INSTALL) -o $(INSTALLUSR) -g $(INSTALLGRP) -d $(LIBINSTALLDIR) || exit $$?
	        $(INSTALL) -o $(INSTALLUSR) -g $(INSTALLGRP) -d $(LIBINSTALLDIR)/include || exit $$? 
	        $(INSTALL) -o $(INSTALLUSR) -g $(INSTALLGRP) -d $(LIBINSTALLDIR)/lib || exit $$?
	        $(INSTALL) -o $(INSTALLUSR) -g $(INSTALLGRP) -m 644 $(LIB) $(LIBINSTALLDIR)/lib || exit $$?
	        $(INSTALL) -o $(INSTALLUSR) -g $(INSTALLGRP) -m 644 vmebridge/driver/vmebus.h $(LIBINSTALLDIR)/include || exit $$?
	        $(INSTALL) -o $(INSTALLUSR) -g $(INSTALLGRP) -m 644 vmebridge/lib/libvmebus.h $(LIBINSTALLDIR)/include || exit $$?
		@echo "libvmebus installed."

uninstall_lib:
		@echo "Uninstalling libvmebus..."
	        @rm -f $(LIBINSTALLDIR)/include/vmebus.h $(LIBINSTALLDIR)/include/libvmebus.h $(LIBINSTALLDIR)/lib/$(LIB)
		@echo "libvmebus uninstalled."

install_api: $(APILIB)
	        @echo "Installing universe_api..."
	        $(INSTALL) -o $(INSTALLUSR) -g $(INSTALLGRP) -d $(LIBINSTALLDIR) || exit $$?
	        $(INSTALL) -o $(INSTALLUSR) -g $(INSTALLGRP) -d $(LIBINSTALLDIR)/include || exit $$? 
	        $(INSTALL) -o $(INSTALLUSR) -g $(INSTALLGRP) -d $(LIBINSTALLDIR)/lib || exit $$?
	        $(INSTALL) -o $(INSTALLUSR) -g $(INSTALLGRP) -m 644 $(APILIB) $(LIBINSTALLDIR)/lib || exit $$?
	        $(INSTALL) -o $(INSTALLUSR) -g $(INSTALLGRP) -m 644 universe_api/universe_api.h $(LIBINSTALLDIR)/include || exit $$?
		@for i in $(APIHEADERS); do ($(INSTALL) -o $(INSTALLUSR) -g $(INSTALLGRP) -m 644 $$i $(LIBINSTALLDIR)/include) || exit $$?; done
		@echo "universe_api installed."

uninstall_api:
		@echo "Uninstalling universe_api..."
	        @rm -f $(LIBINSTALLDIR)/include/universe_api.h $(LIBINSTALLDIR)/include/*.hh $(LIBINSTALLDIR)/lib/$(APILIB)
		@echo "universe_api uninstalled."



check:
	        @if lsmod | grep -q $(BASENAME); then echo "Driver loaded"; else echo "Driver not loaded."; exit 1; fi

clean:
	        rm -rf $(SUBDIR)/*.o $(SUBDIR)/Module.symvers $(SUBDIR)/$(BASENAME).ko*
		rm -rf $(SUBDIR)/.*.o.cmd $(SUBDIR)/.*.ko.cmd $(SUBDIR)/modules.order $(SUBDIR)/.tmp_versions
		rm -f $(LIB) $(LIBOBJECTS)
		rm -f $(APILIB) universe_api/*.o


