#ifndef __FELIXTRIGGERHANDLER_H__
#define __FELIXTRIGGERHANDLER_H__

#include <TriggerHandler.h>

class felixTriggerHandler : public TriggerHandler
{
public:
    felixTriggerHandler();
    ~felixTriggerHandler();

    //! actual worker routine
    int wait_for_trigger(const int moreinfo = 0);

protected:
    int _highest_fd;
};

#endif
