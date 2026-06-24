CC := gcc

APP := tetris
BUILD_DIR := build

SRC := src/main.c src/tetromino.c src/scoreboard.c src/menus.c src/tetris_menus.c src/sound.c

RAYLIB_DIR := libs/raylib
RAYLIB_SRC := $(RAYLIB_DIR)/src
RAYLIB_LIB := $(RAYLIB_SRC)/libraylib.a

CFLAGS := -Wall -Wextra -std=c99 -Isrc -I$(RAYLIB_SRC)
LDFLAGS := $(RAYLIB_LIB) -lGL -lm -lpthread -ldl -lrt -lX11

all: $(BUILD_DIR)/$(APP)

$(RAYLIB_LIB):
	$(MAKE) -C $(RAYLIB_SRC) PLATFORM=PLATFORM_DESKTOP

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(APP): $(SRC) $(RAYLIB_LIB) | $(BUILD_DIR)
	$(CC) $(SRC) -o $@ $(CFLAGS) $(LDFLAGS)

run: all
	./$(BUILD_DIR)/$(APP)

clean:
	rm -rf $(BUILD_DIR)

clean-raylib:
	$(MAKE) -C $(RAYLIB_SRC) clean

.PHONY: all run clean clean-raylib
