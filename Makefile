# Путь к компилятору
COMPILER=g++

# Флаги компиляции
FLAGS=-Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wmissing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -D_DEBUG -D_EJUDGE_CLIENT_

# Папка с объектами
BIN_DIR=bin

# Папка с исходниками и заголовками
SRC_DIR=src

# Список объектов в папке bin
OBJECTS=$(BIN_DIR)/*.o


all: run


# Объединяет объекты в исполняемый файл
run: $(BIN_DIR)/main.o
	$(COMPILER) $^ -o run.exe


# Компилирует все файлы в папке src в папку bin
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(COMPILER) $(FLAGS) -c $< -o $@
