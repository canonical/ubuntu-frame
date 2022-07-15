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

#ifndef FRAME_CRASH_REPORTER
#define FRAME_CRASH_REPORTER

#include <memory>
#include <mutex>
#include <string>

#include <miral/application.h>
#include <mir/geometry/rectangles.h>

#include <boost/filesystem.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

struct wl_display;

namespace geom = mir::geometry;

class CrashReporter
{
public:
    void set_background_colour(std::string const& option);

    static void render_background(int32_t width, int32_t height, unsigned char* buffer, uint8_t const* colour);
    
    void operator()(wl_display* display);
    void operator()(std::weak_ptr<mir::scene::Session> const& session);

    void stop();

private:
    std::mutex mutable mutex;

    // aubergine in RGB
    uint8_t colour[4] = { 0x38, 0x0c, 0x24, 0xFF };

    boost::filesystem::path const LOG_PATH = boost::filesystem::path("/log/log.txt");

    struct Self;
    std::shared_ptr<Self> self;
};

class TextRenderer
{
public:
    using Pixel = uint32_t;
    using Path = boost::filesystem::path;

    TextRenderer();
    ~TextRenderer();

    void render(
        Pixel* buffer,
        geom::Size buffer_size,
        std::string const& text,
        geom::Point top_left,
        geom::Height height_pixels,
        Pixel color) const;

private:
    const Path FILE_PATH = Path("/log/log.txt"); // TODO - make environment variable
    
    FT_Library library;
    FT_Face face;

    std::mutex mutable mutex;
    
    void set_char_size(geom::Height height) const;
    void rasterize_glyph(char32_t glyph) const;
    void render_glyph(
        Pixel* buf,
        geom::Size buf_size,
        FT_Bitmap const* glyph,
        geom::Point top_left,
        Pixel color) const;

    static auto get_font_path() -> std::string;
    static auto convert_utf8_to_utf32(std::string const& text) -> std::u32string;
};
#endif //FRAME_CRASH_REPORTER
