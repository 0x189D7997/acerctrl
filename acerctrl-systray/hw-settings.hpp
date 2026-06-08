#pragma once

#include <QCheckBox>
#include <QSpinBox>
#include <QWidget>

class KBBacklightTimeout : public QWidget {
    Q_OBJECT

    public:
        explicit KBBacklightTimeout(QWidget *parent = nullptr);
        ~KBBacklightTimeout() = default;

    private:
        QSpinBox *timeout_value;

        void setTimeout();
};

class BatteryChargeLimits : public QWidget {
    Q_OBJECT

    public:
    explicit BatteryChargeLimits(QWidget *parent = nullptr);
    ~BatteryChargeLimits() = default;

private:
    QCheckBox *status;
    QSpinBox *lower_value;
    QSpinBox *upper_value;

    void setLimits();
};