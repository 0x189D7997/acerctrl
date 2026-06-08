#!/bin/sh

if [ "$1" == "pre" ]; then
    acerctrl-cli sleep-enter
elif [ "$1" == "post" ]; then
    acerctrl-cli sleep-exit
fi
