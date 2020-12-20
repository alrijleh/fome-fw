/**
 * @file	rover_v8.cpp
 *
 * V8, firing order 18436572
 *
 * set engine_type 10
 *
 * ROVER_V8 = 10
 *
 * @date Jun 27, 2014
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "global.h"
#include "rover_v8.h"
#include "engine_math.h"
#include "allsensors.h"

void setFrankenstein_01_LCD(engine_configuration_s *engineConfiguration) {
	engineConfiguration->HD44780_rs = GPIO_UNASSIGNED; // todo#2108 GPIOE_9;
	engineConfiguration->HD44780_e = GPIO_UNASSIGNED; // todo#2108 GPIOE_11;
	engineConfiguration->HD44780_db4 = GPIO_UNASSIGNED; // todo#2108 GPIOE_13;
	engineConfiguration->HD44780_db5 = GPIO_UNASSIGNED; // todo#2108 GPIOE_15;
	engineConfiguration->HD44780_db6 = GPIO_UNASSIGNED; // todo#2108 GPIOB_11;
	engineConfiguration->HD44780_db7 = GPIO_UNASSIGNED; // todo#2108 GPIOB_13;
}

EXTERN_CONFIG;

void setRoverv8(DECLARE_CONFIG_PARAMETER_SIGNATURE) {
	setDefaultFrankensoConfiguration(PASS_CONFIG_PARAMETER_SIGNATURE);

	setOperationMode(engineConfiguration, FOUR_STROKE_CRANK_SENSOR);
	// set trigger_type 9
	engineConfiguration->trigger.type = TT_TOOTHED_WHEEL_36_1;

	// todo#2108 	engineConfiguration->is_enabled_spi_2 = false;
	// todo#2108 	engineConfiguration->isHip9011Enabled = false;
	// todo#2108CONFIG(hip9011IntHoldPin) = GPIO_UNASSIGNED;
	// todo#2108 setFrankenstein_01_LCD(engineConfiguration);

	engineConfiguration->specs.displacement = 3.528;
	engineConfiguration->specs.cylindersCount = 8;
	engineConfiguration->specs.firingOrder = FO_1_8_4_3_6_5_7_2;

	// set_rpm_hard_limit 4000
	engineConfiguration->rpmHardLimit = 4000; // yes, 4k. let's play it safe for now
	// set_cranking_rpm 350
	engineConfiguration->cranking.rpm = 350;

	// set cranking_injection_mode 0
	engineConfiguration->crankingInjectionMode = IM_SIMULTANEOUS;
	// set injection_mode 1
	engineConfiguration->injectionMode = IM_SEQUENTIAL;

	// set ignition_mode 2
	engineConfiguration->ignitionMode = IM_WASTED_SPARK;
	/*
	 * this configuration is used on HW CI - in HW CI 'inverted' would rightfully fail unless there is pull-up
	engineConfiguration->ignitionPinMode = OM_INVERTED;
*/

    // set_ignition_channels
    engineConfiguration->ignitionPins[0] = GPIO_UNASSIGNED; // todo#2108 GPIOE_8; // Frankenstein: low side - out #x (?)
    engineConfiguration->ignitionPins[7] = GPIO_UNASSIGNED; // todo#2108 GPIOE_10; // Frankenstein: low side - out #x (?)
    engineConfiguration->ignitionPins[3] = GPIO_UNASSIGNED; // todo#2108 GPIOE_12; // Frankenstein: low side - out #x (?)
    engineConfiguration->ignitionPins[2] = GPIO_UNASSIGNED; // todo#2108 GPIOE_14; // Frankenstein: low side - out #x (?)

	// Frankenstein: low side - out #1: PC14
	// Frankenstein: low side - out #2: PC15
	// Frankenstein: low side - out #3: PE6
	// Frankenstein: low side - out #4: PC13
	// Frankenstein: low side - out #5: PE4
	// Frankenstein: low side - out #6: PE5
	// Frankenstein: low side - out #7: PE2
	// Frankenstein: low side - out #8: PE3
	// Frankenstein: low side - out #9: PE0
	// Frankenstein: low side - out #10: PE1
	// Frankenstein: low side - out #11: PB8
	// Frankenstein: low side - out #12: PB9

	engineConfiguration->injectionPins[0] = GPIO_UNASSIGNED; // todo#2108 GPIOC_14; // Frankenstein: low side - out #1
	engineConfiguration->injectionPins[1] = GPIO_UNASSIGNED; // todo#2108 GPIOC_15; // Frankenstein: low side - out #2
	engineConfiguration->injectionPins[2] = GPIO_UNASSIGNED; // todo#2108 GPIOE_6; // Frankenstein: low side - out #3
	engineConfiguration->injectionPins[3] = GPIO_UNASSIGNED; // todo#2108 GPIOC_13; // Frankenstein: low side - out #4
    engineConfiguration->injectionPins[4] = GPIO_UNASSIGNED; // todo#2108 GPIOE_4; // Frankenstein: low side - out #5
    engineConfiguration->injectionPins[5] = GPIO_UNASSIGNED; // todo#2108 GPIOE_5; // Frankenstein: low side - out #6
    engineConfiguration->injectionPins[6] = GPIO_UNASSIGNED; // todo#2108 GPIOE_2; // Frankenstein: low side - out #7
    engineConfiguration->injectionPins[7] = GPIO_UNASSIGNED; // todo#2108 GPIOE_3; // Frankenstein: low side - out #8

