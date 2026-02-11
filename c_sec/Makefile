CC = gcc
CFLAGS = -Wall -Wextra -Icommon -Imock_sensor -Igateway

MOCK_SENSOR_SRC = main/mock_sensor_main.c mock_sensor/mock_sensor.c common/protocol.c
GATEWAY_SRC = main/gateway_main.c gateway/gateway.c common/protocol.c

all: mock_sensor_app gateway_app

mock_sensor_app:
	$(CC) $(MOCK_SENSOR_SRC) $(CFLAGS) -o mock_sensor_app

gateway_app:
	$(CC) $(GATEWAY_SRC) $(CFLAGS) -o gateway_app

clean:
	rm -f mock_sensor_app gateway_app
