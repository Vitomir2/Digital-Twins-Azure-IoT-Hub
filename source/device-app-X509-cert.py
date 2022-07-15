# The Device app
import os
import time
import glob
import asyncio
import json
from azure.iot.device.aio import IoTHubDeviceClient
from azure.iot.device import Message, X509

SLEEP_TIME = 15
BASE_PATH = "/home/pi/Downloads/Vito/certificates"
CERTIFICATES = [
    # {"certFile": BASE_PATH + "/test-device-auth/test-device-auth-public.pem", "keyFile": BASE_PATH + "/test-device-auth/test-device-auth-private.pem", "pass": "test123"},
    {"certFile": BASE_PATH + "/temperature-sensor-1/temperature-sensor-1-public.pem", "keyFile": BASE_PATH + "/temperature-sensor-1/temperature-sensor-1-private.pem", "pass": "temp123"},
    {"certFile": BASE_PATH + "/temperature-sensor-2/temperature-sensor-2-public.pem", "keyFile": BASE_PATH + "/temperature-sensor-2/temperature-sensor-2-private.pem", "pass": "temp123"},
]

# Sensor files params
os.system('modprobe w1-gpio')
os.system('modprobe w1-therm')

base_dir = '/sys/bus/w1/devices/'
device_folders = glob.glob(base_dir + '28*')

# Azure communication methods
async def create_client(cert_file, key_file, pass_phrase, device_id):
    hostname = "Arduino-IoT-Hub-Temperature.azure-devices.net"
    
    # Instantiate X509 Certificate for the authentication
    x509 = X509(
        cert_file=cert_file,
        key_file=key_file,
        pass_phrase=pass_phrase,
    )
    
    # Instantiate client
    device_client = IoTHubDeviceClient.create_from_x509_certificate(
        hostname=hostname, device_id=device_id, x509=x509
    )
    
    # Connect the client.
    await device_client.connect()

    # Define behavior for receiving twin desired property patches
    def twin_patch_handler(twin_patch):
        print("Twin patch received:")
        print(twin_patch)

    try:
        # Set handlers on the client
        device_client.on_twin_desired_properties_patch_received = twin_patch_handler
    except:
        # Clean up in the event of failure
        device_client.shutdown()

    return device_client

async def close_clients(clients):
    if (len(clients) == 0):
        return
    
    for device_client in clients:
        await device_client.shutdown()

async def send_telemetry_message(client, telemetry_msg):
    msg = Message(json.dumps(telemetry_msg))
    msg.content_encoding = "utf-8"
    msg.content_type = "application/json"
    await client.send_message(msg)

# Sensor data methods
def read_sensors_data(folder):
    device_file = folder + '/w1_slave'
    lines = read_raw_file(device_file)
    if (len(lines) == 0):
        return None, None
        
    while lines[0].strip()[-3:] != 'YES':
        time.sleep(0.2)
        lines = read_raw_file(device_file)

    equals_pos = lines[1].find('t=')
    if equals_pos == -1:
        return None, None

    temp_string = lines[1][equals_pos+2:]
    temp_c = float(temp_string) / 1000.0
    temp_f = temp_c * 9.0 / 5.0 + 32.0
    return temp_c, temp_f
        
def read_raw_file(file):
    f = open(file, 'r')
    lines = f.readlines()
    f.close()
    return lines
        
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
