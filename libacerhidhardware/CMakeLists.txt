add_library(AcerHIDHardware SHARED)

target_sources(AcerHIDHardware PUBLIC FILE_SET CXX_MODULES FILES acerhidhardware.cppm)
