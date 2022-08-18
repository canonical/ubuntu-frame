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

#include "egfullscreenclient.h"
#include "startup_client.h"

#include "mir/log.h"

#include <chrono>
#include <cstring>
#include <codecvt>
#include <sstream>
#include <thread>

#include <boost/throw_exception.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>

struct StartupClient::Self : egmde::FullscreenClient
{
public:
    using Path = boost::filesystem::path;

    Self(
        wl_display* display,
        Colour* wallpaper_top_colour,
        Colour* wallpaper_bottom_colour,
        Colour* crash_background_colour,
        Colour* crash_text_colour,
        Path diagnostic_path,
        Path font_path,
        uint font_size,
        uint sleep_time);

    void draw_screen(SurfaceInfo& info) const override;
    
    void render_text(int32_t width, int32_t height, Colour* buffer) const;

    Colour* const wallpaper_top_colour;
    Colour* const wallpaper_bottom_colour;
    Colour* const crash_background_colour;
    Colour* const crash_text_colour;

    uint sleep_time;

    mutable SurfaceInfo* current_surface_info;
    TextRenderer text_renderer;
    const Path diagnostic_path;

private:
    void draw() const;
    void draw_on_diag_update();

    uint font_size;

    FileObserver file_observer;
    std::thread file_observer_thread;

    std::mutex mutable buffer_mutex;
    std::atomic<bool> diag_exists = false;
};

void StartupClient::set_colour(std::string const& option, Colour* colour)
{
    uint32_t value;
    std::stringstream interpreter{option};

    if (interpreter >> std::hex >> value)
    {
        colour[0] = value & 0xFF;
        colour[1] = (value >> 8) & 0xFF;
        colour[2] = (value >> 16) & 0xFF;
        // alpha should always be 255 in this instance
        colour[3] = 255;
    }
    else
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(
            "Invalid colour (" + option + ") given in program argument"));
    }
}

void StartupClient::set_wallpaper_top_colour(std::string const& option)
{
    set_colour(option, wallpaper_top_colour);
}

void StartupClient::set_wallpaper_bottom_colour(std::string const& option)
{
    set_colour(option, wallpaper_bottom_colour);
}

void StartupClient::set_crash_background_colour(std::string const& option)
{
    set_colour(option, crash_background_colour);
}

void StartupClient::set_crash_text_colour(std::string const& option)
{
    set_colour(option, crash_text_colour);
}

void StartupClient::set_diagnostic_path(std::string const& option)
{
    auto path_string = option;
    boost::replace_all(path_string, "$USER", getenv("USER"));

    auto const path = boost::filesystem::path(path_string);
    if (boost::filesystem::exists(path.parent_path()))
    {
        diagnostic_path = path;
    }
    else
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(
            "Log file directory (" + path.parent_path().string() + ") does not exist"));
    }
}

void StartupClient::set_font_path(std::string const& option)
{
    auto path_string = option;
    boost::replace_all(path_string, "$USER", getenv("USER"));

    auto const path = boost::filesystem::path(path_string);
    if (boost::filesystem::exists(path.parent_path()))
    {
        font_path = path;
    }
    else
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(
            "Font directory (" + path.parent_path().string() + ") does not exist"));
    }


    if(boost::filesystem::exists(path))
    {
        puts("YAAY IT EXISTS");
    } else {
        puts("IT DONT EXIST");
    }
}

void StartupClient::set_font_size(std::string const& option)
{
    font_size = std::stoi(option);
}

void StartupClient::set_sleep_time(std::string const& option)
{
    sleep_time = std::stoi(option);
}

void StartupClient::render_background(
        int32_t width, 
        int32_t height, 
        Colour* buffer, 
        Colour const* bottom_colour, 
        Colour const* top_colour)
{
    Colour new_pixel[4];

    for (auto current_y = 0; current_y < height; current_y++)
    {
        // Render gradient
        for (auto i = 0; i < 3; i++)
        {
            new_pixel[i] = (current_y * bottom_colour[i] + (height - current_y) * top_colour[i]) / height;
        }
        new_pixel[3] = 0xFF;

        // Copy new_pixel to buffer
        auto const pixel_size = sizeof(new_pixel);
        for (auto current_x = 0; current_x < pixel_size*width; current_x += pixel_size)
        {
            memcpy(buffer + current_x, new_pixel, pixel_size);
        }

        // Move pointer to next pixel
        buffer += 4*width;
    }
}

