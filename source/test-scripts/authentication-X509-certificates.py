# The Device app
import os
import time
import glob
import asyncio
import json
from azure.iot.device.aio import IoTHubDeviceClient
from azure.iot.device import Message, X509

BASE_PATH = "/home/pi/Downloads/Vito/certificates/rsa"
CERTIFICATES = [
    {"certFile": BASE_PATH + "/temperature-sensor-1/temperature-sensor-1-public.pem", "keyFile": BASE_PATH + "/temperature-sensor-1/temperature-sensor-1-private.pem", "pass": "temp123"},
    # {"certFile": BASE_PATH + "/temperature-sensor-2/temperature-sensor-2-public.pem", "keyFile": BASE_PATH + "/temperature-sensor-2/temperature-sensor-2-private.pem", "pass": "temp123"},
]

# Azure communication methods
async def create_client(cert_file, key_file, pass_phrase, device_id):
    hostname = "Arduino-IoT-Hub-Temperature.azure-devices.net"
    
    # Instantiate X509 Certificate for the authentication
    print("Instantiate X509 Certificate object, for {0}, with the pre-generated key pair from the X509 certificate files.".format(device_id))
    x509 = X509(
        cert_file=cert_file,
        key_file=key_file,
        pass_phrase=pass_phrase,
    )
    
    # Instantiate client. This will throw an error if the certificate data is incorrect
    print("Send the public data of the {0} certificate to the CA and the IoT Hub.".format(device_id))
    print("The Azure CA and the IoT Hub client will very the device's certificate and, if everything is correct, it will instantiate the client.")
    device_client = IoTHubDeviceClient.create_from_x509_certificate(
        hostname=hostname, device_id=device_id, x509=x509
    )
    
    # Connect the client.
    print("Establish the secure client connection between {0} and its Digital Twin.".format(device_id))
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
    # for certificate in CERTIFICATES:
    try:
        # The device that has been created on the portal using X509 CA signing or Self signing capabilities
        # The certificate file should be with the same name as the device
        # device_id = os.path.basename(certificate["certFile"]).strip('-public.pem.pfx')
        device_id = os.path.basename(CERTIFICATES[0]["certFile"]).strip('-public.pem.pfx')
        print("Connecting to device {} ...".format(device_id))
        # temp_client = await create_client(certificate["certFile"], certificate["keyFile"], certificate["pass"], device_id)
        temp_client = await create_client(CERTIFICATES[0]["certFile"], CERTIFICATES[0]["keyFile"], CERTIFICATES[0]["pass"], device_id)
        clients.append(temp_client)
    except Exception as ex:
        print("Warning: Could not find device for the following certificate {}".format(CERTIFICATES[0]))
        print(ex)
        # continue
        
    return clients
    
if __name__ == '__main__':
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
    clients = asyncio.run(main())
    asyncio.run(close_clients(clients))
    print("done")
