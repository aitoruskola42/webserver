NAME = webserver
CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -std=c++98 -I./includes -I./src
SRC_DIR = src
OBJ_DIR = .obj
RESET			= 	\033[0m
GREEN 			= 	\033[38;5;46m
WHITE 			= 	\033[38;5;15m
GREY 			= 	\033[38;5;8m
ORANGE 			= 	\033[38;5;202m
RED 			= 	\033[38;5;160m
# Encuentra todos los archivos .cpp en src/ y sus subdirectorios
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
# Genera los nombres de los archivos objeto correspondientes
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Nombre del ejecutable
TARGET = $(NAME)

# Regla principal
$(TARGET): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "$(GREEN)╔════════════════════════════════════════════════╗$(RESET)"
	@echo "$(GREEN)║  🎉 RESOUNDING SUCCESS! Executable created 🎉  ║$(RESET)"
	@echo "$(GREEN)║                                                ║$(RESET)"
	@echo "$(GREEN)║    🚀 $(ORANGE)$(TARGET)$(GREEN) is ready to conquer 🚀         ║$(RESET)"
	@echo "$(GREEN)║                                                ║$(RESET)"
	@echo "$(GREEN)╚════════════════════════════════════════════════╝$(RESET)"
	@echo "$(ORANGE)         🌟 Let the adventure begin! 🌟$(RESET)"
	@echo "$(WHITE)             ⚔️  Good luck! ⚔️$(RESET)"
	@echo ""
	@chmod 000 www/examen/files/sinpermisos.jpg

# Regla para compilar cada archivo .cpp a .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	@echo "$(GREY)Compiling.............................$(GREEN)$<"
	@$(CXX) $(CXXFLAGS) -c -o $@ $<
	@echo "$(GREEN)DONE!$(RESET)"
	@sleep .1
	@clear



# Regla para limpiar los archivos objeto
clean:
	@clear
	@rm -rf $(OBJ_DIR)
	@echo "$(RED)╔═══════════════════════════════════════════════╗$(RESET)"
	@echo "$(RED)║       🗑️  MASSIVE CLEANUP COMPLETED 🗑️          ║$(RESET)"
	@echo "$(RED)║                                               ║$(RESET)"
	@echo "$(RED)║   🧹 Objects removed  🧹 Folders emptied      ║$(RESET)"
	@echo "$(RED)║                                               ║$(RESET)"
	@echo "$(RED)║     💨 The project is now squeaky clean 💨    ║$(RESET)"
	@echo "$(RED)║                                               ║$(RESET)"
	@echo "$(RED)╚═══════════════════════════════════════════════╝$(RESET)"
	@echo "$(ORANGE)    🌪️  Ready for a fresh compilation!  🌪️$(RESET)"
	@sleep 2
	@clear

# Regla para limpiar todo, incluyendo el ejecutable
fclean: 
	@clear
	@echo "$(RED)╔═══════════════════════════════════════════════╗$(RESET)"
	@echo "$(RED)║      🔥 TOTAL ANNIHILATION IN PROGRESS 🔥     ║$(RESET)"
	@echo "$(RED)╚═══════════════════════════════════════════════╝$(RESET)"
	@echo "$(ORANGE)Removing object files...$(RESET)"
	@rm -rf $(OBJ_DIR)
	@echo "$(ORANGE)Erasing executable [$(WHITE)$(TARGET)$(ORANGE)]...$(RESET)"
	@rm -f $(TARGET)
	@echo "$(RED)╔═══════════════════════════════════════════════╗$(RESET)"
	@echo "$(RED)║      🔥 TOTAL ANNIHILATION COMPLETED 🔥       ║$(RESET)"
	@echo "$(RED)║                                               ║$(RESET)"
	@echo "$(RED)║  💥 Objects obliterated  💥 Executable erased ║$(RESET)"
	@echo "$(RED)║                                               ║$(RESET)"
	@echo "$(RED)║   🌟 The project has returned to stardust 🌟  ║$(RESET)"
	@echo "$(RED)║                                               ║$(RESET)"
	@echo "$(RED)╚═══════════════════════════════════════════════╝$(RESET)"
	@echo "$(ORANGE) 🚀 Ready to rebuild from the ashes! 🚀$(RESET)"
	@chmod 666 www/examen/files/sinpermisos.jpg
	@clear

# Regla para recompilar todo
re: fclean all

# Regla para compilar todo (útil para la regla re)
all: $(TARGET)

# Regla para regenerar las dependencias
depend: $(SRCS)
	@mkdir -p $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) -MM $^ | sed 's/^\(.*\)\.o:/$(OBJ_DIR)\/\1.o:/' > $(OBJ_DIR)/depend

-include $(OBJ_DIR)/depend

.PHONY: clean fclean re all depend format author


author:
	@echo "Authors: $(GREEN)xamayuel | javigarc | auskola-$(RESET)"



format:
	@find src -type f \( -name "*.cpp"  \) -print0 | xargs -0 clang-format -i -style=file
	@echo "$(GREEN)Formateo completado para todos los archivos .cpp .$(RESET)"
	@find src -type f \( -name "*.hpp"  \) -print0 | xargs -0 clang-format -i -style=file:.hpp_format
	@echo "$(ORANGE)Formateo completado para todos los archivos .hpp .$(ORANGE)"

size:
	@echo "$(ORANGE)Ficheros totales en .src: $(GREEN)"
	@find ./src -type f | wc -l
	@echo "$(ORANGE)Ficheros .cpp: $(GREEN)"
	@find ./src -type f -name "*.cpp" | wc -l
	@echo "$(ORANGE)Ficheros .hpp: $(GREEN)"
	@find ./src -type f -name "*.hpp" | wc -l
	@echo "$(ORANGE)Numero de lineas de codigo: $(GREEN)"
	@find ./src -type f -exec cat {} + | wc -l