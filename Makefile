CC = gcc
CFLAGS += -march=native -O1 -pipe -Wall -g -std=c99

# Explicitly include glib, xcb, and X11 headers and libraries
GLIB_CFLAGS := $(shell pkg-config --cflags glib-2.0)
GLIB_LIBS := $(shell pkg-config --libs glib-2.0)

XCB_CFLAGS := $(shell pkg-config --cflags xcb x11 xcb-randr)
XCB_LIBS := $(shell pkg-config --libs xcb x11 xcb-randr)

LDFLAGS += -lm $(GLIB_LIBS) $(XCB_LIBS)

# Read version from version.txt
FITTSMON_VERSION := $(shell cat version.txt))

# Object files
OBJS = fittsmon.o

# Target executable
TARGET = fittsmon

# Configuration file to be installed
CONFIG_FILE := fittsmonrc_example
DEST_DIR := ~/.config/fittsmon
DEST_FILE := $(DEST_DIR)/fittsmonrc

# Installation directories
INSTALL_DIR := /usr/local/bin
MAN_INSTALL_DIR := /usr/local/share/man/man1
MAN_PAGE := fittsmon.1

# Default target builds binary and man page
.PHONY: all
all: $(TARGET) $(MAN_PAGE)

# Build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) $(GLIB_CFLAGS) $(XCB_CFLAGS) -c -o $@ $<

# Generate the man page from template
$(MAN_PAGE): fittsmon.1.in version.txt
	sed "s/@VERSION@/$(FITTSMON_VERSION)/g" $< > $@

# Clean build artifacts
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS) $(MAN_PAGE)

# Install executable, config, and man page
.PHONY: install
install: all
	@echo "Creating destination directory if it does not exist: $(DEST_DIR)"
	@mkdir -p $(DEST_DIR)
	
	@if [ -e $(DEST_FILE) ]; then \
		echo "Configuration file $(DEST_FILE) already exists – leaving it untouched."; \
	else \
		echo "Copying $(CONFIG_FILE) to $(DEST_FILE)"; \
		cp $(CONFIG_FILE) $(DEST_FILE); \
	fi
	
	@echo "Installing executable to $(INSTALL_DIR)/$(TARGET)"
	@sudo mkdir -p $(INSTALL_DIR)
	@sudo cp $(TARGET) $(INSTALL_DIR)

	@echo "Installing man page to $(MAN_INSTALL_DIR)"
	@sudo mkdir -p $(MAN_INSTALL_DIR)
	@sudo cp $(MAN_PAGE) $(MAN_INSTALL_DIR)

	@echo "Installation complete!"
	@echo "You can now access the manual with: man fittsmon"

# Uninstall executable and man page
.PHONY: uninstall
uninstall:
	@echo "Removing executable from $(INSTALL_DIR)"
	@sudo rm -f $(INSTALL_DIR)/$(TARGET)
	@echo "Removing man page from $(MAN_INSTALL_DIR)"
	@sudo rm -f $(MAN_INSTALL_DIR)/$(MAN_PAGE)
	@echo "Uninstall complete!"
	@echo "Note: Configuration file in $(DEST_FILE) was not removed"
