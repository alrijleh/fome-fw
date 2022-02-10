# Combine the related files for a specific platform and MCU.

# Target ECU board design
BOARDCPPSRC = $(BOARDS_DIR)/hellen/hellen-nb1/board_configuration.cpp
BOARDINC = $(BOARDS_DIR)/hellen/hellen-nb1

# Set this if you want a default engine type other than normal hellen-nb1
ifeq ($(DEFAULT_ENGINE_TYPE),)
  DEFAULT_ENGINE_TYPE = -DDEFAULT_ENGINE_TYPE=HELLEN_NA8_96
endif

# 144 package MCU
ifeq ($(LED_CRITICAL_ERROR_BRAIN_PIN),)
  LED_CRITICAL_ERROR_BRAIN_PIN = -DLED_CRITICAL_ERROR_BRAIN_PIN=GPIOG_0
endif

DDEFS += -DEFI_MAIN_RELAY_CONTROL=TRUE

DDEFS += -DTS_NO_SECONDARY

# Add them all together
DDEFS += -DEFI_USE_OSC=TRUE -DFIRMWARE_ID=\"hellenNB1\" $(DEFAULT_ENGINE_TYPE) $(LED_CRITICAL_ERROR_BRAIN_PIN) $(LED_COMMUNICATION_BRAIN_PIN)
DDEFS += -DEFI_SOFTWARE_KNOCK=TRUE -DSTM32_ADC_USE_ADC3=TRUE
DDEFS += -DEFI_ICU_INPUTS=FALSE -DHAL_TRIGGER_USE_PAL=TRUE



# Enable serial pins on expansion header
DDEFS += -DEFI_CONSOLE_TX_BRAIN_PIN=GPIOD_6 -DEFI_CONSOLE_RX_BRAIN_PIN=GPIOD_5 -DTS_PRIMARY_PORT=UARTD2 -DSTM32_UART_USE_USART2=1

include $(BOARDS_DIR)/hellen/hellen-common.mk

