idf_component_register(SRCS "cpp_pthread.cpp"
							"./code/spifs.cpp"
							"./code/espfile.cpp"
							"./code/wifi.cpp"
							"./code/mqttclient.cpp"
							"./code/aws/awsmessage.cpp"
							"./code/aws/awsconnection.cpp"
							"./code/aws/awsshadow.cpp"
							"./code/drivers/ds18b20.cpp"
							"./code/drivers/bus/onewirebus.cpp"
							"./code/drivers/bus/i2cbus.cpp"
                    INCLUDE_DIRS "."
                                 "./code"
                                 "./code/aws"
                                 "./code/drivers"
                                 "./code/drivers/bus")
  
component_compile_options(-std=c++17)                 
spiffs_create_partition_image(config  ./data FLASH_IN_PROJECT)