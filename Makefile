CC = gcc
CFLAGS += -march=native -O1 -pipe -Wall -g -std=c99

# Explicitly include glib, xcb, and X11 headers and libraries
GLIB_CFLAGS := $(shell pkg-config --cflags glib-2.0)
GLIB_LIBS := $(shell pkg-config --libs glib-2.0)

XCB_CFLAGS := $(shell pkg-config --cflags xcb x11 xcb-randr)
XCB_LIBS := $(shell pkg-config --libs xcb x11 xcb-randr)

LDFLAGS += -lm $(GLIB_LIBS) $(XCB_LIBS)

# Define the object files
OBJS = fittsmon.o

# Target executable
TARGET = fittsmon

# Configuration file to be installed
CONFIG_FILE := fittsmonrc
DEST_DIR := ~/.config/fittsmon/
DEST_FILE := $(DEST_DIR)$(CONFIG_FILE)
INSTALL_DIR := /usr/local/bin

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(GLIB_CFLAGS) $(XCB_CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)

install:
	@echo "Creating destination directory if it does not exist: $(DEST_DIR)"
	@mkdir -p $(DEST_DIR)
	@echo "Copying $(CONFIG_FILE) to $(DEST_FILE)"
	@cp $(CONFIG_FILE) $(DEST_FILE)
	@echo "Installing executable to $(INSTALL_DIR)/$(TARGET)"
	@sudo mkdir -p $(INSTALL_DIR)
	@sudo cp $(TARGET) $(INSTALL_DIR)
