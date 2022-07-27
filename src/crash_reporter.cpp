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
 *
 * Authored by: Grayson Guarino <graysonguarino@canonical.com>
 */

#include "crash_reporter.h"
#include "egfullscreenclient.h"

#include "mir/log.h"

#include <cstring>
#include <codecvt>
#include <sstream>

#include <boost/throw_exception.hpp>
#include <boost/filesystem.hpp>


struct CrashReporter::Self : egmde::FullscreenClient
{
    using Path = boost::filesystem::path;

    Self(wl_display* display, Pixel colour);

    void draw_screen(SurfaceInfo& info) const override;
    
    void render_text(int32_t width, int32_t height, Pixel* buffer, boost::filesystem::path const& log_path) const;

    Pixel const colour;
    TextRenderer text_renderer;
    Path const log_path;
};

void CrashReporter::render_background(int32_t width, int32_t height, Pixel* buffer, Pixel colour)
{
    for (int current_y = 0; current_y < height; current_y++)
    {
        for (int current_x = 0; current_x < width; current_x++)
        {
            memcpy(buffer + current_x, &colour, sizeof(buffer[current_x]));
        }

        buffer += width;
    }
}

void CrashReporter::set_background_colour(std::string const& option)
{
    uint32_t value;
    std::stringstream interpreter{option};

    if (interpreter >> std::hex >> value)
    {
        colour = value;
    }
}

void CrashReporter::operator()(wl_display* display)
{
    auto client = std::make_shared<Self>(display, colour);
    {
        std::lock_guard<decltype(mutex)> lock{mutex};
        self = client;
    }
    client->run(display);

    // Possibly need to wait for stop() to release the client.
    // (This would be less ugly with a ref-counted wrapper for wl_display* in the miral API)
    std::lock_guard<decltype(mutex)> lock{mutex};
    client.reset();
}

void CrashReporter::operator()(std::weak_ptr<mir::scene::Session> const& /*session*/)
{
}

CrashReporter::Self::Self(wl_display* display, Pixel colour) :
    FullscreenClient(display),
    colour{colour},
    text_renderer{TextRenderer()},
    log_path{Path("/home/graysonguarino/Documents/log/log.txt")}
{
    wl_display_roundtrip(display);
    wl_display_roundtrip(display);
}

void CrashReporter::Self::render_text(
    int32_t width, 
    int32_t height,
    Pixel* buffer, 
    boost::filesystem::path const& log_path) const
{
    std::string line;

    auto size = geom::Size{width, height};

    // TODO - this aint it chief
    auto top_left = geom::Point{0, 0};
    auto const height_pixels = geom::Height(40);
    auto const y_kerning = height_pixels + (height_pixels / 5);
    auto const colour = Pixel(255, 255, 255, 255); // TODO - Rewrite this to load a selectable colour
    
    auto file_observer = FileObserver(log_path);
    if (file_observer.file_exists()) 
    {
        auto stream = boost::filesystem::ifstream(log_path);
        while (getline(stream, line))
        {
            text_renderer.render(buffer, size, line, top_left, height_pixels, colour);
            auto const new_top_left = geom::Point{top_left.x, top_left.y.as_value() + y_kerning.as_value()};
            top_left = new_top_left;
        }
    }
}

void CrashReporter::Self::draw_screen(SurfaceInfo& info) const
{
    bool const rotated = info.output->transform & WL_OUTPUT_TRANSFORM_90;
    auto const width = rotated ? info.output->height : info.output->width;
    auto const height = rotated ? info.output->width : info.output->height;

    if (width <= 0 || height <= 0)
        return;

    auto const stride = 4*width;

    if (!info.surface)
    {
        info.surface = wl_compositor_create_surface(compositor);
    }

    if (!info.shell_surface)
    {
        info.shell_surface = wl_shell_get_shell_surface(shell, info.surface);
        wl_shell_surface_set_fullscreen(
            info.shell_surface,
            WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
            0,
            info.output->output);
    }

    if (info.buffer)
    {
        wl_buffer_destroy(info.buffer);
    }

    {
        auto const shm_pool = make_shm_pool(stride * height, &info.content_area);

        info.buffer = wl_shm_pool_create_buffer(
            shm_pool.get(),
            0,
            width, height, stride,
            WL_SHM_FORMAT_ARGB8888);
    }

    auto buffer = static_cast<Pixel*>(info.content_area);
    render_background(width, height, buffer, colour);

    render_text(width, height, buffer, log_path);

    wl_surface_attach(info.surface, info.buffer, 0, 0);
    wl_surface_set_buffer_scale(info.surface, info.output->scale_factor);
    wl_surface_commit(info.surface);
}

