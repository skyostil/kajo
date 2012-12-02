// Copyright (C) 2012 Sami Kyöstilä
#include "Preview.h"
#include "Image.h"
#include "Util.h"

#include <cstring>
#include <iostream>
#include <SDL/SDL.h>

const int fontSize = 16;
const int statusLineHeight = fontSize + 5;

std::unique_ptr<Preview> Preview::create(Image* image, bool useOpenGL)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return nullptr;

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf: " << TTF_GetError() << std::endl;
        return nullptr;
    }
    std::unique_ptr<Preview> preview(new Preview(image));

    preview->m_font = TTF_OpenFont("../data/SourceSansPro-Regular.ttf", fontSize);
    if (!preview->m_font) {
        std::cerr << "SDL_ttf: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    preview->m_screen = SDL_SetVideoMode(image->width, image->height + statusLineHeight, 32,
                                         useOpenGL ? SDL_OPENGL : SDL_SWSURFACE);
    if (!preview->m_screen)
        return nullptr;
    SDL_WM_SetCaption("Rayno", "Rayno");

    if (useOpenGL) {
        if (glewInit() != GLEW_OK) {
            std::cerr << "glewInit() failed" << std::endl;
            return nullptr;
        }
        glGenTextures(1, &preview->m_texture);
        glBindTexture(GL_TEXTURE_2D, preview->m_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height + statusLineHeight, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glGenFramebuffers(1, &preview->m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, preview->m_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, preview->m_texture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        preview->m_statusSurface =
            SDL_CreateRGBSurface(SDL_SWSURFACE, image->width, statusLineHeight, 32,
                                 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    }

    preview->updateScreen(0, 0, image->width, image->height);
    return preview;
}

Preview::Preview(Image* image):
    m_image(image),
    m_font(0),
    m_texture(0),
    m_fbo(0),
    m_statusSurface(0),
    m_startTime(std::chrono::steady_clock::now())
{
}

Preview::~Preview()
{
    if (m_fbo)
        glDeleteFramebuffers(1, &m_fbo);
    if (m_texture)
        glDeleteTextures(1, &m_texture);
    if (m_font)
        TTF_CloseFont(m_font);
    if (m_statusSurface)
        SDL_FreeSurface(m_statusSurface);
    if (m_screen)
        SDL_FreeSurface(m_screen);
    TTF_Quit();
    SDL_Quit();
}

void Preview::update(std::thread::id threadId, int pass, int samples, int xOffset, int yOffset, int width, int height)
{
    ThreadStatistics& stats = m_threadStatistics[threadId];
    stats.samples += samples * width * height;
    stats.pass = pass;

    auto now = std::chrono::steady_clock::now();
    if (now - stats.startTime > std::chrono::seconds(1))
    {
        stats.samplesPerSecond = stats.samples - stats.startSamples;
        stats.startSamples = stats.samples;
        stats.startTime = now;
    }

    if (now - m_lastUpdate > std::chrono::milliseconds(100))
    {
        m_lastUpdate = now;
        updateScreen(0, 0, m_image->width, m_image->height);
    }
}

void Preview::updateScreen(int xOffset, int yOffset, int width, int height)
{
    auto* src = &m_image->pixels[0];
    size_t srcStride = m_image->width;

    if (m_texture) {
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, m_image->width);
        glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, width, height,
                        GL_BGRA, GL_UNSIGNED_BYTE, &src[srcStride * yOffset + xOffset]);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        drawStatusLine();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
        glBlitFramebuffer(0, 0, m_image->width, m_image->height + statusLineHeight,
                             0, m_image->height + statusLineHeight, m_image->width, 0,
                             GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

        SDL_GL_SwapBuffers();
        return;
    }

    if (SDL_LockSurface(m_screen) != 0)
        return;

    assert(xOffset + width <= m_screen->w);
    assert(yOffset + height <= m_screen->h);

    auto* dest = reinterpret_cast<uint32_t*>(m_screen->pixels);
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
    drawStatusLine();
    SDL_Flip(m_screen);
}

void Preview::drawStatusLine()
{
    long long totalSamples = 0;
    long long totalPerSecond = 0;
    long long maxPerSecond = 0;
    for (auto& thread: m_threadStatistics)
    {
        totalSamples += thread.second.samples;
        totalPerSecond += thread.second.samplesPerSecond;
        maxPerSecond = std::max(maxPerSecond, thread.second.samplesPerSecond);
    }
    auto elapsed = std::chrono::steady_clock::now() - m_startTime;
    SDL_Surface* surface = m_statusSurface ? m_statusSurface : m_screen;

    std::ostringstream status;
    status << m_image->width << 'x' << m_image->height << ", ";
    status << std::chrono::duration_cast<std::chrono::minutes>(elapsed).count() << " min ";
    status << std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() % 60 << " s, ";
    formatSI(status, totalSamples / float(m_image->width * m_image->height), "samples/pixel, ");
    formatSI(status, totalPerSecond, "samples/s");

    SDL_Color textColor = {0x0, 0x0, 0x0, 0x0};
    uint32_t backgroundColor = SDL_MapRGB(surface->format, 0xaa, 0xaa, 0xaa);
    SDL_Surface* statusText = TTF_RenderText_Blended(m_font, status.str().c_str(), textColor);

    if (!statusText)
        return;

    SDL_Rect textRect = {
        4, int16_t(surface->h - statusText->h), uint16_t(statusText->w), uint16_t(statusText->h)
    };
    SDL_Rect statusLineRect = {
        0, int16_t(surface->h - statusLineHeight), uint16_t(surface->w), uint16_t(statusLineHeight)
    };
    SDL_FillRect(surface, &statusLineRect, backgroundColor);

    if (maxPerSecond) {
        uint32_t barColor = SDL_MapRGB(surface->format, 0x66, 0x66, 0x66);
        int n = 1;
        int barWidth = 2;
        int barHeight = 12;
        int margin = 2;
        int padding = 2;
        for (auto& thread: m_threadStatistics)
        {
            float ratio = float(thread.second.samplesPerSecond) / maxPerSecond;
            int h = barHeight * ratio;
            SDL_Rect rect = {
                int16_t(surface->w - margin - (barWidth + padding) * n), int16_t(surface->h - margin - h),
                uint16_t(barWidth), uint16_t(h)
            };
            SDL_FillRect(surface, &rect, barColor);
            n++;
        }
    }

    SDL_BlitSurface(statusText, nullptr, surface, &textRect);
    SDL_FreeSurface(statusText);

    if (m_texture && SDL_LockSurface(surface) == 0) {
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->pitch / sizeof(uint32_t));
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, m_image->height, surface->w, surface->h,
                        GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        SDL_UnlockSurface(surface);
    }
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
                else if (event.key.keysym.sym == SDLK_s && (event.key.keysym.mod & KMOD_LCTRL)) {
                    std::cout << "Saving preview to preview.png" << std::endl;
                    m_image->save("preview.png");
                }
                break;
        }
    }
    return true;
}
