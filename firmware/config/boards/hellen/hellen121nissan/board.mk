# Combine the related files for a specific platform and MCU.

# Target ECU board design
BOARDCPPSRC = $(BOARDS_DIR)/hellen/hellen121nissan/board_configuration.cpp
BOARDINC = $(BOARDS_DIR)/hellen/hellen121nissan

# Set this if you want a default engine type other than normal hellen121nissan
ifeq ($(DEFAULT_ENGINE_TYPE),)
  DEFAULT_ENGINE_TYPE = -DDEFAULT_ENGINE_TYPE=HELLEN_121_NISSAN_6_CYL
endif

# 144 package MCU
ifeq ($(LED_CRITICAL_ERROR_BRAIN_PIN),)
  LED_CRITICAL_ERROR_BRAIN_PIN = -DLED_CRITICAL_ERROR_BRAIN_PIN=GPIOG_0
endif

DDEFS += -DEFI_MAIN_RELAY_CONTROL=TRUE

# Disable serial ports on this board as UART3 causes a DMA conflict with the SD card
DDEFS += -DTS_NO_PRIMARY=1 -DTS_NO_SECONDARY=1

# Add them all together
DDEFS += -DEFI_USE_OSC=TRUE -DFIRMWARE_ID=\"hellen121nissan\" $(DEFAULT_ENGINE_TYPE) $(LED_CRITICAL_ERROR_BRAIN_PIN) $(LED_COMMUNICATION_BRAIN_PIN)
DDEFS += -DEFI_SOFTWARE_KNOCK=TRUE -DSTM32_ADC_USE_ADC3=TRUE
DDEFS += -DEFI_ICU_INPUTS=FALSE -DHAL_TRIGGER_USE_PAL=TRUE



include $(BOARDS_DIR)/hellen/hellen-common.mk
