// Copyright (C) 2012 Sami Kyöstilä
#ifndef CPU_SCHEDULER_H
#define CPU_SCHEDULER_H

#include "renderer/Scheduler.h"
#include <memory>

class Image;
class Preview;

namespace scene
{
class Scene;
}

namespace cpu
{

class Renderer;

class Scheduler: public ::Scheduler
{
public:
    Scheduler(const scene::Scene&, Image*, Preview*);

    virtual void run() override;

private:
    std::unique_ptr<Renderer> m_renderer;
    Image* m_image;
    Preview* m_preview;
};

}

#endif
