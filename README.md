



Testing packets

cat ./packet_set_leds | nc -u $(esp32_wifi_ip) 17777 | hexdump