// not valid ICU pin	engineConfiguration->vehicleSpeedSensorInputPin = GPIOC_2;

	//GPIOE_0 AND GPIOE_1 are bad pins since they conflict with accelerometer
	//no malfunction indicator pin needed, since we use CAN_BUS_MAZDA_RX8
	//engineConfiguration->fuelPumpPin = GPIOE_0; // Frankenstein: low side - out #9
	//engineConfiguration->malfunctionIndicatorPin = GPIOE_1; // Frankenstein: low side - out #10
    // todo#2108 engineConfiguration->fuelPumpPin = GPIO_UNASSIGNED; // todo#2108 GPIOB_8; // Frankenstein: low side - out #11
	// todo#2108 engineConfiguration->fuelPumpPinMode = OM_DEFAULT;
	// todo#2108 engineConfiguration->mainRelayPin  = GPIO_UNASSIGNED; // todo#2108 GPIOB_9; // Frankenstein: low side - out #12

	// todo#2108 engineConfiguration->triggerInputPins[0] = GPIOC_6; // 2G YEL/BLU
    // todo#2108 engineConfiguration->triggerInputPins[1] = GPIOA_5; // 2E White CKP

    setCommonNTCSensor(&engineConfiguration->clt, 2700);
    setCommonNTCSensor(&engineConfiguration->iat, 2700);

    // todo#2108     engineConfiguration->tps1_1AdcChannel = EFI_ADC_3; //Frankenstein: inp2
    // todo#2108 engineConfiguration->vbattAdcChannel = EFI_ADC_0; //Frankenstein: inp5
    // todo#2108 engineConfiguration->clt.adcChannel = EFI_ADC_11;
    // todo#2108 engineConfiguration->iat.adcChannel = EFI_ADC_13;

    /**
     * TPS 0% 0.9v
     * TPS 100% 2.34v
     */
    engineConfiguration->tpsMin = convertVoltageTo10bitADC(1.250);
    engineConfiguration->tpsMax = convertVoltageTo10bitADC(4.538);

    // Stepper logic:
    // todo#2108engineConfiguration->idle.stepperDirectionPin = GPIO_UNASSIGNED; // todo#2108 GPIOB_10;
    // todo#2108engineConfiguration->idle.stepperStepPin = GPIO_UNASSIGNED; // todo#2108 GPIOB_15;
    // todo#2108engineConfiguration->stepperEnablePin = GPIO_UNASSIGNED; // todo#2108 GPIOB_14;
    // todo#2108engineConfiguration->idleStepperReactionTime = 10;
    // todo#2108engineConfiguration->idleStepperTotalSteps = 150;

    // todo#2108engineConfiguration->useStepperIdle = false;

	// set injection_pin_mode 0
	engineConfiguration->injectionPinMode = OM_DEFAULT;

	// todo#2108engineConfiguration->canWriteEnabled = true;
	// todo#2108engineConfiguration->canReadEnabled = false;
	// todo#2108engineConfiguration->canNbcType = CAN_BUS_MAZDA_RX8;


    setAlgorithm(LM_SPEED_DENSITY PASS_CONFIG_PARAMETER_SUFFIX);

    // todo: make this official Frankenstein joystick?
    engineConfiguration->joystickCenterPin = GPIO_UNASSIGNED; // todo#2108 GPIOD_8;
    engineConfiguration->joystickAPin = GPIO_UNASSIGNED; // todo#2108 GPIOD_10;
    engineConfiguration->joystickBPin = GPIO_UNASSIGNED;
    engineConfiguration->joystickCPin = GPIO_UNASSIGNED;
    engineConfiguration->joystickDPin = GPIO_UNASSIGNED; // todo#2108 GPIOD_11;

}
