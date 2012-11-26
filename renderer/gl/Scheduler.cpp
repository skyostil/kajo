// Copyright (C) 2012 Sami Kyöstilä
#include "Scheduler.h"
#include "Renderer.h"
#include "renderer/Preview.h"
#include "renderer/Image.h"

namespace gl
{

Scheduler::Scheduler(const scene::Scene& scene, Image* image, Preview* preview):
    m_renderer(new Renderer(scene)),
    m_image(image),
    m_preview(preview)
{
}

void Scheduler::run()
{
}

}
