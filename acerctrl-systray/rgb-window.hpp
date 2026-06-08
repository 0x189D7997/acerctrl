#pragma once

#include <QWidget>
#include <QComboBox>
#include <QSlider>

class RGBWindow : public QWidget {
    Q_OBJECT

    public:
        explicit RGBWindow(QWidget *parent = nullptr);
        ~RGBWindow() = default;

    private:
        QComboBox *dev_selector;
        QComboBox *effect_selector;
        QComboBox *direction_selector;
        QSlider *brightness_slider;
        QSlider *speed_slider;
        QSlider *r_slider;
        QSlider *g_slider;
        QSlider *b_slider;

        void setRgb();
};
