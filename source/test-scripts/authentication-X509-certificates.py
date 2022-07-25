# The Device app
import os
import time
import glob
import asyncio
import json
from azure.iot.device.aio import IoTHubDeviceClient
from azure.iot.device import Message, X509

BASE_PATH = "/home/pi/Downloads/Vito/certificates"
CERTIFICATES = [
    {"certFile": BASE_PATH + "/temperature-sensor-1/temperature-sensor-1-public.pem", "keyFile": BASE_PATH + "/temperature-sensor-1/temperature-sensor-1-private.pem", "pass": "temp123"},
    {"certFile": BASE_PATH + "/temperature-sensor-2/temperature-sensor-2-public.pem", "keyFile": BASE_PATH + "/temperature-sensor-2/temperature-sensor-2-private.pem", "pass": "temp123"},
]

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

    return device_client

async def close_clients(clients):
    if (len(clients) == 0):
        return
    
    for device_client in clients:
        await device_client.shutdown()

# main method and run app
async def main():
    clients = []
    for certificate in CERTIFICATES:
        try:
            # The device that has been created on the portal using X509 CA signing or Self signing capabilities
            # The certificate file should be with the same name as the device
            device_id = os.path.basename(certificate["certFile"]).strip('-public.pem.pfx')
            temp_client = await create_client(certificate["certFile"], certificate["keyFile"], certificate["pass"], device_id)
            clients.append(temp_client)
        except:
            print("Warning: Could not find device for the following certificate {}".format(certificate))
            continue
        
    return clients
    
if __name__ == '__main__':
    # Use the following loop logic if want to get average runtime for 10 rounds
	# rounds = 10
    # total_seconds = 0
    # for i in range(rounds):
    #    start_time = time.time()
    #    clients = asyncio.run(main())
    #    execution_time = time.time() - start_time
    #    total_seconds += execution_time
    #    asyncio.run(close_clients(clients))
        
    # print("done")
    # print("Average execution time is: " + str(total_seconds / rounds))
	
	# Comment this part if using the loop above
	clients = asyncio.run(main())
	asyncio.run(close_clients(clients))
	print("done")
