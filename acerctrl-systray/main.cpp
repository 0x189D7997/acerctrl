#include <sys/socket.h>
#include <sys/un.h>
#include <print>
#include <unistd.h>

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>
#include <QPixmap>
#include <QColor>

#include "rgb-window.hpp"
#include "hw-settings.hpp"

std::string socket_path = "/run/acerctrl.sock";

void setUsageMode(uint8_t mode) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr {};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), socket_path.length());

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        std::string msg = std::format("SET_USAGE_MODE {}", mode);
        write(sock, msg.c_str(), msg.length());
    }

    close(sock);
}

int main(int argc, char *argv[]) {
    int test_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr {};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), socket_path.length());

    if (connect(test_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::println("Failed to connect to acerctrld! (it is running?)");
        close(test_sock);
        return -1;
    }

    close(test_sock);


    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        std::println("System tray is not available!");
        return -1;
    }

    QApplication::setQuitOnLastWindowClosed(false);

    QPixmap icon(16, 16);
    icon.fill(QColor("teal"));

    QSystemTrayIcon *sys_tray = new QSystemTrayIcon(QIcon(icon), &app);
    QMenu *main_menu = new QMenu();

    RGBWindow *rgb_window = new RGBWindow();
    auto *rgb_action = main_menu->addAction("Change RGB");
    QObject::connect(rgb_action, &QAction::triggered, [rgb_window]() {
        rgb_window->show();
        rgb_window->activateWindow();
    });

    QMenu *usage_mode_menu = main_menu->addMenu("Usage Mode");

    auto addUsageMode = [&](const QString& name, uint8_t mode) {
        QObject::connect(usage_mode_menu->addAction(name), &QAction::triggered, [mode]() {
            setUsageMode(mode);
        });
    };

    addUsageMode("Turbo", 0x00);
    addUsageMode("Performance", 0x01);
    addUsageMode("Normal", 0x02);
    addUsageMode("Quiet", 0x03);
    addUsageMode("Eco", 0x04);
    //addUsageMode("Eco+", 0x05);

    KBBacklightTimeout *kb_timeout_window = new KBBacklightTimeout();
    auto *kb_timeout_action = main_menu->addAction("Keyboard Backlight Timeout");
    QObject::connect(kb_timeout_action, &QAction::triggered, [kb_timeout_window]() {
        kb_timeout_window->show();
        kb_timeout_window->activateWindow();
    });

    BatteryChargeLimits *battery_charge_limits_window = new BatteryChargeLimits();
    auto *battery_limits_action = main_menu->addAction("Battery Charging Limits");
    QObject::connect(battery_limits_action, &QAction::triggered, [battery_charge_limits_window]() {
        battery_charge_limits_window->show();
        battery_charge_limits_window->activateWindow();
    });

    main_menu->addSeparator();
    auto *quit_action = main_menu->addAction("Quit");
    QObject::connect(quit_action, &QAction::triggered, &app, &QCoreApplication::quit);
    sys_tray->setContextMenu(main_menu);
    sys_tray->show();

    int retval = app.exec();
    return retval;
}
