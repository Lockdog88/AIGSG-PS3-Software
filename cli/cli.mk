CLI_DIR := $(APP_DIR)/magnetometer
$(info CLI_DIR = $(CLI_DIR))

CLI_SOURCES = $(wildcard $(CLI_DIR)/*.c)
OBJ_DIRS += $(CLI_DIR)/$(BUILD)/$(SOC)/$(OBJ)
OBJECTS += $(addprefix $(CLI_DIR)/$(BUILD)/$(SOC)/$(OBJ)/,$(notdir $(CLI_SOURCES:.c=.o)))
CFLAGS += -I$(CLI_DIR)

$(CLI_DIR)/$(BUILD)/$(SOC)/$(OBJ)/%.o: $(CLI_DIR)/%.c $(CLI_DIR)/%.h libqmsi
	$(call mkdir, $(CLI_DIR)/$(BUILD)/$(SOC)/$(OBJ))
	$(CC) $(CFLAGS) -c -o $@ $<