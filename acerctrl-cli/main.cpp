#include <sys/socket.h>
#include <sys/un.h>
#include <print>
#include <unistd.h>
#include <vector>

std::string socket_path = "/run/acerctrl.sock";

void printHelp() {
	std::println("Usage: acerctrl-cli <function> <function arguments>");
	std::println("  set-rgb:            set RGB");
	std::println("  set-timeout:        set keyboard backlight timeout");
	std::println("  set-usage-mode:     set usage mode/performance profile");
	std::println("  set-battery-limits: set battery charging limits");
	std::println("  --help:             show this help");
	std::println("\n\nFunction Options:");
	std::println("  set-rgb --device <keyboard, lid, profile_button> --effect <off, static, breathing, neon, wave, ripple, zoom, snake, disco> --brightness <0-100> --rgb <red 0-255> <green 0-255> <blue 0-255> --zone <bitmask 0-15>");
	std::println("  set-timeout <timeout in seconds>");
	std::println("  set-usage-mode <0-5, 0 = turbo, 5 = eco+>");
	std::println("  set-battery-limits: --status <0 = off, 1 = on> --lower <lower limit> --upper <upper limit>");
}

void sendMsgToSocket(std::string msg) {
	int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	sockaddr_un addr {};
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socket_path.c_str(), socket_path.length());

	if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		std::println("Failed to connect to acerctrld! (is it running?)");
        close(sock);
		return;
	}

	write(sock, msg.c_str(), msg.length());

	close(sock);
}

void functionSetRgb(std::vector<std::string>& args) {
	std::string device = "0";
	std::string effect = "0";
	std::string brightness = "25";
	std::string speed = "0";
	std::string direction = "0";
	std::string r = "255";
	std::string g = "255";
	std::string b = "255";
	std::string zone = "15";

	for (auto it = args.begin(); it != args.end(); it++) {
		if (it->compare("--device") == 0 && std::next(it) != args.end()) {
			device = std::next(it)->c_str();
		} else if (it->compare("--effect") == 0 && std::next(it) != args.end()) {
			effect = std::next(it)->c_str();
		} else if (it->compare("--brightness") == 0 && std::next(it) != args.end()) {
			brightness = std::next(it)->c_str();
		} else if (it->compare("--speed") == 0 && std::next(it) != args.end()) {
			speed = std::next(it)->c_str();
		} else if (it->compare("--direction") == 0 && std::next(it) != args.end()) {
			direction = std::next(it)->c_str();
		} else if (it->compare("--rgb") == 0 && std::next(it) != args.end()) {
			r = std::next(it)->c_str();
            it++;
			g = std::next(it)->c_str();
			it++;
			b = std::next(it)->c_str();
		} else if (it->compare("--zone") == 0 && std::next(it) != args.end()) {
			zone = std::next(it)->c_str();
		}
	}

	std::string msg = std::format("SET_RGB {} {} {} {} {} {} {} {} {}", device, effect, brightness, speed, direction, r, g, b, zone);
	sendMsgToSocket(msg);
}

void functionSetKeyboardTimeout(std::string arg) {
	std::string msg = std::format("SET_TIMEOUT {}", arg);
	sendMsgToSocket(msg);
}

void functionSetBatteryLimits(std::vector<std::string> args) {
	std::string status = "1";
	std::string lower = "70";
	std::string upper = "80";

	for (auto it = args.begin(); it != args.end(); it++) {
		if (it->compare("--status") == 0 && std::next(it) != args.end()) {
			status = std::next(it)->c_str();
		} else if (it->compare("--lower") == 0 && std::next(it) != args.end()) {
			lower = std::next(it)->c_str();
		} else if (it->compare("--upper") == 0 && std::next(it) != args.end()) {
			upper = std::next(it)->c_str();
		}
	}

	std::string msg = std::format("SET_BATTERY_LIMITS {} {} {}", status, lower, upper);
	sendMsgToSocket(msg);
}

void functionSetUsageMode(std::string arg) {
	std::string msg = std::format("SET_USAGE_MODE {}", arg);
	sendMsgToSocket(msg);
}

void functionSetSleepEnter() {
    sendMsgToSocket("SLEEP_ENTER");
}

void functionSetSleepExit() {
    sendMsgToSocket("SLEEP_EXIT");
}

int main(int argc, char *argv[]) {
	if (argc > 1) {
		if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
			printHelp();
			return -1;
		}
	}

	if (argc < 2) {
		std::println("Please provide argument!");
		printHelp();
		return -1;
	} else if (std::string(argv[1]) == "set-rgb") {
		if (argc < 4) {
			std::println("set-rgb requires more arguments!\n");
			printHelp();
			return -1;
		}
		std::vector<std::string> args = { argv + 2, argv + argc };
		functionSetRgb(args);
	} else if (std::string(argv[1]) == "set-timeout") {
		if (argc < 3) {
			std::println("set-timeout requires more arguments!\n");
			printHelp();
			return -1;
		}
		functionSetKeyboardTimeout(std::string(argv[2]));
	} else if (std::string(argv[1]) == "set-usage-mode") {
		if (argc < 3) {
			std::println("set-usage-mode requires more arguments!\n");
			printHelp();
			return -1;
		}
		functionSetUsageMode(std::string(argv[2]));
	} else if (std::string(argv[1]) == "set-battery-limits") {
		if (argc < 7) {
			std::println("set-battery-limits requires more arguments!\n");
			printHelp();
			return -1;
		}
		std::vector<std::string> args = { argv + 2, argv + argc };
		functionSetBatteryLimits(args);
    } else if (std::string(argv[1]) == "sleep-enter") {
        functionSetSleepEnter();
    } else if (std::string(argv[1]) == "sleep-exit") {
        functionSetSleepExit();
	} else {
		std::println("Invalid argument!");
		printHelp();
		return -1;
	}

}
