# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set (EXTRA_COMPONENT_DIRS "/home/wit/wertyq/xxWertyqCode/esp32/esp-aws/esp-aws-iot /home/wit/wertyq/xxWertyqCode/esp32/esp_apps/ha_sensor/components")
project(cpp_exceptions_example)
