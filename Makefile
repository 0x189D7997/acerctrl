PREFIX=/usr/local

.PHONY: all
all:
	@cmake -DCMAKE_BUILD_TYPE=Release -S . -B build -G Ninja
	@cmake --build build

.PHONY: clean
clean:
	@cmake --build build --target clean
	@rm -r ./build

.PHONY: install
install:
	@cmake --install ./build --config Release --prefix $(PREFIX) --strip -v
	@cp -v ./acerctrld.service /etc/systemd/system/

.PHONY: uninstall
uninstall:
	@rm $(PREFIX)/lib/libAcerHIDHardware.so*
	@rm $(PREFIX)/lib/libAcerHIDRGB.so*
	@rm $(PREFIX)/bin/acerctrld
	@rm $(PREFIX)/bin/acerctrl-cli
	@rm /etc/systemd/system/acerctrld.service
