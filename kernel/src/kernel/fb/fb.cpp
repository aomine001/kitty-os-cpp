//
// Created by Piotr on 02.05.2024.
//

#include "libmath/math.hpp"
#include "fb.hpp"

namespace Framebuffer
{
    volatile limine_framebuffer_request _FramebuffersRequest = {
            .id = LIMINE_FRAMEBUFFER_REQUEST,
            .revision = 0,
            .response = nullptr,
    };

    size_t _Fbcount = 0;
    limine_framebuffer** _Framebuffers = nullptr;
    limine_framebuffer* _MainFramebuffer = nullptr;

    void Initialize()
    {
        if (_FramebuffersRequest.response == nullptr)
        {
            unreachable();
        }

        _Framebuffers = _FramebuffersRequest.response->framebuffers;
        _Fbcount = _FramebuffersRequest.response->framebuffer_count;

        if (_Fbcount > 0)
        _MainFramebuffer = _Framebuffers[0];
    }
    void DrawPixel(size_t _FbIdx, size_t xpos, size_t ypos, uint8_t r, uint8_t g, uint8_t b)
    {
        if (_FbIdx >= _Fbcount)
        {
            return; // Invalid framebuffer index, exit function
        }

        auto fb = _Framebuffers[_FbIdx]; // Get the framebuffer at the specified index

        auto fbwidth = fb->width;
        auto fbheight = fb->height;

        if (xpos >= fbwidth || ypos >= fbheight)
        {
            return; // Invalid pixel coordinates, exit function
        }

        // Calculate the offset of the pixel in memory
        size_t offset = (ypos * fbwidth + xpos) * (fb->bpp / 8);

        // Calculate the address of the pixel in memory
        volatile uint8_t* pixelAddress = reinterpret_cast<volatile uint8_t*>(fb->address) + offset;

        // Calculate the shift for each color component
        uint8_t rShift = fb->red_mask_shift;
        uint8_t gShift = fb->green_mask_shift;
        uint8_t bShift = fb->blue_mask_shift;

        // Calculate the mask for each color component
        uint32_t rMask = (1 << fb->red_mask_size) - 1;
        uint32_t gMask = (1 << fb->green_mask_size) - 1;
        uint32_t bMask = (1 << fb->blue_mask_size) - 1;

        // Clear the existing color components at the pixel address
        *reinterpret_cast<volatile uint32_t*>(pixelAddress) &= ~(rMask << rShift);
        *reinterpret_cast<volatile uint32_t*>(pixelAddress) &= ~(gMask << gShift);
        *reinterpret_cast<volatile uint32_t*>(pixelAddress) &= ~(bMask << bShift);

        // Write the color components to the framebuffer memory
        *reinterpret_cast<volatile uint32_t*>(pixelAddress) |= (r & rMask) << rShift;
        *reinterpret_cast<volatile uint32_t*>(pixelAddress) |= (g & gMask) << gShift;
        *reinterpret_cast<volatile uint32_t*>(pixelAddress) |= (b & bMask) << bShift;
    }
    void DrawRectangle(size_t _FbIdx, size_t xpos, size_t ypos, uint8_t r, uint8_t g, uint8_t b, size_t rectw, size_t recth)
    {
        if (_FbIdx >= _Fbcount)
        {
            return; // Invalid framebuffer index, exit function
        }

        auto fb = _Framebuffers[_FbIdx]; // Get the framebuffer at the specified index

        auto fbwidth = fb->width;
        auto fbheight = fb->height;

        if (xpos >= fbwidth || ypos >= fbheight)
        {
            return; // Invalid starting coordinates, exit function
        }

        // Ensure the rectangle dimensions are within bounds
        rectw = Math::min(rectw, fbwidth - xpos);
        recth = Math::min(recth, fbheight - ypos);

        // Draw top edge
        for (size_t x = xpos; x < xpos + rectw; ++x)
        {
            DrawPixel(_FbIdx, x, ypos, r, g, b);
        }

        // Draw bottom edge
        for (size_t x = xpos; x < xpos + rectw; ++x)
        {
            DrawPixel(_FbIdx, x, ypos + recth - 1, r, g, b);
        }

        // Draw left edge
        for (size_t y = ypos; y < ypos + recth; ++y)
        {
            DrawPixel(_FbIdx, xpos, y, r, g, b);
        }

        // Draw right edge
        for (size_t y = ypos; y < ypos + recth; ++y)
        {
            DrawPixel(_FbIdx, xpos + rectw - 1, y, r, g, b);
        }
    }
    void DrawFilledRectangle(size_t _FbIdx, size_t xpos, size_t ypos, uint8_t r, uint8_t g, uint8_t b, size_t rectw, size_t recth)
    {
        if (_FbIdx >= _Fbcount)
        {
            return; // Invalid framebuffer index, exit function
        }

        auto fb = _Framebuffers[_FbIdx]; // Get the framebuffer at the specified index

        auto fbwidth = fb->width;
        auto fbheight = fb->height;

        if (xpos >= fbwidth || ypos >= fbheight)
        {
            return; // Invalid starting coordinates, exit function
        }

        // Ensure the rectangle dimensions are within bounds
        rectw = Math::min(rectw, fbwidth - xpos);
        recth = Math::min(recth, fbheight - ypos);

        // Draw each pixel within the rectangle with the specified color
        for (size_t y = ypos; y < ypos + recth; ++y)
        {
            for (size_t x = xpos; x < xpos + rectw; ++x)
            {
                DrawPixel(_FbIdx, x, y, r, g, b);
            }
        }
    }
    void DrawCircle(size_t _FbIdx, size_t xpos, size_t ypos, uint8_t r, uint8_t g, uint8_t b, size_t radius)
    {
        if (_FbIdx >= _Fbcount)
        {
            return; // Invalid framebuffer index, exit function
        }

        auto fb = _Framebuffers[_FbIdx]; // Get the framebuffer at the specified index

        auto fbwidth = fb->width;
        auto fbheight = fb->height;

        if (xpos >= fbwidth || ypos >= fbheight)
        {
            return; // Invalid starting coordinates, exit function
        }

        size_t x = radius;
        size_t y = 0;
        ssize_t radiusError = 1 - x;

        while (x >= y)
        {
            DrawPixel(_FbIdx, xpos + x, ypos + y, r, g, b);
            DrawPixel(_FbIdx, xpos - x, ypos + y, r, g, b);
            DrawPixel(_FbIdx, xpos + x, ypos - y, r, g, b);
            DrawPixel(_FbIdx, xpos - x, ypos - y, r, g, b);
            DrawPixel(_FbIdx, xpos + y, ypos + x, r, g, b);
            DrawPixel(_FbIdx, xpos - y, ypos + x, r, g, b);
            DrawPixel(_FbIdx, xpos + y, ypos - x, r, g, b);
            DrawPixel(_FbIdx, xpos - y, ypos - x, r, g, b);

            y++;
            if (radiusError < 0)
            {
                radiusError += 2 * y + 1;
            }
            else
            {
                x--;
                radiusError += 2 * (y - x) + 1;
            }
        }
    }
    void DrawFilledCircle(size_t _FbIdx, size_t xpos, size_t ypos, uint8_t r, uint8_t g, uint8_t b, size_t radius)
    {
        if (_FbIdx >= _Fbcount)
        {
            return; // Invalid framebuffer index, exit function
        }

        auto fb = _Framebuffers[_FbIdx]; // Get the framebuffer at the specified index

        auto fbwidth = fb->width;
        auto fbheight = fb->height;

        if (xpos >= fbwidth || ypos >= fbheight)
        {
            return; // Invalid starting coordinates, exit function
        }

        size_t x = radius;
        size_t y = 0;
        ssize_t radiusError = 1 - x;

        while (x >= y)
        {
            // Draw horizontal lines to fill the circle
            for (size_t i = xpos - x; i <= xpos + x; ++i)
            {
                DrawLine(_FbIdx, i, ypos + y, i, ypos - y, r, g, b);
            }
            for (size_t i = xpos - y; i <= xpos + y; ++i)
            {
                DrawLine(_FbIdx, i, ypos + x, i, ypos - x, r, g, b);
            }

            y++;
            if (radiusError < 0)
            {
                radiusError += 2 * y + 1;
            }
            else
            {
                x--;
                radiusError += 2 * (y - x) + 1;
            }
        }
    }

    void DrawLine(size_t _FbIdx, size_t x0, size_t y0, size_t x1, size_t y1, uint8_t r, uint8_t g, uint8_t b)
    {
        ssize_t dx = static_cast<ssize_t>(Math::abs(static_cast<ssize_t>(x1) - static_cast<ssize_t>(x0)));
        ssize_t dy = static_cast<ssize_t>(Math::abs(static_cast<ssize_t>(y1) - static_cast<ssize_t>(y0)));
        ssize_t sx = x0 < x1 ? 1 : -1;
        ssize_t sy = y0 < y1 ? 1 : -1;
        ssize_t err = dx - dy;

        while (true)
        {
            DrawPixel(_FbIdx, x0, y0, r, g, b);

            if (x0 == x1 && y0 == y1) break;

            ssize_t e2 = 2 * err;
            if (e2 > -dy)
            {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx)
            {
                err += dx;
                y0 += sy;
            }
        }
    }

    limine_framebuffer* GetFramebuffer(size_t _FbIdx)
    {
        if (_FbIdx >= _Fbcount) return nullptr;
        return _Framebuffers[_FbIdx];
    }
}