# Conch Shell Makefile
# Production-grade build system

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -D_DEFAULT_SOURCE -Iinclude
LDFLAGS =
DEBUGFLAGS = -g -O0 -DDEBUG
RELEASEFLAGS = -O2 -DNDEBUG

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN_DIR = bin
DOCS_DIR = docs

# Target executable
TARGET = conch
INSTALL_DIR = /usr/local/bin

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPENDS = $(OBJECTS:.o=.d)

# Colors for output
COLOR_RESET = \033[0m
COLOR_GREEN = \033[32m
COLOR_YELLOW = \033[33m
COLOR_CYAN = \033[36m

# Default target
.PHONY: all
all: release

# Release build
.PHONY: release
release: CFLAGS += $(RELEASEFLAGS)
release: $(BIN_DIR)/$(TARGET)
	@echo "$(COLOR_GREEN)✓ Release build complete: $(BIN_DIR)/$(TARGET)$(COLOR_RESET)"

# Debug build
.PHONY: debug
debug: CFLAGS += $(DEBUGFLAGS)
debug: clean $(BIN_DIR)/$(TARGET)
	@echo "$(COLOR_YELLOW)✓ Debug build complete: $(BIN_DIR)/$(TARGET)$(COLOR_RESET)"

# Link the target
$(BIN_DIR)/$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@echo "$(COLOR_CYAN)Linking $(TARGET)...$(COLOR_RESET)"
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "$(COLOR_CYAN)Compiling $<...$(COLOR_RESET)"
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Create directories
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Include dependencies
-include $(DEPENDS)

# Clean build artifacts
.PHONY: clean
clean:
	@echo "$(COLOR_YELLOW)Cleaning build artifacts...$(COLOR_RESET)"
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "$(COLOR_GREEN)✓ Clean complete$(COLOR_RESET)"

# Install to system
.PHONY: install
install: release
	@echo "$(COLOR_CYAN)Installing $(TARGET) to $(INSTALL_DIR)...$(COLOR_RESET)"
	@sudo cp $(BIN_DIR)/$(TARGET) $(INSTALL_DIR)/$(TARGET)
	@sudo chmod 755 $(INSTALL_DIR)/$(TARGET)
	@echo "$(COLOR_GREEN)✓ Installation complete$(COLOR_RESET)"

# Uninstall from system
.PHONY: uninstall
uninstall:
	@echo "$(COLOR_YELLOW)Uninstalling $(TARGET) from $(INSTALL_DIR)...$(COLOR_RESET)"
	@sudo rm -f $(INSTALL_DIR)/$(TARGET)
	@echo "$(COLOR_GREEN)✓ Uninstall complete$(COLOR_RESET)"

# Run the shell
.PHONY: run
run: release
	@echo "$(COLOR_CYAN)Starting $(TARGET)...$(COLOR_RESET)"
	@./$(BIN_DIR)/$(TARGET)

# Run the test suite (pipes, redirects, builtins)
.PHONY: test
test: release
	@echo "$(COLOR_CYAN)Running test suite...$(COLOR_RESET)"
	tests/run_tests.sh $(BIN_DIR)/$(TARGET)

# Run the test suite under valgrind --leak-check=full (Linux only)
.PHONY: memcheck
memcheck: debug
	@echo "$(COLOR_CYAN)Running test suite under valgrind...$(COLOR_RESET)"
	tests/run_tests.sh --valgrind $(BIN_DIR)/$(TARGET)

# Run the shell interactively under valgrind
.PHONY: memcheck-interactive
memcheck-interactive: debug
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(BIN_DIR)/$(TARGET)

# Static analysis with cppcheck
.PHONY: check
check:
	@echo "$(COLOR_CYAN)Running static analysis...$(COLOR_RESET)"
	cppcheck --enable=all --suppress=missingIncludeSystem $(SRC_DIR)

# Format code
.PHONY: format
format:
	@echo "$(COLOR_CYAN)Formatting code...$(COLOR_RESET)"
	@find $(SRC_DIR) $(INC_DIR) -name "*.c" -o -name "*.h" | xargs clang-format -i
	@echo "$(COLOR_GREEN)✓ Code formatted$(COLOR_RESET)"

# Count lines of code
.PHONY: stats
stats:
	@echo "$(COLOR_CYAN)Code statistics:$(COLOR_RESET)"
	@echo "Source files:"
	@wc -l $(SRC_DIR)/*.c
	@echo ""
	@echo "Header files:"
	@wc -l $(INC_DIR)/*.h
	@echo ""
	@echo "Total:"
	@cat $(SRC_DIR)/*.c $(INC_DIR)/*.h | wc -l

# Show help
.PHONY: help
help:
	@echo "$(COLOR_GREEN)Conch Shell Build System$(COLOR_RESET)"
	@echo ""
	@echo "Available targets:"
	@echo "  $(COLOR_CYAN)all$(COLOR_RESET)       - Build release version (default)"
	@echo "  $(COLOR_CYAN)release$(COLOR_RESET)   - Build optimized release version"
	@echo "  $(COLOR_CYAN)debug$(COLOR_RESET)     - Build debug version with symbols"
	@echo "  $(COLOR_CYAN)clean$(COLOR_RESET)     - Remove build artifacts"
	@echo "  $(COLOR_CYAN)install$(COLOR_RESET)   - Install to $(INSTALL_DIR)"
	@echo "  $(COLOR_CYAN)uninstall$(COLOR_RESET) - Remove from $(INSTALL_DIR)"
	@echo "  $(COLOR_CYAN)run$(COLOR_RESET)       - Build and run the shell"
	@echo "  $(COLOR_CYAN)test$(COLOR_RESET)      - Run the test suite"
	@echo "  $(COLOR_CYAN)memcheck$(COLOR_RESET)  - Run the test suite under valgrind (requires valgrind)"
	@echo "  $(COLOR_CYAN)memcheck-interactive$(COLOR_RESET) - Run the shell under valgrind"
	@echo "  $(COLOR_CYAN)check$(COLOR_RESET)     - Run static analysis (requires cppcheck)"
	@echo "  $(COLOR_CYAN)format$(COLOR_RESET)    - Format code (requires clang-format)"
	@echo "  $(COLOR_CYAN)stats$(COLOR_RESET)     - Show code statistics"
	@echo "  $(COLOR_CYAN)help$(COLOR_RESET)      - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make              # Build release version"
	@echo "  make debug        # Build debug version"
	@echo "  make run          # Build and run"
	@echo "  make install      # Install system-wide"

.PHONY: docker-build
docker-build:
	@echo "$(COLOR_CYAN)Building Docker image...$(COLOR_RESET)"
	docker build -t conch-shell .
	@echo "$(COLOR_GREEN)✓ Docker image built$(COLOR_RESET)"

.PHONY: docker-run
docker-run:
	@echo "$(COLOR_CYAN)Running Conch Shell in Docker...$(COLOR_RESET)"
	docker run -it --rm conch-shell
