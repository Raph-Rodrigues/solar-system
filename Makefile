# Compilador e flags de otimização/avisos
CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2

# Coleta os caminhos de include e bibliotecas do SDL3 usando o pkg-config
SDL3_CFLAGS := $(shell pkg-config --cflags sdl3)
SDL3_LIBS   := $(shell pkg-config --libs sdl3)

# Nome do binário final gerado
TARGET = solar-system

# Arquivo fonte
SRCS = main.cpp

# Regra padrão executada ao rodar apenas 'make'
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SDL3_CFLAGS) $(SRCS) -o $(TARGET) $(SDL3_LIBS)

# Regra para compilar e rodar o projeto de forma direta
run: $(TARGET)
	./$(TARGET)

# Limpeza dos arquivos gerados
clean:
	rm -f $(TARGET)

.PHONY: all run clean

