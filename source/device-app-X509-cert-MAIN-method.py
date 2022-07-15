# The Device app
import os
import time
import glob
import asyncio
import json
from azure.iot.device.aio import IoTHubDeviceClient
from azure.iot.device import Message, X509

SLEEP_TIME = 15

device_folders = glob.glob(base_dir + '28*')
        
# main method and run app
async def main():
    print("IoT Hub - Simulating Device to Digital Twin Communication ...")

    clients = []
    for certificate in CERTIFICATES:
        try:
            # The device that has been created on the portal using X509 CA signing or Self signing capabilities
            # The certificate file should be with the same name as the device
            device_id = os.path.basename(certificate["certFile"]).strip('-public.pem.pfx')
            print("Connecting to device {} ...".format(device_id))
            temp_client = await create_client(certificate["certFile"], certificate["keyFile"], certificate["pass"], device_id)
            clients.append(temp_client)
        except:
            print("Warning: Could not authenticate or find device for the following certificate {}".format(certificate))
            continue
			
	if (len(clients) == 0):
		print("Error: The devices failed to authenticate!")
		await close_clients(clients)
		return;

    if (len(clients) != len(device_folders)):
        print("Error: The number of device clients mismatch the number connected physical devices!")
        await close_clients(clients)
        return
            
    print("IoTHubDeviceClient waiting for commands, press Ctrl-C to exit")

    try:
        # Update reported properties with WiFi information and send telemetry message
        print("Sending data as reported property...")

        # Update the temperature until the program exit
        while True:
            client_index = 0
            for folder in device_folders:
                temp_c, temp_f = read_sensors_data(folder)
                if (temp_c != None):
                    print("temperature(C): {}, temperature(F): {}".format(temp_c, temp_f))
                
                    reported_patch = {"currentTemperatureC": temp_c, "currentTemperatureF": temp_f, "connectivity": "WiFi"}
                    await clients[client_index].patch_twin_reported_properties(reported_patch)
                    await send_telemetry_message(clients[client_index], reported_patch)
                    client_index += 1

            print("The reported properties of the sensors are updated")
            time.sleep(SLEEP_TIME)
    except KeyboardInterrupt:
        print("IoT Hub Device Twin device sample stopped")
    finally:
        # Graceful exit and shut down all clients
        print("Shutting down IoT Hub Client")
        await close_clients(clients)
        clients = []

if __name__ == '__main__':
    asyncio.run(main())
    print("done")
