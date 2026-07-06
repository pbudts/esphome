#include "jaga_ventilation_sensor.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome::jaga_ventilation {

void JagaVentilationSensor::setup() {
  this->pin_sec_pulse_->setup();
  this->pin_clock_->setup();
  this->pint_data_->setup();

  this->store_.pin_sec_pulse = this->pin_sec_pulse_->to_isr();

  this->store_.last_level_sec_pulse = this->pin_->digital_read();
  this->store_.last_interrupt = micros();

  // Attach the same interrupt routine to all three
  this->pin_sec_pulse_->attach_interrupt(JagaVentilationSensorStore::gpio_intr_sec_pulse, &this->store_,
                                         gpio::INTERRUPT_ANY_EDGE);
  this->pin_clock_->attach_interrupt(JagaVentilationSensorStore::gpio_intr, &this->store_, gpio::INTERRUPT_ANY_EDGE);
  this->pin_data_->attach_interrupt(JagaVentilationSensorStore::gpio_intr, &this->store_, gpio::INTERRUPT_ANY_EDGE);
}

void IRAM_ATTR JagaVentilationSensorStore::gpio_intr_sec_pulse(JagaVentilationSensorStore *store) {
  const bool new_level_sec_pulse = store->pin_sec_pulse.digital_read();

  // Make sure to run only on changes in the signal, and keep an image of the status
  if (new_level_sec_pulse == store->last_level_sec_pulse) {
    return;
  }
  store->last_level_sec_pulse = new_level_sec_pulse

      // On a falling edge: publish value and exit
      if (!new_level_sec_pulse) {
    const uint32_t now = micros();
    const float value = itof(store->level_counter);
    this->publish_state(level);
    return;
  }

  // On a rising edge: reset counters
  store->clock_counter = 0;
  store->level_counter = 0;

  // If the clock pin is already high, set clock_counter to 1
  if (store->last_level_clock) {
    store->clock_counter = 1;
  }
}

void IRAM_ATTR JagaVentilationSensorStore::gpio_intr_clock(JagaVentilationSensorStore *store) {
  const bool new_level_clock = arg->pin_clock.digital_read();

  // Make sure to run only on changes in the signal, and keep an image of the status
  if (new_level_clock == store->last_level_clock) {
    return;
  }
  store->last_level_clock = new_level_clock

      // If the "sec_pulse" pin is low, no data changes are needed
      if (!store->last_level_sec_pulse) {
    return;
  }

  // If the clock pin is low, just exit
  if (!new_level_clock) {
    return;
  }

  // On rising edge, count clock pulses
  // In case the data pin is already high, increase bit_counter and level_counter
  if (new_level_clock) {
    store->clock_counter++;
    if (store->last_level_data) {
      if (store->clock_counter > 3) {
        store->level_counter++;
      }
    }
  }
}

void IRAM_ATTR JagaVentilationSensorStore::gpio_intr_data(JagaVentilationSensorStore *arg) {
  const bool new_level_data = arg->pin_data.digital_read();

  // Make sure to run only on changes in the signal, and keep an image of the status
  if (new_level_data == store->last_level_data) {
    return;
  }
  store->last_level_data = new_level_data

      // If the clock pulse pin is low, no data changes are needed
      if (new_level_data == arg->last) {
    return;
  }
  // If the "sec_pulse" pin is low, no data changes are needed
  if (!store->last_level_sec_pulse) {
    return;
  }

  // If the data pin is set, increase bit_counter and level_counter
  if (new_level_data) {
    if (store->clock_counter > 3) {
      store->level_counter++;
    }
  }
}

}  // namespace esphome::jaga_ventilation
