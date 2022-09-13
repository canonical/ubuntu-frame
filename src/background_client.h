/*
 * Copyright © 2022 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FRAME_BACKGROUND_CLIENT
#define FRAME_BACKGROUND_CLIENT

#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include <miral/application.h>
#include <mir/geometry/rectangles.h>

#include <boost/filesystem.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

struct wl_display;

namespace geom = mir::geometry;
using Colour = unsigned char[4];

class BackgroundClient
{
public:
    void set_wallpaper_top_colour(std::string const& option);
    void set_wallpaper_bottom_colour(std::string const& option);
    void set_crash_background_colour(std::string const& option);
    void set_crash_text_colour(std::string const& option);
    void set_diagnostic_path(std::string const& option);
    void set_font_size(std::string const& option);
    void set_x_margin(std::string const& option);
    void set_y_margin(std::string const& option);

    /// Renders background as a gradient from top_colour to bottom_colour
    static void render_background(
        int32_t width,
        int32_t height,
        unsigned char* buffer,
        Colour const& bottom_colour,
        Colour const& top_colour);
    
    static void render_background(
        int32_t width,
        int32_t height,
        unsigned char* buffer,
        Colour const& colour);

    void operator()(wl_display* display);
    void operator()(std::weak_ptr<mir::scene::Session> const& session);

    void stop();

private:
    std::mutex mutable mutex;

    Colour wallpaper_top_colour = {127, 127, 127, 255};
    Colour wallpaper_bottom_colour = {31, 31, 31, 255};
    Colour crash_background_colour = {36, 12, 56, 255};
    Colour crash_text_colour = {255, 255, 255, 255};

    uint font_size;
    
    uint x_margin_percent;
    uint y_margin_percent;

    std::optional<boost::filesystem::path> diagnostic_path;

    struct Self;
    std::weak_ptr<Self> self;

    void set_colour(std::string const& option, Colour& colour);
};

class TextRenderer
{
public:
    using Path = boost::filesystem::path;

    TextRenderer(Path font_path);
    ~TextRenderer();

    void render(
        unsigned char* buf,
        geom::Size buf_size,
        std::string const& text,
        geom::Point top_left,
        geom::Height height_pixels,
        Colour const& colour) const;

private:
    Path font_path;

    FT_Library library;
    FT_Face face;

    std::mutex mutable mutex;
    
    void set_char_size(geom::Height height) const;
    void rasterize_glyph(char32_t glyph) const;
    void render_glyph(
        unsigned char* buffer,
        geom::Size buf_size,
        FT_Bitmap const* glyph,
        geom::Point top_left,
        Colour const& colour) const;

    static auto get_font_path() -> std::optional<Path>;
    static auto convert_utf8_to_utf32(std::string const& text) -> std::u32string;
};

#endif //FRAME_BACKGROUND_CLIENT
