module;

#include <algorithm>
#include <filesystem>
#include <format>
#include <fcntl.h>
#include <fstream>
#include <linux/hidraw.h>
#include <unordered_map>
#include <print>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

export module AcerHIDRGB;

export constexpr int RGB_HID_DEVICE_VID          = 0x0CF2;
export constexpr int RGB_HID_DEVICE_PID          = 0x5130;

export constexpr uint8_t RGB_FEATURE_ID          = 0xA4;

export constexpr uint8_t RGB_KEYBOARD_ID         = 0x21;
export constexpr uint8_t RGB_PROFILE_BUTTON_ID   = 0x65;
export constexpr uint8_t RGB_LID_ID              = 0x83;

export constexpr uint8_t RGB_EFFECT_OFF          = 0x01;
export constexpr uint8_t RGB_EFFECT_STATIC       = 0x02;
export constexpr uint8_t RGB_EFFECT_BREATHING    = 0x04;
export constexpr uint8_t RGB_EFFECT_NEON         = 0x05;
export constexpr uint8_t RGB_EFFECT_MODE_CHANGE  = 0x06;
export constexpr uint8_t RGB_EFFECT_WAVE         = 0x07;
export constexpr uint8_t RGB_EFFECT_RIPPLE       = 0x08;
export constexpr uint8_t RGB_EFFECT_ZOOM         = 0x09;
export constexpr uint8_t RGB_EFFECT_SNAKE        = 0x0A;
export constexpr uint8_t RGB_EFFECT_DISCO        = 0x0B;

export constexpr uint8_t RGB_DIRECTION_NONE      = 0x00;
export constexpr uint8_t RGB_DIRECTION_RIGHT     = 0x01;
export constexpr uint8_t RGB_DIRECTION_LEFT      = 0x02;

namespace acerhidrgb {
	std::string hidraw_file;

    struct hidraw_fd {
        void operator()(int* fd) const {
            if (fd && *fd != -1) {
                close(*fd);
            }
            delete fd;
        }
    };

    std::unique_ptr<int, hidraw_fd> hidraw_fd;

    const std::unordered_map<std::string_view, uint8_t> device_map = {
        { "keyboard",        RGB_KEYBOARD_ID},
        { "profile_button",  RGB_PROFILE_BUTTON_ID },
        { "lid",             RGB_LID_ID}
    };

    const std::unordered_map<std::string_view, uint8_t> effect_map = {
        { "off",          RGB_EFFECT_OFF },
        { "static",       RGB_EFFECT_STATIC },
        { "breathing",    RGB_EFFECT_BREATHING },
        { "neon",         RGB_EFFECT_NEON },
        { "mode_change",  RGB_EFFECT_MODE_CHANGE },
        { "wave",         RGB_EFFECT_WAVE },
        { "ripple",       RGB_EFFECT_RIPPLE },
        { "zoom",         RGB_EFFECT_ZOOM },
        { "snake",        RGB_EFFECT_SNAKE },
        { "disco",        RGB_EFFECT_DISCO }
    };

	bool findDevice() {
		std::string sysfs_base_path = "/sys/class/hidraw";

		for (const auto& entry : std::filesystem::directory_iterator(sysfs_base_path)) {
			std::string uevent_path = entry.path().string() + "/device/uevent";
			std::ifstream uevent_file(uevent_path);
			std::string line;

			while(std::getline(uevent_file, line)) {
				if (line.find("HID_ID") != std::string::npos) {
					if (line.find(std::format("{:04X}", RGB_HID_DEVICE_VID)) != std::string::npos && line.find(std::format("{:04X}", RGB_HID_DEVICE_PID)) != std::string::npos) {
						hidraw_file = "/dev/" + entry.path().filename().string();
						return true;
					}
				}
			}
		}

		return false;
	}

	bool hidSetFeature(const std::vector<uint8_t>& bytes) {
		if (!hidraw_fd || *hidraw_fd < 0) {
			std::println("[ERR] {} is not open!", hidraw_file);
			return false;
		}

		int res = ioctl(*hidraw_fd, HIDIOCSFEATURE(bytes.size()), bytes.data());
		if (res < 0) {
			std::println("[ERR] HIDIOCSFEATURE failed!");
			return false;
		}

		return true;
	}

	void rgbSendFeature(uint8_t dev, uint8_t effect, uint8_t brightness, uint8_t speed, uint8_t direction, uint8_t r, uint8_t g, uint8_t b, uint8_t zone) {
		std::vector<uint8_t> hid_data = { RGB_FEATURE_ID, dev, effect, brightness, speed, direction, r, g, b, zone, 0x00 };

		hidSetFeature(hid_data);
	}

	bool rgbValidate(uint8_t device, uint8_t effect, uint8_t brightness, uint8_t speed, uint8_t direction, uint8_t zone) {
        auto device_good = std::ranges::any_of(device_map, [device](const auto& pair) {
            return pair.second == device;
        });

        if (!device_good) {
            std::println("[ERR] Invalid RGB device {:x}!", device);
            return false;
        }

        auto effect_good = std::ranges::any_of(effect_map, [effect](const auto& pair) {
            return pair.second == effect;
        });

        if (!effect_good) {
            std::println("[ERR] Invalid RGB effect {:x}!", effect);
            return false;
        }

		if (brightness > 100) {
			std::println("[ERR] Invalid RGB brightness {:d}!", brightness);
			return false;
		}

		if (speed > 9) {
			std::println("[ERR] Invalid RGB speed {:d}!", speed);
			return false;
		}

		if (direction > 2) {
			std::println("[ERR] Invalid RGB direction {:d}!", direction);
			return false;
		}

		/* TODO: validate zone*/

		return true;
	}

	bool rgbSet(uint8_t dev, uint8_t effect, uint8_t brightness, uint8_t speed, uint8_t direction, uint8_t r, uint8_t g, uint8_t b, uint8_t zone) {
		if (!rgbValidate(dev, effect, brightness, speed, direction, zone)) {
			std::println("[ERR] RGB validation failed!");
			return false;
		}

		rgbSendFeature(dev, effect, brightness, speed, direction, r, g, b, zone);

		return true;
	}
}

export namespace acerhidrgb {
	bool rgbSet(std::string_view in_device, std::string_view in_effect, uint8_t brightness, uint8_t speed, uint8_t direction, uint8_t r, uint8_t g, uint8_t b, uint8_t zone) {
		uint8_t dev = 0x00;
		uint8_t effect = 0x00;

        if (auto it = device_map.find(in_device); it != device_map.end()) {
            dev = it->second;
        } else {
            std::println("[ERR] Invalid device {}!", in_device);
            return false;
        }

        if (auto it = effect_map.find(in_effect); it != effect_map.end()) {
            effect = it->second;
        } else {
            std::println("[ERR] Invalid effect {}!", in_effect);
            return false;
        }

		return rgbSet(dev, effect, brightness, speed, direction, r, g, b, zone);
	}

	bool init() {
		if (!findDevice()) {
			return false;
		}

        int fd = open(hidraw_file.c_str(), O_RDWR);
        if (fd < 0) {
            return false;
        }

        hidraw_fd.reset(new int(fd));

		std::println("[INFO] Using {} for RGB features", hidraw_file);

		return true;
	}
}
