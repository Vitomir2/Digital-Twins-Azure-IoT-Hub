# The Back-end app
import os
import sys
from time import sleep
from azure.iot.hub import IoTHubRegistryManager
from azure.iot.hub.models import Twin, TwinProperties, QuerySpecification, QueryResult

IOTHUB_CONNECTION_STRING = os.getenv("IOTHUB_CONNECTION_STRING")
IOTHUB_DEVICE_IDS = [os.getenv("IOTHUB_SENSOR_1_ID"), os.getenv("IOTHUB_SENSOR_1_ID")]

def iothub_service_sample_run():
    try:
        iothub_registry_manager = IoTHubRegistryManager(IOTHUB_CONNECTION_STRING)

        new_tags = {
                'location' : {
                    'region' : 'BG',
                    'plant' : 'Sofia'
                }
            }
        
        for device_id in IOTHUB_DEVICE_IDS:
            print(device_id)
            twin = iothub_registry_manager.get_twin(device_id)
            twin_patch = Twin(tags=new_tags, properties= TwinProperties(desired={'maxTemperature' : 30}))
            twin = iothub_registry_manager.update_twin(device_id, twin_patch, twin.etag)

        # Add a delay to account for any latency before executing the query
        sleep(1)

        query_spec = QuerySpecification(query="SELECT * FROM devices WHERE tags.location.plant = 'Sofia'")
        query_result = iothub_registry_manager.query_iot_hub(query_spec, None, 100)
        print("Devices in Sofia plant: {}".format(', '.join([twin.device_id for twin in query_result.items])))

        print()

        query_spec = QuerySpecification(query="SELECT * FROM devices WHERE tags.location.plant = 'Sofia' AND properties.reported.connectivity = 'WiFi'")
        query_result = iothub_registry_manager.query_iot_hub(query_spec, None, 100)
        print("Devices in Sofia plant using WiFi network: {}".format(', '.join([twin.device_id for twin in query_result.items])))

    except Exception as ex:
        print("Error: Unexpected error {0}".format(ex))
        return
    except KeyboardInterrupt:
        print("IoT Hub Device Twin service sample stopped")

if __name__ == '__main__':
    print("Starting the Python IoT Hub Device Twin service sample...")
    print()

    iothub_service_sample_run()