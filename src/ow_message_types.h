#ifndef OWLWAVE_MESSAGE_TYPES_H
#define OWLWAVE_MESSAGE_TYPES_H

#define OW_MESSAGE_SEPERATOR ","

class OwlWaveMessageTypes
{
public:
    enum MessageTypes {
        SIGNAL_STRENGTH         = 0, // dBm signal strength of last message.
        TEMPERATURE             = 1,
        HUMIDITY                = 2,
        PRESSURE                = 3,
        GAS                     = 4,
        RAINTIPS                = 5,
        RAINRATE                = 6,
        WIND_DIRECTION          = 7,
        WIND_SPEED              = 8,
        AIRQUALITY_PM2_5        = 9,
        AIRQUALITY_PM5          = 10,
        AIRQUALITY_PM10         = 11,
    };
};

#endif //OWLWAVE_MESSAGE_TYPES_H