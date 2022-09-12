import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ICON, ICON_POWER
from . import CONF_OPPO_103_ID, Oppo103

DEPENDENCIES = ["oppo_bdp103"]

CONF_SENSOR_POWER = "sensor_power"

TYPES = [
    CONF_SENSOR_POWER,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_OPPO_103_ID): cv.use_id(Oppo103),

        cv.Required(CONF_SENSOR_POWER):
            binary_sensor.binary_sensor_schema(),
    }
)

async def to_code(config):
    comp = await cg.get_variable(config[CONF_OPPO_103_ID])

    if CONF_SENSOR_POWER in config:
        conf = config[CONF_SENSOR_POWER]
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(comp.set_power_binary_sensor(sens))
