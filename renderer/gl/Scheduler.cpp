// Copyright (C) 2012 Sami Kyöstilä
#include "Scheduler.h"
#include "Renderer.h"
#include "Scene.h"
#include "renderer/Preview.h"
#include "renderer/Image.h"

namespace gl
{

Scheduler::Scheduler(const scene::Scene& scene, Image* image, Preview* preview):
    m_renderer(new Renderer(scene, image)),
    m_image(image),
    m_preview(preview)
{
}

void Scheduler::run()
{
    int pass = 1;
    while (m_preview->processEvents())
    {
        m_renderer->render();
        std::thread::id threadId = std::this_thread::get_id();
        m_preview->update(threadId, pass++, 16, 0, 0, m_image->width, m_image->height);
    }
}

}
