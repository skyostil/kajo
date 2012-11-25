// Copyright (C) 2012 Sami Kyöstilä
#ifndef SCHEDULER_H
#define SCHEDULER_H

class Image;

namespace scene
{
class Scene;
}

class Scheduler
{
public:
    virtual void run() = 0;
};

#endif