void StartupClient::render_background(int32_t width, int32_t height, Colour* buffer, Colour const* colour)
{
    render_background(width, height, buffer, colour, colour);
}

void StartupClient::operator()(wl_display* display)
{
    auto client = std::make_shared<Self>(
        display, 
        wallpaper_top_colour, 
        wallpaper_bottom_colour, 
        crash_background_colour,
        crash_text_colour,
        diagnostic_path,
        font_path,
        font_size,
        sleep_time);
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

void StartupClient::operator()(std::weak_ptr<mir::scene::Session> const& /*session*/)
{
}

StartupClient::Self::Self(
    wl_display* display, 
    Colour* wallpaper_top_colour,
    Colour* wallpaper_bottom_colour, 
    Colour* crash_background_colour, 
    Colour* crash_text_colour,
    Path diagnostic_path,
    Path font_path,
    uint font_size,
    uint sleep_time)
    : FullscreenClient(display),
      wallpaper_top_colour{wallpaper_top_colour},
      wallpaper_bottom_colour{wallpaper_bottom_colour},
      crash_background_colour{crash_background_colour},
      crash_text_colour{crash_text_colour},
      text_renderer{TextRenderer(font_path)},
      diagnostic_path{diagnostic_path},
      font_size{font_size},
      sleep_time{sleep_time},
      file_observer{FileObserver(diagnostic_path)}
{
    wl_display_roundtrip(display);
    wl_display_roundtrip(display);

    file_observer_thread = std::thread(&StartupClient::Self::draw_on_diag_update, this);
}

void StartupClient::Self::render_text(
    int32_t width, 
    int32_t height,
    Colour* buffer) const
{
    auto size = geom::Size{width, height};
    auto top_left = geom::Point{0, 0};
    auto const height_pixels = geom::Height(font_size);
    auto const y_kerning = height_pixels + (height_pixels / 5);
    
    std::string line;
    auto stream = boost::filesystem::ifstream(diagnostic_path);
    while (getline(stream, line))
    {
        text_renderer.render(buffer, size, line, top_left, height_pixels, crash_text_colour);
        auto const new_top_left = geom::Point{top_left.x, top_left.y.as_value() + y_kerning.as_value()};
        top_left = new_top_left;
    }
}

void StartupClient::Self::draw_screen(SurfaceInfo& info) const
{
    current_surface_info = &info;
    draw();
}

void StartupClient::Self::draw() const
{
    std::lock_guard lock{buffer_mutex};

    bool const rotated = current_surface_info->output->transform & WL_OUTPUT_TRANSFORM_90;
    auto const width = rotated ? current_surface_info->output->height : current_surface_info->output->width;
    auto const height = rotated ? current_surface_info->output->width : current_surface_info->output->height;

    if (width <= 0 || height <= 0)
        return;

    auto const stride = 4*width;

    if (!current_surface_info->surface)
    {
        current_surface_info->surface = wl_compositor_create_surface(compositor);
    }

    if (!current_surface_info->shell_surface)
    {
        current_surface_info->shell_surface = wl_shell_get_shell_surface(shell, current_surface_info->surface);
        wl_shell_surface_set_fullscreen(
            current_surface_info->shell_surface,
            WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
            0,
            current_surface_info->output->output);
    }

    if (current_surface_info->buffer)
    {
        wl_buffer_destroy(current_surface_info->buffer);
    }

    {
        auto const shm_pool = make_shm_pool(stride * height, &current_surface_info->content_area);

        current_surface_info->buffer = wl_shm_pool_create_buffer(
            shm_pool.get(),
            0,
            width, height, stride,
            WL_SHM_FORMAT_ARGB8888);
    }

    auto buffer = static_cast<Colour*>(current_surface_info->content_area);

    if (diag_exists)
    {
        render_background(width, height, buffer, crash_background_colour);
        render_text(width, height, buffer);

        wl_surface_attach(current_surface_info->surface, current_surface_info->buffer, 0, 0);
        wl_surface_set_buffer_scale(current_surface_info->surface, current_surface_info->output->scale_factor);
        wl_surface_commit(current_surface_info->surface);
        wl_display_flush(display);
    }
    else
    {
        render_background(width, height, buffer, wallpaper_bottom_colour, wallpaper_top_colour);

        wl_surface_attach(current_surface_info->surface, current_surface_info->buffer, 0, 0);
        wl_surface_set_buffer_scale(current_surface_info->surface, current_surface_info->output->scale_factor);
        wl_surface_commit(current_surface_info->surface);
    }
}

void StartupClient::Self::draw_on_diag_update()
{
    if (!diag_exists)
    {
        file_observer.wait_for_create();
        diag_exists = true;
        draw();
    }

    while (true)
    {
        if (file_observer.file_updated())
        {
            draw();
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
    }
}

void StartupClient::stop()
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

TextRenderer::TextRenderer(Path font_path)
    : font_path{font_path}
{
    if (auto const error = FT_Init_FreeType(&library))
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(
            "Initializing freetype library failed with error " + std::to_string(error)));
    }
    
    if (auto const error = FT_New_Face(library, font_path.c_str(), 0, &face))
    {
        if (error == FT_Err_Unknown_File_Format)
        {
            BOOST_THROW_EXCEPTION(std::runtime_error(
                "Font " + font_path.string() + " has unsupported format"));
        }
            
        else
        {
            BOOST_THROW_EXCEPTION(std::runtime_error(
                "Loading font from " + font_path.string() + " failed with error " + std::to_string(error)));
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
    Colour* buf,
    geom::Size buf_size,
    std::string const& text,
    geom::Point top_left,
    geom::Height height_pixels,
    Colour* colour) const
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
    Colour* buffer,
    geom::Size buf_size,
    FT_Bitmap const* glyph,
    geom::Point top_left,
    Colour* colour) const
{
    geom::X const buffer_left = std::max(top_left.x, geom::X{});
    geom::X const buffer_right = std::min(top_left.x + geom::DeltaX{glyph->width}, as_x(buf_size.width));

    geom::Y const buffer_top = std::max(top_left.y, geom::Y{});
    geom::Y const buffer_bottom = std::min(top_left.y + geom::DeltaY{glyph->rows}, as_y(buf_size.height));

    geom::Displacement const glyph_offset = as_displacement(top_left);

    auto const colour_alpha = colour[3];
    
    auto buffer_pixels = reinterpret_cast<uint32_t*>(buffer);

    for (geom::Y buffer_y = buffer_top; buffer_y < buffer_bottom; buffer_y += geom::DeltaY{1})
    {
        geom::Y const glyph_y = buffer_y - glyph_offset.dy;
        Colour* const glyph_row = glyph->buffer + glyph_y.as_int() * glyph->pitch;
        uint32_t* const buffer_row = buffer_pixels + buffer_y.as_int() * buf_size.width.as_int();

        for (geom::X buffer_x = buffer_left; buffer_x < buffer_right; buffer_x += geom::DeltaX{1})
        {
            geom::X const glyph_x = buffer_x - glyph_offset.dx;
            unsigned char const glyph_alpha = (static_cast<Colour>(glyph_row[glyph_x.as_int()]) * colour_alpha) / 255;
            auto* const buffer_pixels = reinterpret_cast<Colour*>(buffer_row + buffer_x.as_int());
            for (int i = 0; i < 3; i++)
            {
                // Blend colour with the previous buffer colour based on the glyph's alpha
                buffer_pixels[i] =
                    (buffer_pixels[i] * (255 - glyph_alpha)) / 255 +
                    (colour[i] * glyph_alpha) / 255;
            }
        }
    }

}

FileObserver::FileObserver(Path file_path)
    : file_path{file_path}
{
    fd = inotify_init();
    if (fd < 0)
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(
            "Initializing inotify failed with error " + std::to_string(fd)));
    }

    // Set watch on parent path as watching for file directly
    // has some complicated behavior
    wd = inotify_add_watch(
        fd,
        file_path.parent_path().c_str(),
        IN_CREATE | IN_CLOSE_WRITE
    );
}

FileObserver::~FileObserver()
{
    inotify_rm_watch(fd, wd);
    close(fd);
}

auto FileObserver::file_exists() -> bool
{
    read(fd, buffer, BUF_LEN);
    if (buffer->len)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void FileObserver::wait_for_create()
{
    while (true)
    {
        if (file_exists())
        {
            return;
        }
    }
}

auto FileObserver::file_updated() -> bool
{
    read(fd, buffer, BUF_LEN);

    if (buffer->mask & IN_CREATE | IN_CLOSE_WRITE
        && buffer->name == file_path.filename())
    {
        return true;
    }
    
    return false;
}