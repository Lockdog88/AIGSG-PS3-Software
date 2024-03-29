MAGNETOMETER_DIR := $(APP_DIR)/magnetometer
$(info MAGNETOMETER_DIR = $(MAGNETOMETER_DIR))

MAGNETOMETER_SOURCES = $(wildcard $(MAGNETOMETER_DIR)/*.c)
OBJ_DIRS += $(MAGNETOMETER_DIR)/$(BUILD)/$(SOC)/$(OBJ)
OBJECTS += $(addprefix $(MAGNETOMETER_DIR)/$(BUILD)/$(SOC)/$(OBJ)/,$(notdir $(MAGNETOMETER_SOURCES:.c=.o)))
CFLAGS += -I$(MAGNETOMETER_DIR)

$(MAGNETOMETER_DIR)/$(BUILD)/$(SOC)/$(OBJ)/%.o: $(MAGNETOMETER_DIR)/%.c $(MAGNETOMETER_DIR)/%.h libqmsi
	$(call mkdir, $(MAGNETOMETER_DIR)/$(BUILD)/$(SOC)/$(OBJ))
	$(CC) $(CFLAGS) -c -o $@ $<