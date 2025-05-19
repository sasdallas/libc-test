include ./make.config
include ./features.config

SRC_DIR = src
OBJ_DIR = obj
OBJ_DIR_REQUIRED = obj/required
OBJ_DIR_API = obj/api

API_TESTS = $(SRC_DIR)/api

TEST_DIRS = $(SRC_DIR)/functional $(SRC_DIR)/math $(SRC_DIR)/regression
TEST_OUTPUT_DIRS = $(patsubst $(SRC_DIR)%, $(OBJ_DIR)%, $(TEST_DIRS))

REQUIRED_SRC = $(SRC_DIR)/common
TEST_BACKUP = $(SRC_DIR)/special/test-backup.c
TEST_RUNNER = $(SRC_DIR)/special/test-runner.c

# Construct object lists
TEST_C_SOURCES = $(shell find $(TEST_DIRS) -maxdepth 1 -name "*.c" -type f)
TEST_C_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%, $(TEST_C_SOURCES))

REQUIRED_C_SOURCES = $(wildcard $(REQUIRED_SRC)/*.c)
REQUIRED_C_OBJECTS = $(REQUIRED_C_SOURCES:$(REQUIRED_SRC)%.c=$(OBJ_DIR_REQUIRED)%.o)

API_C_SOURCES = $(wildcard $(API_TESTS)/*.c)
API_C_OBJECTS = $(API_C_SOURCES:$(API_TESTS)%.c=$(OBJ_DIR_API)%.o)

MAKE_DIRECTORIES:
	@printf "[MAKE DIRECTORIES] Make directories\n"
	-@mkdir -pv $(OBJ_DIR)
	-@mkdir -pv $(OBJ_DIR_REQUIRED)
	-@mkdir -pv $(TEST_OUTPUT_DIRS)
	-rm compile_log.txt

$(OBJ_DIR_REQUIRED)/%.o: $(REQUIRED_SRC)/%.c
	@printf "[COMPILE:REQUIRED] $<\n"
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/libtest.a: $(REQUIRED_C_OBJECTS)
	@printf "[LD:REQUIRED     ] $@\n"
	@$(AR) rcs $@ $(REQUIRED_C_OBJECTS)

$(OBJ_DIR_API)/%.o: $(API_TESTS)/%.c
	@printf "[COMPILE:API     ] $<\n"
	@if $(CC) $(CFLAGS) -c $< -o $@ >> compile_log.txt 2>&1; then \
		./log_update.sh "[COMPILE:SUCCESS] $<" \
	else \
		./log_update.sh "[COMPILE:FAILURE] $<" \
	fi


$(OBJ_DIR)/%: $(SRC_DIR)/%.c Makefile
	@printf "[COMPILE:TEST    ] $@\n"	
	@if $(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LIBS) >> compile_log.txt 2>&1; then \
		./log_update.sh "[COMPILE:SUCCESS] $<"; \
	else \
		./log_update.sh "[COMPILE:FAILURE] $<"; \
	fi

	@if [ ! -f "$@" ]; then \
		$(CC) $(CFLAGS) $(LDFLAGS) $(TEST_BACKUP) -o $@ $(LIBS) -DTEST_FILE_NAME="$<"; \
	fi

$(OBJ_DIR)/test: $(TEST_RUNNER)
	@printf "[COMPILE:RUNNER  ] $@\n"
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LIBS)


.PHONY: all
all: MAKE_DIRECTORIES $(OBJ_DIR)/libtest.a $(TEST_C_OBJECTS) $(OBJ_DIR)/test

clean:
	-rm -rf $(OBJ_DIR)