void CrashReporter::stop()
{
    if (auto ss = self.get())
    {
        std::lock_guard<decltype(mutex)> lock{mutex};
        ss->stop();
    }
}

inline auto area(geom::Size size) -> size_t
{
    return (size.width > geom::Width{} && size.height > geom::Height{})
        ? size.width.as_int() * size.height.as_int()
        : 0;
}

TextRenderer::TextRenderer()
{
    if (auto const error = FT_Init_FreeType(&library))
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(
            "Initializing freetype library failed with error " + std::to_string(error)));
    }
        
    auto const path = get_font_path();
    if (auto const error = FT_New_Face(library, path.c_str(), 0, &face))
    {
        if (error == FT_Err_Unknown_File_Format)
        {
            BOOST_THROW_EXCEPTION(std::runtime_error(
                "Font " + path + " has unsupported format"));
        }
            
        else
        {
            BOOST_THROW_EXCEPTION(std::runtime_error(
                "Loading font from " + path + " failed with error " + std::to_string(error)));
        }
    }
}

TextRenderer::~TextRenderer()
{
    if (auto const error = FT_Done_Face(face))
    {
        mir::log_warning("Failed to uninitialize font face with error %d", error);
    } 
    face = nullptr;

    if (auto const error = FT_Done_FreeType(library))
    {
        mir::log_warning("Failed to uninitialize FreeType with error %d", error);
    }
    library = nullptr;
}

auto default_font() -> std::string
{
    struct FontPath
    {
        char const* filename;
        std::vector<char const*> prefixes;
    };

    FontPath const font_paths[]{
        FontPath{"Ubuntu-B.ttf", {
            "ubuntu-font-family",   // Ubuntu < 18.04
            "ubuntu",               // Ubuntu >= 18.04/Arch
        }},
        FontPath{"FreeSansBold.ttf", {
            "freefont",             // Debian/Ubuntu
            "gnu-free",             // Fedora/Arch
        }},
        FontPath{"DejaVuSans-Bold.ttf", {
            "dejavu",               // Ubuntu (others?)
            "",                     // Arch
        }},
        FontPath{"LiberationSans-Bold.ttf", {
            "liberation-sans",      // Fedora
            "liberation",           // Arch
        }},
    };

    char const* const font_path_search_paths[]{
        "/usr/share/fonts/truetype",    // Ubuntu/Debian
        "/usr/share/fonts/TTF",         // Arch
        "/usr/share/fonts",             // Fedora/Arch
    };

    std::vector<std::string> usable_search_paths;
    for (auto const& path : font_path_search_paths)
    {
        if (boost::filesystem::exists(path))
            usable_search_paths.push_back(path);
    }

    for (auto const& font : font_paths)
    {
        for (auto const& prefix : font.prefixes)
        {
            for (auto const& path : usable_search_paths)
            {
                auto const full_font_path = path + '/' + prefix + '/' + font.filename;
                if (boost::filesystem::exists(full_font_path))
                {
                    return full_font_path;
                }
                    
            }
        }
    }

    mir::log_warning("Can't find a default font!");
    return "";
}

auto TextRenderer::convert_utf8_to_utf32(std::string const& text) -> std::u32string
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::u32string utf32_text;
    try 
    {
        utf32_text = converter.from_bytes(text);
    } 
    catch(const std::range_error& e) 
    {
        mir::log_warning("Window title %s is not valid UTF-8", text.c_str());
        // fall back to ASCII
        for (char const c : text)
        {
            if (isprint(c))
            {
                utf32_text += c;
            }  
            else
            {
                utf32_text += 0xFFFD; // REPLACEMENT CHARACTER (�)
            }    
        }
    }

    return utf32_text;
}

