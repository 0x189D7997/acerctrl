#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>


#include "hw-settings.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>


KBBacklightTimeout::KBBacklightTimeout(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::Window | Qt::Tool | Qt::Dialog | Qt::WindowStaysOnTopHint);
    setFixedSize(250, 100);
    setWindowTitle("Keyboard Timeout");

    auto *main_layout = new QVBoxLayout(this);

    auto label = new QLabel("Keyboard Backlight Timeout");

    timeout_value = new QSpinBox();
    timeout_value->setRange(0, 255); /* if you want it higher for whatever reason just use the cli at that point */
    timeout_value->setValue(0);

    main_layout->addWidget(label);
    main_layout->addWidget(timeout_value);

    auto *apply_button = new QPushButton("Set Timeout");
    main_layout->addWidget(apply_button);

    connect(apply_button, &QPushButton::clicked, this, &KBBacklightTimeout::setTimeout);
}

BatteryChargeLimits::BatteryChargeLimits(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::Window | Qt::Tool | Qt::Dialog | Qt::WindowStaysOnTopHint);
    setFixedSize(250, 200);
    setWindowTitle("Battery Charging Limits");

    auto *main_layout = new QVBoxLayout(this);

    auto label = new QLabel("Battery Charging Limits");

    status = new QCheckBox("Enabled");
    status->setChecked(true);
    lower_value = new QSpinBox();
    auto lower_label = new QLabel("Lower Limit:");
    lower_value->setRange(0, 100);
    lower_value->setValue(0);
    upper_value = new QSpinBox();
    auto upper_label = new QLabel("Upper Limit:");
    upper_value->setRange(0, 100);
    upper_value->setValue(0);

    main_layout->addWidget(label);
    main_layout->addWidget(status);
    main_layout->addWidget(lower_label);
    main_layout->addWidget(lower_value);
    main_layout->addWidget(upper_label);
    main_layout->addWidget(upper_value);

    auto *apply_button = new QPushButton("Set Limits");
    main_layout->addWidget(apply_button);

    connect(apply_button, &QPushButton::clicked, this, &BatteryChargeLimits::setLimits);
}

void BatteryChargeLimits::setLimits() {
    std::string socket_path = "/run/acerctrl.sock";
    int sock = ::socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr {};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), socket_path.length());

    if (::connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        int status_int = status->isChecked();
        std::string msg = std::format("SET_BATTERY_LIMITS {} {} {}", status_int, lower_value->value(), upper_value->value());
        ::write(sock, msg.c_str(), msg.length());
    }

    ::close(sock);
}

void KBBacklightTimeout::setTimeout() {
    std::string socket_path = "/run/acerctrl.sock";
    int sock = ::socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr {};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), socket_path.length());

    if (::connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        std::string msg = std::format("SET_TIMEOUT {}", timeout_value->value());
        ::write(sock, msg.c_str(), msg.length());
    }

    ::close(sock);
};
