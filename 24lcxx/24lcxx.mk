24LCXX_DIR := $(APP_DIR)/rtc
$(info 24LCXX_DIR = $(24LCXX_DIR))

24LCXX_SOURCES = $(wildcard $(24LCXX_DIR)/*.c)
OBJ_DIRS += $(24LCXX_DIR)/$(BUILD)/$(SOC)/$(OBJ)
OBJECTS += $(addprefix $(24LCXX_DIR)/$(BUILD)/$(SOC)/$(OBJ)/,$(notdir $(24LCXX_SOURCES:.c=.o)))
CFLAGS += -I$(24LCXX_DIR)

$(24LCXX_DIR)/$(BUILD)/$(SOC)/$(OBJ)/%.o: $(24LCXX_DIR)/%.c $(24LCXX_DIR)/%.h libqmsi
	$(call mkdir, $(24LCXX_DIR)/$(BUILD)/$(SOC)/$(OBJ))
	$(CC) $(CFLAGS) -c -o $@ $<