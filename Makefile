SRC_DIR=src
BUILD_DIR=build
TARGET_EXEC=main

SDL_LIBS=SDL3_ttf SDL3_image SDL3
SDL_INCLUDE_DIR=C:/SDL/x86_64-w64-mingw32/include
SDL_LIBS_DIR=C:/SDL/x86_64-w64-mingw32/lib

SDL_LIBS_EXTENSION=.dll.a



SRCS=$(wildcard $(SRC_DIR)/*.c)
HEADERS=$(wildcard $(SRC_DIR)/*.h)

SDL_L_ARGS=$(SDL_LIBS:%=-l%)
SDL_LIB_REQ=$(SDL_LIBS:%=$(SDL_LIBS_DIR)/lib%$(SDL_LIBS_EXTENSION))
SDL_H_REQ=$(foreach lib,$(SDL_LIBS),$(SDL_INCLUDE_DIR)/$(lib)/$(lib:SDL3%=SDL%).h)


$(BUILD_DIR)/$(TARGET_EXEC): $(SRCS) $(HEADERS) $(SDL_LIB_REQ) $(SDL_H_REQ)
	@echo -------------------
ifeq ($(wildcard path/$(var)/*),)
	@echo create build directory
	mkdir $(BUILD_DIR)
endif
	@echo Compiling...
	gcc "$(SRC_DIR)/*.c" -o "$(BUILD_DIR)/$(TARGET_EXEC)" -L $(SDL_LIBS_DIR) -I $(SDL_INCLUDE_DIR) $(SDL_L_ARGS)
	@echo -------------------
