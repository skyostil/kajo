// Copyright (C) 2012 Sami Kyöstilä
#ifndef PREVIEW_H
#define PREVIEW_H

#include <chrono>
#include <map>
#include <memory>
#include <thread>
#include <SDL/SDL_ttf.h>

class Image;
class SDL_Surface;

class Preview
{
public:
    ~Preview();
    static std::unique_ptr<Preview> create(Image* image);

    bool processEvents();
    void update(std::thread::id threadId, int pass, int samples, int xOffset, int yOffset, int width, int height);

private:
    Preview(Image* image);
    void updateScreen(int xOffset, int yOffset, int width, int height);
    void drawStatusLine();

    Image* m_image;
    SDL_Surface* m_screen;
    TTF_Font* m_font;

    class ThreadStatistics
    {
    public:
        ThreadStatistics():
            pass(0),
            samples(0),
            samplesPerSecond(0)
        {
        }

        int pass;
        long long samples;

        std::chrono::monotonic_clock::time_point startTime;
        long long startSamples;

        long long samplesPerSecond;
    };

    std::chrono::monotonic_clock::time_point m_lastUpdate;
    std::chrono::monotonic_clock::time_point m_startTime;
    std::map<std::thread::id, ThreadStatistics> m_threadStatistics;
};

#endif
