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
    while (m_preview->processEvents())
    {
        m_renderer->render(*m_image, 0, 0, m_image->width, m_image->height);
        
        std::thread::id threadId = std::this_thread::get_id();
        m_preview->update(threadId, 1, 1, 0, 0, m_image->width, m_image->height);
    }
}

}
