SX1272_DIR := $(APP_DIR)/rtc
$(info SX1272_DIR = $(SX1272_DIR))

SX1272_SOURCES = $(wildcard $(SX1272_DIR)/*.c)
OBJ_DIRS += $(SX1272_DIR)/$(BUILD)/$(SOC)/$(OBJ)
OBJECTS += $(addprefix $(SX1272_DIR)/$(BUILD)/$(SOC)/$(OBJ)/,$(notdir $(SX1272_SOURCES:.c=.o)))
CFLAGS += -I$(SX1272_DIR)

$(SX1272_DIR)/$(BUILD)/$(SOC)/$(OBJ)/%.o: $(SX1272_DIR)/%.c $(SX1272_DIR)/%.h libqmsi
	$(call mkdir, $(SX1272_DIR)/$(BUILD)/$(SOC)/$(OBJ))
	$(CC) $(CFLAGS) -c -o $@ $<