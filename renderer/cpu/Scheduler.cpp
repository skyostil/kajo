// Copyright (C) 2012 Sami Kyöstilä
#include "Scheduler.h"
#include "Renderer.h"
#include "Queue.h"
#include "renderer/Preview.h"
#include "renderer/Image.h"

#include <algorithm>
#include <future>
#include <glm/gtc/matrix_transform.hpp>
#include <unistd.h>
#include <vector>

namespace cpu
{

int cpuCount()
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

struct RenderUpdate
{
    std::thread::id threadId;
    int pass;
    int samples;
    int xOffset, yOffset, width, height;
};

typedef std::vector<std::future<void>> RenderTasks;

void createTasks(Image& image, Renderer& renderer, RenderTasks& tasks)
{
    int slice = (image.height + 1) / cpuCount();
    for (int y = 0; y < image.height; y += slice)
    {
        auto task = std::async(std::launch::async, [=, &renderer, &image] {
            renderer.render(image, 0, y, image.width, slice);
        });
        tasks.push_back(std::move(task));
    }
}

void joinTasks(RenderTasks& tasks)
{
    while (tasks.size())
    {
        tasks.back().wait();
        tasks.pop_back();
    }
}

Scheduler::Scheduler(const scene::Scene& scene, Image* image, Preview* preview):
    m_renderer(new Renderer(scene)),
    m_image(image),
    m_preview(preview)
{
}

void Scheduler::run()
{
    bool done = false;

    Queue<RenderUpdate> updateQueue;
    m_renderer->setObserver([&updateQueue, &done] (int pass, int samples, int xOffset, int yOffset, int width, int height) {
        std::thread::id threadId = std::this_thread::get_id();
        updateQueue.push(RenderUpdate{threadId, pass, samples, xOffset, yOffset, width, height});
        return !done;
    });

    RenderTasks tasks;
    createTasks(*m_image, *m_renderer, tasks);

    while (m_preview->processEvents())
    {
        RenderUpdate update;
        if (updateQueue.pop(update, std::chrono::milliseconds(500)))
            m_preview->update(update.threadId, update.pass, update.samples,
                              update.xOffset, update.yOffset,
                              update.width, update.height);
    }

    done = true;
    joinTasks(tasks);
}

}
