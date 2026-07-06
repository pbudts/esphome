from esphome import pins
import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import ICON_PERCENT, STATE_CLASS_MEASUREMENT, UNIT_PERCENT

CONF_PIN_SEC_PULSE = "pin_sec_pulse"
CONF_PIN_CLOCK = "pin_clock"
CONF_PIN_DATA = "pin_data"

jaga_ventilation_ns = cg.esphome_ns.namespace("jaga_ventilation")
JagaVentilationSensor = jaga_ventilation_ns.class_(
    "JagaVentilationSensor", sensor.Sensor, cg.PollingComponent
)

CONFIG_SCHEMA = cv.Schema(
    {
        sensor.sensor_schema(
            JagaVentilationSensor,
            unit_of_measurement=UNIT_PERCENT,
            icon=ICON_PERCENT,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
        )
        .extend(
            {
                cv.Required(CONF_PIN_SEC_PULSE): cv.All(
                    pins.internal_gpio_input_pin_schema
                )
            }
        )
        .extend(
            {cv.Required(CONF_PIN_CLOCK): cv.All(pins.internal_gpio_input_pin_schema)}
        )
        .extend(
            {cv.Required(CONF_PIN_DATA): cv.All(pins.internal_gpio_input_pin_schema)}
        )
        .extend(cv.polling_component_schema("5s"))
    }
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)

    pin_sec_pulse = await cg.gpio_pin_expression(config[CONF_PIN_SEC_PULSE])
    cg.add(var.set_pin_sec_pulse(pin_sec_pulse))
    pin_clock = await cg.gpio_pin_expression(config[CONF_PIN_CLOCK])
    cg.add(var.set_pin_clock(pin_clock))
    pin_data = await cg.gpio_pin_expression(config[CONF_PIN_DATA])
    cg.add(var.set_pin_data(pin_data))
