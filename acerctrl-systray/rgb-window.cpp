#include <format>
#include <sys/socket.h>
#include <sys/un.h>

#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>

#include "rgb-window.hpp"

#include <unistd.h>

RGBWindow::RGBWindow(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::Window | Qt::Tool | Qt::Dialog | Qt::WindowStaysOnTopHint);
    setFixedSize(350, 550);
    setWindowTitle("Acer RGB Control");

    auto *main_layout = new QVBoxLayout(this);

    auto *config_group = new QGroupBox("RGB Control");
    auto *form_layout = new QFormLayout(config_group);

    dev_selector = new QComboBox();
    dev_selector->addItems({"Keyboard", "Lid"});

    effect_selector = new QComboBox();
    effect_selector->addItems({"Off", "Static", "Breathing", "Neon", "Wave", "Ripple", "Zoom", "Snake", "Disco"});

    form_layout->addRow("Device:", dev_selector);
    form_layout->addRow("Effect:", effect_selector);

    main_layout->addWidget(config_group);

    auto *attrib_group = new QGroupBox("Attributes");
    auto *attrib_layout = new QVBoxLayout(attrib_group);

    brightness_slider = new QSlider(Qt::Horizontal);
    brightness_slider->setRange(0, 4);
    brightness_slider->setValue(2);
    attrib_layout->addWidget(new QLabel("Brightness"));
    attrib_layout->addWidget(brightness_slider);

    speed_slider = new QSlider(Qt::Horizontal);
    speed_slider->setRange(0, 9);
    speed_slider->setValue(2);
    attrib_layout->addWidget(new QLabel("Effect Speed"));
    attrib_layout->addWidget(speed_slider);

    direction_selector = new QComboBox();
    direction_selector->addItems({"None", "Right", "Left"});
    attrib_layout->addWidget(new QLabel("Direction"));
    attrib_layout->addWidget(direction_selector);

    main_layout->addWidget(attrib_group);

    auto *color_group = new QGroupBox("Color");
    auto *color_layout = new QVBoxLayout(color_group);

    QLabel *r_label = new QLabel("Red");
    r_slider = new QSlider(Qt::Horizontal);
    r_slider->setRange(0, 255);
    QLabel *g_label = new QLabel("Green");
    g_slider = new QSlider(Qt::Horizontal);
    g_slider->setRange(0, 255);
    QLabel *b_label = new QLabel("Blue");
    b_slider = new QSlider(Qt::Horizontal);
    b_slider->setRange(0, 255);

    color_layout->addWidget(r_label);
    color_layout->addWidget(r_slider);
    color_layout->addWidget(g_label);
    color_layout->addWidget(g_slider);
    color_layout->addWidget(b_label);
    color_layout->addWidget(b_slider);

    main_layout->addWidget(color_group);

    auto *apply_button = new QPushButton("Apply RGB");
    main_layout->addWidget(apply_button);

    connect(apply_button, &QPushButton::clicked, this, &RGBWindow::setRgb);
}

void RGBWindow::setRgb() {
    std::string socket_path = "/run/acerctrl.sock";
    int sock = ::socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr {};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), socket_path.length());

    if (::connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        std::string dev, effect;

        switch(dev_selector->currentIndex()) {
            case 0:
                dev = "keyboard";
                break;
            case 1:
                dev = "lid";
                break;
            case 2:
                dev = "profile_button";
                break;
            default:
                dev = "???";
         }

        switch (effect_selector->currentIndex()) {
            case 0:
                effect = "off";
                break;
            case 1:
                effect = "static";
                break;
            case 2:
                effect = "breathing";
                break;
            case 3:
                effect = "neon";
                break;
            case 4:
                effect = "wave";
                break;
            case 5:
                effect = "ripple";
                break;
            case 6:
                effect = "zoom";
                break;
            case 7:
                effect = "snake";
                break;
            case 8:
                effect = "disco";
                break;
            default:
                effect = "static";
        }

        std::string msg = std::format("SET_RGB {} {} {} {} {} {} {} {} 15", dev, effect, brightness_slider->value() * 25, speed_slider->value(), direction_selector->currentIndex(), r_slider->value(), g_slider->value(), b_slider->value());
        ::write(sock, msg.c_str(), msg.length());
     }
     
     ::close(sock);
};
