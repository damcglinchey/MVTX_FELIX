#ifndef __FELIXTRIGGERHANDLER_H__
#define __FELIXTRIGGERHANDLER_H__

#include <TriggerHandler.h>

#include <flxcard/FlxCard.h>
#include "flxdefs.h"

class felixTriggerHandler : public TriggerHandler
{
public:
    felixTriggerHandler(FlxCard* flx, const int deadtime = 0); //deadtime > 0 stands for polling based trigger
    ~felixTriggerHandler();

    //! actual worker routine
    int wait_for_trigger(const int moreinfo = 0);

protected:
    FlxCard* _flx;

    //! flag indicating trigger/polling
    bool _extTrigger;

    //! fixed deadtime
    int _deadtime;
};

#endif
