module;

#include <filesystem>
#include <format>
#include <fcntl.h>
#include <fstream>
#include <linux/hidraw.h>
#include <poll.h>
#include <print>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

export module AcerHIDHardware;

export constexpr int HW_HID_DEVICE_VID       = 0x1025;
export constexpr int HW_HID_DEVICE_PID       = 0x174B;

export constexpr uint8_t HW_USAGE_MODE_TURBO        = 0x00;
export constexpr uint8_t HW_USAGE_MODE_PERFORMANCE  = 0x01;
export constexpr uint8_t HW_USAGE_MODE_NORMAL       = 0x02;
export constexpr uint8_t HW_USAGE_MODE_QUIET        = 0x03;
export constexpr uint8_t HW_USAGE_MODE_ECO          = 0x04;
export constexpr uint8_t HW_USAGE_MODE_ECO_PLUS     = 0x05;

namespace acerhidhw {
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

	uint8_t current_usage_mode = 0xff;

	bool findDevice() {
		std::string sysfs_base_path = "/sys/class/hidraw";

		for (const auto& entry : std::filesystem::directory_iterator(sysfs_base_path)) {
			std::string uevent_path = entry.path().string() + "/device/uevent";
			std::ifstream uevent_file(uevent_path);
			std::string line;

			while(std::getline(uevent_file, line)) {
				if (line.find("HID_ID") != std::string::npos) {
					if (line.find(std::format("{:X}", HW_HID_DEVICE_VID)) != std::string::npos && line.find(std::format("{:X}", HW_HID_DEVICE_PID)) != std::string::npos) {
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

	bool hidSetBatteryLimits(uint8_t status, uint8_t lower, uint8_t upper) {
		std::vector<uint8_t> hid_data(65, 0x00);

		hid_data[0] = 0xA0;
		hid_data[2] = 0xA0;
		hid_data[3] = 0x03;
		hid_data[4] = 0x0b;
		hid_data[5] = 0x01;
		hid_data[6] = 0x03;
		hid_data[7] = status;
		hid_data[8] = upper;
		hid_data[9] = lower;

		return hidSetFeature(hid_data);
	}

	bool hidSetKeyboardTimeout(uint8_t seconds) {
		std::vector<uint8_t> hid_data(65, 0x00);
        /* magic values obtained from Predator Sense */
		hid_data[0] = 0xA0;
		hid_data[2] = 0xA0;
		hid_data[3] = 0x0A;
		hid_data[5] = 0x01;
		hid_data[6] = 0x02;
		hid_data[7] = 0x01;
		hid_data[9] = 0x64;
		hid_data[11] = seconds;

		return hidSetFeature(hid_data);
	}
} /* namespace acerhidhw */

export namespace acerhidhw {
	bool setBatteryLimits(uint8_t status, uint8_t lower, uint8_t upper) {
		if (status != 0 && status != 1) {
			std::println("[ERR] Battery limit status is not valid!");
			return false;
		}

		std::println("[INFO] Setting battery limits {}-{}", lower, upper);
		return hidSetBatteryLimits(status, lower, upper);
	}

	bool keyboardTimeoutSet(uint8_t seconds) {
		return hidSetKeyboardTimeout(seconds);
	}

	bool setUsageMode(uint8_t usage_mode) {
		if (usage_mode > 0x05) {
			std::println("[ERR] Mode {:X} is not a valid usage mode!", usage_mode);
			return false;
		}

		std::println("[INFO] Setting mode {:X}", usage_mode);

		std::vector<uint8_t> hid_data(65, 0x00);
        /* magic values obtained from Predator Sense */
		hid_data[0] = 0xA0;
		hid_data[2] = 0xA0;
		hid_data[3] = 0x01;
		hid_data[5] = 0x01;
		hid_data[6] = usage_mode;

		if (hidSetFeature(hid_data)) {
			current_usage_mode = usage_mode;
			return true;
		}

		return false;
	}

	/* i dont know what the behavior is on windows so this may not be accurate */
	uint8_t cycleUsageMode() {
		switch (current_usage_mode) {
			case HW_USAGE_MODE_TURBO:
				setUsageMode(HW_USAGE_MODE_ECO);
				break;
			case HW_USAGE_MODE_PERFORMANCE:
				setUsageMode(HW_USAGE_MODE_TURBO);
				break;
			case HW_USAGE_MODE_NORMAL:
				setUsageMode(HW_USAGE_MODE_PERFORMANCE);
				break;
			case HW_USAGE_MODE_QUIET:
				setUsageMode(HW_USAGE_MODE_NORMAL);
				break;
			case HW_USAGE_MODE_ECO:
				setUsageMode(HW_USAGE_MODE_QUIET);
				break;
			default:
				setUsageMode(HW_USAGE_MODE_NORMAL);
				break;
		}

		return current_usage_mode;
	}

	bool waitForTurboButtonEvent() {
		if (!hidraw_fd || *hidraw_fd < 0) {
			std::println("[ERR] {} is not open!", hidraw_file);
			return false;
		}

		struct pollfd fds;
		fds.fd = *hidraw_fd;
		fds.events = POLLIN;

		unsigned char buf[64] = {0};
		for(;;) {
			int ret = poll(&fds, 1, -1);

			if (ret > 0 && fds.revents & POLLIN) {
				read(*hidraw_fd, buf, sizeof(buf));

				if (buf[0] == 0x04 && buf[1] == 0x85 && buf[2] == 0xff) {
					return true;
				}
			}
		}

		return false;
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

		std::println("[INFO] Using {} for hardware features", hidraw_file);

		return true;
	}
}
