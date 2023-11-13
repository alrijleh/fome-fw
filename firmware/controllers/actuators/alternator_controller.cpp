/**
 * @file    alternator_controller.cpp
 * @brief   alternator controller - some newer vehicles control alternator with ECU
 *
 * @date Apr 6, 2014
 * @author Dmitry Sidin
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#include "pch.h"

#if EFI_ALTERNATOR_CONTROL
#include "alternator_controller.h"
#include "efi_pid.h"
#include "local_version_holder.h"
#include "periodic_task.h"

#if defined(HAS_OS_ACCESS)
#error "Unexpected OS ACCESS HERE"
#endif /* HAS_OS_ACCESS */

static SimplePwm alternatorControl("alt");
static Pid alternatorPid(&persistentState.persistentConfiguration.engineConfiguration.alternatorControl);

static percent_t currentAltDuty;

static bool shouldResetPid = false;

static void pidReset() {
	alternatorPid.reset();
}

void AlternatorController::onFastCallback() {
	if (!isBrainPinValid(engineConfiguration->alternatorControlPin)) {
		return;
	}

#if ! EFI_UNIT_TEST
	if (shouldResetPid) {
		pidReset();
		shouldResetPid = false;
	}
#endif

	// this block could be executed even in on/off alternator control mode
	// but at least we would reflect latest state
#if EFI_TUNER_STUDIO
	alternatorPid.postState(engine->outputChannels.alternatorStatus);
#endif /* EFI_TUNER_STUDIO */

	update();
}

expected<float> AlternatorController::getSetpoint() {
	bool alternatorShouldBeEnabledAtCurrentRpm = Sensor::getOrZero(SensorType::Rpm) > engineConfiguration->cranking.rpm;

	if (!engineConfiguration->isAlternatorControlEnabled || !alternatorShouldBeEnabledAtCurrentRpm) {
		return unexpected;
	}

	return engineConfiguration->targetVBatt;
}

expected<float> AlternatorController::observePlant() const {
	return Sensor::get(SensorType::BatteryVoltage);
}

expected<percent_t> AlternatorController::getOpenLoop(float /*target*/) {
	// see "idle air Bump for AC" comment
	return engine->module<AcController>().unmock().acButtonState ? engineConfiguration->acRelayAlternatorDutyAdder : 0;
}

expected<percent_t> AlternatorController::getClosedLoop(float setpoint, float observation) {
	return alternatorPid.getOutput(setpoint, observation, FAST_CALLBACK_PERIOD_MS / 1000.0f);
}

void AlternatorController::setOutput(expected<percent_t> outputValue) {
	if (outputValue) {
		alternatorControl.setSimplePwmDutyCycle(PERCENT_TO_DUTY(outputValue.Value));
	} else {
		// turn off in case of fault and reset
		alternatorPid.reset();
		alternatorControl.setSimplePwmDutyCycle(0);
	}
}

void AlternatorController::onConfigurationChange(engine_configuration_s const * previousConfiguration) {
	shouldResetPid = !alternatorPid.isSame(&previousConfiguration->alternatorControl);
}

void initAlternatorCtrl() {
	if (!isBrainPinValid(engineConfiguration->alternatorControlPin))
		return;

	startSimplePwm(&alternatorControl,
				"Alternator control",
				&engine->executor,
				&enginePins.alternatorPin,
				engineConfiguration->alternatorPwmFrequency, 0);
}

#endif /* EFI_ALTERNATOR_CONTROL */
