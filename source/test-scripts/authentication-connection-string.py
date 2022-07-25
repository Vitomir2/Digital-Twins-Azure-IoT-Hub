# The Device app
import os
import time
import glob
import numpy as np
from azure.iot.device import IoTHubModuleClient

# Azure constants
IOTHUB_DEVICE_CONNECTION_STRINGS = ["HostName=Arduino-IoT-Hub-Temperature.azure-devices.net;DeviceId=temp-sensor-1;SharedAccessKey=Y6pgvS/Fp1rSVfkhymOTOGe9UjRU9bUgrUUbz2l2tU0=", "HostName=Arduino-IoT-Hub-Temperature.azure-devices.net;DeviceId=temp-sensor-2;SharedAccessKey=c1FlVKagDkDdUEimxUDln4hngrOg6kJt/I7EtdoVycw="]

# Azure connection methods
def create_client(connection_string):
    # Instantiate client
    device_client = IoTHubModuleClient.create_from_connection_string(connection_string, websockets=True)

    return device_client

def main():
    clients = []
    for connectionString in IOTHUB_DEVICE_CONNECTION_STRINGS:
        try:
            # get the device id from the connection string
            deviceIdPos = connectionString.find('DeviceId=')
            
            temp_client = create_client(connectionString)
            clients.append(temp_client)
        except:
            print("Warning: Could not find device with string " + connectionString)
            continue

if __name__ == '__main__':
    # Use the following loop logic if want to get average runtime for 10 rounds
	# rounds = 10
    # total_seconds = 0
    # for i in range(rounds):
    #    start_time = time.time()
    #    main()
    #    execution_time = time.time() - start_time
    #    total_seconds += execution_time
        
    # print("done")
    # print("Average execution time is: " + str(total_seconds / rounds))
	
	# Comment this part if using the loop above
    main()
    print("done")