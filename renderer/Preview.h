// Copyright (C) 2012 Sami Kyöstilä
#ifndef PREVIEW_H
#define PREVIEW_H

#include <memory>

class Surface;
class SDL_Surface;

class Preview
{
public:
    ~Preview();
    static std::unique_ptr<Preview> create(Surface* surface);

    bool processEvents();
    void update(int xOffset, int yOffset, int width, int height);
private:
    Preview(Surface* surface);

    Surface* m_surface;
    SDL_Surface* m_screen;
};

#endif
