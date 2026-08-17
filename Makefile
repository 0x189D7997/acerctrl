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
	@rm -f $(PREFIX)/lib/libAcerHIDHardware.so*
	@rm -f $(PREFIX)/lib/libAcerHIDRGB.so*
	@rm -f $(PREFIX)/bin/acerctrld
	@rm -f $(PREFIX)/bin/acerctrl-cli
	@rm -f $(PREFIX)/bin/acerctrl-systray
	@rm -f /etc/systemd/system/acerctrld.service
	@rm -fr /var/lib/acerctrl/
