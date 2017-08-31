#include "felixTriggerHandler.h"

#include <iostream>
#include <unistd.h>

felixTriggerHandler::felixTriggerHandler(FlxCard* flx, const int deadtime)
{
    _flx = flx;

    _deadtime = deadtime;
    _extTrigger = (deadtime <= 0);
}

felixTriggerHandler::~felixTriggerHandler() {}

int felixTriggerHandler::wait_for_trigger(const int moreinfo)
{
    if(_extTrigger)
    {
        _flx->irq_wait(FLX_INT_DATA_AVAILABLE);
    }
    else
    {
        usleep(_deadtime);
    }

    return 1;
}
