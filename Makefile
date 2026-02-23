generate-ota:
	rm -f $(CURDIR)/public_images/OTA.bin
	cp $(CURDIR)/build/esp32-ota-github.bin $(CURDIR)/public_images/OTA.bin