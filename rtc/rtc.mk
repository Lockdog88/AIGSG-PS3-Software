RTC_DIR := $(APP_DIR)/rtc
$(info RTC_DIR = $(RTC_DIR))

RTC_SOURCES = $(wildcard $(RTC_DIR)/*.c)
OBJ_DIRS += $(RTC_DIR)/$(BUILD)/$(SOC)/$(OBJ)
OBJECTS += $(addprefix $(RTC_DIR)/$(BUILD)/$(SOC)/$(OBJ)/,$(notdir $(RTC_SOURCES:.c=.o)))
CFLAGS += -I$(RTC_DIR)

$(RTC_DIR)/$(BUILD)/$(SOC)/$(OBJ)/%.o: $(RTC_DIR)/%.c $(RTC_DIR)/%.h libqmsi
	$(call mkdir, $(RTC_DIR)/$(BUILD)/$(SOC)/$(OBJ))
	$(CC) $(CFLAGS) -c -o $@ $<