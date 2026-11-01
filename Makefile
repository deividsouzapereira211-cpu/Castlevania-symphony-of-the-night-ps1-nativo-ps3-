TARGET = sotn_ps3
INCLUDE = -I./ps1rt/include -I./ps1rt/source
CXX = g++
CC = gcc
OBJCOPY = objcopy

CXXFLAGS = -std=c++17 -O0 -g -Wall $(INCLUDE) -D__PS3__
LDFLAGS = 

SRC_DIR = ps1rt/source
OBJ_DIR = obj_ps3

SRCS = $(SRC_DIR)/main.cpp \
       $(SRC_DIR)/mips_router.cpp \
       $(SRC_DIR)/mips_main_gen.cpp \
       $(SRC_DIR)/mips_dra_gen.cpp

OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: $(TARGET).elf

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET).elf $(OBJS) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	@echo "Compilando bloco Castlevania: $<"
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ_DIR) $(TARGET).elf

.PHONY: all clean
