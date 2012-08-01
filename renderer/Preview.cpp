// Copyright (C) 2012 Sami Kyöstilä
#include "Preview.h"
#include "Surface.h"

#include <SDL/SDL.h>
#include <cstring>

std::unique_ptr<Preview> Preview::create(Surface* surface)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return nullptr;

    std::unique_ptr<Preview> preview(new Preview(surface));
    preview->m_screen = SDL_SetVideoMode(surface->width, surface->height, 32,
                                         SDL_SWSURFACE);
    if (!preview->m_screen)
        return nullptr;
    SDL_WM_SetCaption("Rayno", "Rayno");

    preview->update(0, 0, surface->width, surface->height);
    return preview;
}

Preview::Preview(Surface* surface):
    m_surface(surface)
{
}

Preview::~Preview()
{
    if (m_screen)
        SDL_FreeSurface(m_screen);
    SDL_Quit();
}

void Preview::update(int xOffset, int yOffset, int width, int height)
{
    if (SDL_LockSurface(m_screen) != 0)
        return;

    assert(xOffset + width <= m_screen->w);
    assert(yOffset + height <= m_screen->h);
    
    auto* src = &m_surface->pixels[0];
    auto* dest = reinterpret_cast<uint32_t*>(m_screen->pixels);
    size_t srcStride = m_surface->width;
    size_t destStride = m_screen->pitch / sizeof(*dest);

    src += yOffset * srcStride + xOffset;
    dest += yOffset * destStride + xOffset;

    while (height--)
    {
        std::memcpy(dest, src, width * sizeof(*src));
        src += srcStride;
        dest += destStride;
    }

    SDL_UnlockSurface(m_screen);
    SDL_Flip(m_screen);
}

bool Preview::processEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    return false;
                break;
        }
    }
    return true;
}
