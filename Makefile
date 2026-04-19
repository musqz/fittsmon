CC      = gcc
# Caller's CFLAGS are respected; we append only what is strictly required.
# -march=native is intentionally excluded — it is not appropriate for
# distributed packages and breaks DESTDIR-based staging.
CFLAGS  += -O1 -pipe -Wall -g -std=c99

GLIB_CFLAGS := $(shell pkg-config --cflags glib-2.0)
GLIB_LIBS   := $(shell pkg-config --libs   glib-2.0)

XCB_CFLAGS  := $(shell pkg-config --cflags xcb x11 xcb-randr)
XCB_LIBS    := $(shell pkg-config --libs   xcb x11 xcb-randr)

LDFLAGS += -lm $(GLIB_LIBS) $(XCB_LIBS)

OBJS   = fittsmon.o
TARGET = fittsmon

# Installation layout — all overridable on the command line.
# DESTDIR is used by package managers to stage files before final install.
DESTDIR     =
PREFIX      = /usr/local
BINDIR      = $(PREFIX)/bin
MANDIR      = $(PREFIX)/share/man/man1
DATADIR     = $(PREFIX)/share/$(TARGET)
LICENSEDIR  = $(PREFIX)/share/licenses/$(TARGET)

# Example config (never written to $HOME by make install)
CONFIG_FILE = fittsmonrc_example
MAN_PAGE    = fittsmon.1

# -----------------------------------------------------------------------

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(GLIB_CFLAGS) $(XCB_CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)

install: $(TARGET)
	install -Dm755 $(TARGET)        $(DESTDIR)$(BINDIR)/$(TARGET)
	install -Dm644 $(MAN_PAGE)      $(DESTDIR)$(MANDIR)/$(MAN_PAGE).gz
	gzip -nf                        $(DESTDIR)$(MANDIR)/$(MAN_PAGE).gz 2>/dev/null; \
	  install -Dm644 $(MAN_PAGE)    $(DESTDIR)$(MANDIR)/$(MAN_PAGE) && \
	  gzip -nf                      $(DESTDIR)$(MANDIR)/$(MAN_PAGE) || true
	install -Dm644 $(CONFIG_FILE)   $(DESTDIR)$(DATADIR)/$(CONFIG_FILE)
	install -Dm644 COPYING          $(DESTDIR)$(LICENSEDIR)/COPYING
	@echo ""
	@echo "Installation complete."
	@echo "  Binary : $(DESTDIR)$(BINDIR)/$(TARGET)"
	@echo "  Man    : $(DESTDIR)$(MANDIR)/$(MAN_PAGE).gz"
	@echo "  Example config: $(DESTDIR)$(DATADIR)/$(CONFIG_FILE)"
	@echo ""
	@echo "Copy the example config to get started:"
	@echo "  mkdir -p ~/.config/fittsmon"
	@echo "  cp $(DATADIR)/$(CONFIG_FILE) ~/.config/fittsmon/fittsmonrc"

uninstall:
	rm -f  $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f  $(DESTDIR)$(MANDIR)/$(MAN_PAGE).gz
	rm -f  $(DESTDIR)$(DATADIR)/$(CONFIG_FILE)
	rm -f  $(DESTDIR)$(LICENSEDIR)/COPYING
	@echo "Uninstall complete."
	@echo "Note: ~/.config/fittsmon/fittsmonrc was not removed."

.PHONY: all clean install uninstall