void TextRenderer::render(
    Pixel* buf,
    geom::Size buf_size,
    std::string const& text,
    geom::Point top_left,
    geom::Height height_pixels,
    Pixel colour) const
{
    if (!area(buf_size) || height_pixels <= geom::Height{})
    {
        return;
    }

    std::lock_guard lock{mutex};

    if (!library || !face)
    {
        mir::log_warning("FreeType not initialized");
        return;
    }

    try
    {
        set_char_size(height_pixels);
    }
    catch (std::runtime_error const& error)
    {
        mir::log_warning("%s", error.what());
        return;
    }

    auto const utf32 = convert_utf8_to_utf32(text);

    for (char32_t const glyph : utf32)
    {
        try
        {
            rasterize_glyph(glyph);

            geom::Point glyph_top_left =
                top_left +
                geom::Displacement{
                    face->glyph->bitmap_left,
                    height_pixels.as_int() - face->glyph->bitmap_top};
            render_glyph(buf, buf_size, &face->glyph->bitmap, glyph_top_left, colour);

            top_left += geom::Displacement{
                face->glyph->advance.x / 64,
                face->glyph->advance.y / 64};
        }
        catch (std::runtime_error const& error)
        {
            mir::log_warning("%s", error.what());
        }
    }
}

void TextRenderer::set_char_size(geom::Height height) const
{
    if (auto const error = FT_Set_Pixel_Sizes(face, 0, height.as_int()))
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(
            "Setting char size failed with error " + std::to_string(error)));
    }
}

void TextRenderer::rasterize_glyph(char32_t glyph) const
{
    auto const glyph_index = FT_Get_Char_Index(face, glyph);

    if (auto const error = FT_Load_Glyph(face, glyph_index, 0))
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(
            "Failed to load glyph " + std::to_string(glyph_index)));
    }

    if (auto const error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(
            "Failed to render glyph " + std::to_string(glyph_index)));
    }
}

void TextRenderer::render_glyph(
    Pixel* buf,
    geom::Size buf_size,
    FT_Bitmap const* glyph,
    geom::Point top_left,
    Pixel colour) const
{
    geom::X const buffer_left = std::max(top_left.x, geom::X{});
    geom::X const buffer_right = std::min(top_left.x + geom::DeltaX{glyph->width}, as_x(buf_size.width));

    geom::Y const buffer_top = std::max(top_left.y, geom::Y{});
    geom::Y const buffer_bottom = std::min(top_left.y + geom::DeltaY{glyph->rows}, as_y(buf_size.height));

    geom::Displacement const glyph_offset = as_displacement(top_left);

    for (geom::Y buffer_y = buffer_top; buffer_y < buffer_bottom; buffer_y += geom::DeltaY{1})
    {
        geom::Y const glyph_y = buffer_y - glyph_offset.dy;
        uint8_t* const glyph_row = glyph->buffer + (glyph_y.as_int() * glyph->pitch);
        Pixel* const buffer_row = buf + (buffer_y.as_int() * buf_size.width.as_int());

        for (geom::X buffer_x = buffer_left; buffer_x < buffer_right; buffer_x += geom::DeltaX{1})
        {
            geom::X const glyph_x = buffer_x - glyph_offset.dx;
            uint8_t const glyph_alpha = ((int)glyph_row[glyph_x.as_int()] * colour.a) / 255;
            Pixel* buffer_pixel = buffer_row + buffer_x.as_int();
            for (int i = 0; i < 3; i++)
            {
                // Blend color with the previous buffer color based on the glyph's alpha
                (*buffer_pixel)[i] = ((*buffer_pixel)[i] * (255 - glyph_alpha)) / 255 +
                    (colour[i] * glyph_alpha) / 255;
            }
        }
    }
}

auto TextRenderer::get_font_path() -> std::string
{
    auto const path = default_font();
    if (path.empty())
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to find a font"));
    }
    return path;
}

FileObserver::FileObserver(Path file_path)
    : file_path{file_path}
{
    // Remove old log (if existant)
    remove(file_path.c_str());

    fd = inotify_init();
    if (fd < 0)
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(
            "Initializing inotify failed with error " + std::to_string(fd)));
    }

    // Set watch on parent path as watching for file directly
    // Doesn't work if the file exists before execution
    wd = inotify_add_watch(
        fd,
        file_path.parent_path().c_str(),
        IN_CREATE
    );
}

FileObserver::~FileObserver()
{
    inotify_rm_watch(fd, wd);
    close(fd);
}

auto FileObserver::file_exists() -> bool
{
    inotify_event buffer[BUF_LEN];
    read(fd, buffer, BUF_LEN);

    if (buffer->len 
        && buffer->mask & IN_CREATE
        && buffer->name == file_path.filename())
    {
        return true;
    }
    return false;
}