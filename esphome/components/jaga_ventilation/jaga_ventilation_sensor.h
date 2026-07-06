#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome::jaga_ventilation {

/// Store data in a class that doesn't use multiple-inheritance (vtables in flash)
struct JagaVentilationSensorStore {
  volatile bool last_level_sec_pulse{false};  // used for edge detection on sec_pulse pin
  volatile bool last_level_clock{false};      // used for edge detection on clock pin
  volatile bool last_level_data{false};       // used for edge detection on data pin

  volatile uint32_t clock_counter{0};  // Counts clock pulses (1 to 8)
  volatile uint32_t bit_counter{0};    // As soon as a data bit is high, increased to match clock pulses
  volatile uint32_t level_counter{0};  // Level-counter (high bits starting from the 4th clock cycle)

  ISRInternalGPIOPin pin_sec_pulse;
  ISRInternalGPIOPin pin_clock;
  ISRInternalGPIOPin pin_data;

  static void gpio_intr_sec_pulse(DutyCycleSensorStore *arg);
  static void gpio_intr_clock(DutyCycleSensorStore *arg);
  static void gpio_intr_data(DutyCycleSensorStore *arg);
};

class JagaVentilationSensor : public sensor::Sensor, public PollingComponent {
 public:
  void set_pin_sec_pulse(InternalGPIOPin *pin) { pin_sec_pulse_ = pin; }
  void set_pin_clock(InternalGPIOPin *pin) { pin_clock_ = pin; }
  void set_pin_data(InternalGPIOPin *pin) { pin_data_ = pin; }

  void setup() override;
  void dump_config() override;
  void update() override;

 protected:
  InternalGPIOPin *pin_sec_pulse_;
  InternalGPIOPin *pin_clock_;
  InternalGPIOPin *pin_data_;

  JagaVentilationSensorStore store_{};
  uint32_t last_update_{0};
};

}  // namespace esphome::jaga_ventilation
