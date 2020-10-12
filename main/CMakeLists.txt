idf_component_register(SRCS "cpp_pthread.cpp"
							"./code/spifs.cpp"
							"./code/espfile.cpp"
							"./code/wifi.cpp"
							"./code/mqttclient.cpp"
							"./code/aws/aws.cpp"
							"./code/aws/awspublishmessage.cpp"
							"./code/aws/awsconnection.cpp"
							"./code/drivers/ds18b20.cpp"
							"./code/drivers/bus/onewirebus.cpp"
							"./code/drivers/bus/i2cbus.cpp"
                    INCLUDE_DIRS "."
                                 "./code"
                                 "./code/aws"
                                 "./code/drivers"
                                 "./code/drivers/bus")
  
component_compile_options(-std=c++14)                 
spiffs_create_partition_image(config  ./data FLASH_IN_PROJECT)