NAME := gather_weather_data

$(NAME)_SOURCES := gather_weather_data.c

$(NAME)_COMPONENTS := protocols/MQTT
                      
VALID_PLATFORMS	:= CYW943907AEVAL1F_WW101 CYW943907AEVAL1F        

WIFI_CONFIG_DCT_H := wifi_config_dct.h

# GLOBAL_DEFINES     := USE_SELF_SIGNED_TLS_CERT       