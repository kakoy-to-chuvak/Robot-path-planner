SDL_INCLUDE_DIR=C:/SDL/x86_64-w64-mingw32/include
SDL_LIBS_DIR=C:/SDL/x86_64-w64-mingw32/lib


SRC_DIR=src
BUILD_DIR=build
TARGET_EXEC=main



SDL_LIBS=SDL3_ttf SDL3_image SDL3
SRCS=$(wildcard $(SRC_DIR)/*.c)
HEADERS=$(wildcard $(SRC_DIR)/*.h)

SDL_L_ARGS=$(SDL_LIBS:%=-l%)


$(BUILD_DIR)/$(TARGET_EXEC): $(SRCS) $(HEADERS)
	@echo -------------------
ifeq ($(wildcard $(BUILD_DIR)/*),)
	@echo create build directory
	mkdir $(BUILD_DIR)
endif
	@echo Compiling...
	gcc 	$(SRCS) \
		-o "$(BUILD_DIR)/$(TARGET_EXEC)" \
		-L $(SDL_LIBS_DIR) -I $(SDL_INCLUDE_DIR) $(SDL_L_ARGS) \
		-Wall -Wextra -Werror \
		-Wno-alloc-size
	@echo -------------------

all: $(BUILD_DIR)/$(TARGET_EXEC)