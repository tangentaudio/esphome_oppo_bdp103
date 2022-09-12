import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from . import CONF_OPPO_103_ID, Oppo103

DEPENDENCIES = ["oppo_bdp103"]

CONF_PLAYBACK_STATUS = "sensor_playback_status"
CONF_DISC_TYPE = "sensor_disc_type"
CONF_AUDIO_TYPE = "sensor_audio_type"
CONF_INPUT_SOURCE = "sensor_input_source"
CONF_TIME_CODE = "sensor_time_code"

TYPES = [
    CONF_PLAYBACK_STATUS,
    CONF_DISC_TYPE,
    CONF_AUDIO_TYPE,
    CONF_INPUT_SOURCE,
    CONF_TIME_CODE
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_OPPO_103_ID): cv.use_id(Oppo103),

        cv.Required(CONF_PLAYBACK_STATUS):
            text_sensor.text_sensor_schema(),
        cv.Required(CONF_DISC_TYPE):
            text_sensor.text_sensor_schema(),
        cv.Required(CONF_AUDIO_TYPE):
            text_sensor.text_sensor_schema(),
        cv.Required(CONF_INPUT_SOURCE):
            text_sensor.text_sensor_schema(),
        cv.Required(CONF_TIME_CODE):
            text_sensor.text_sensor_schema(),

    }
)

async def to_code(config):
    comp = await cg.get_variable(config[CONF_OPPO_103_ID])

    if CONF_PLAYBACK_STATUS in config:
        conf = config[CONF_PLAYBACK_STATUS]
        sens = await text_sensor.new_text_sensor(conf)
        cg.add(comp.set_playback_status_text_sensor(sens))

    if CONF_DISC_TYPE in config:
        conf = config[CONF_DISC_TYPE]
        sens = await text_sensor.new_text_sensor(conf)
        cg.add(comp.set_disc_type_text_sensor(sens))

    if CONF_AUDIO_TYPE in config:
        conf = config[CONF_AUDIO_TYPE]
        sens = await text_sensor.new_text_sensor(conf)
        cg.add(comp.set_audio_type_text_sensor(sens))

    if CONF_INPUT_SOURCE in config:
        conf = config[CONF_INPUT_SOURCE]
        sens = await text_sensor.new_text_sensor(conf)
        cg.add(comp.set_input_source_text_sensor(sens))

    if CONF_TIME_CODE in config:
        conf = config[CONF_TIME_CODE]
        sens = await text_sensor.new_text_sensor(conf)
        cg.add(comp.set_time_code_text_sensor(sens))
