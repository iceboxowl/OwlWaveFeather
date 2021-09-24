#ifndef OWLWAVE_MESSAGE_TYPES_H
#define OWLWAVE_MESSAGE_TYPES_H

#define OW_MESSAGE_SEPERATOR ","

class OwlWaveMessageTypes
{
public:
    enum MessageTypes {
        TEMPERATURE     = 0,
        HUMIDITY        = 1,
        PRESSURE        = 2,
        GAS             = 3,
        RAINTIPS        = 4,
        RAINRATE        = 5,
        AIRQUALITY      = 6
    };
};

#endif //OWLWAVE_MESSAGE_TYPES_H