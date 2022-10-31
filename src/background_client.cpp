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
#include "background_client.h"

#include "mir/log.h"

#include <chrono>
#include <cstring>
#include <codecvt>
#include <sstream>
#include <thread>

#include <boost/throw_exception.hpp>
#include <boost/filesystem.hpp>

#include <mir/fatal.h>

using Path = boost::filesystem::path;

namespace
{
auto get_font_path() -> Path
{
    static auto const ubuntu_font = "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf";
    if (auto const snap = getenv("SNAP"))
    {
        return Path(snap).append(ubuntu_font);
    }

    return ubuntu_font;
}
} // namespace

BackgroundClient::BackgroundClient(miral::MirRunner* runner, WindowManagerObserver* window_manager_observer)
: runner{runner},
  window_manager_observer{window_manager_observer}
{
}

struct BackgroundClient::Self : egmde::FullscreenClient
{
public:
    Self(
        wl_display* display,
        miral::MirRunner* runner,
        WindowManagerObserver* window_manager_observer,
        Colour const& wallpaper_top_colour,
        Colour const& wallpaper_bottom_colour,
        Colour const& crash_background_colour,
        Colour const& crash_text_colour,
        std::optional<Path> diagnostic_path,
        uint diagnostic_delay);

    void draw_screen(SurfaceInfo& info, bool draws_crash) const override;
    
    void render_text(uint32_t width, uint32_t height, unsigned char* buffer) const;

    Colour const& wallpaper_top_colour;
    Colour const& wallpaper_bottom_colour;
    Colour const& crash_background_colour;
    Colour const& crash_text_colour;

    TextRenderer text_renderer;
    const std::optional<Path> diagnostic_path;

private:
    miral::MirRunner* const runner;

    const uint x_margin_percent = 5;
    const uint y_margin_percent = 5;

    std::mutex mutable buffer_mutex;
};

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
            auto const error_str = "Loading font from " + font_path.string() + " failed with error " + std::to_string(error);
            mir::fatal_error(error_str.c_str());
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

void BackgroundClient::set_colour(std::string const& option, Colour& colour)
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

void BackgroundClient::set_wallpaper_top_colour(std::string const& option)
{
    set_colour(option, wallpaper_top_colour);
}

void BackgroundClient::set_wallpaper_bottom_colour(std::string const& option)
{
    set_colour(option, wallpaper_bottom_colour);
}

void BackgroundClient::set_crash_background_colour(std::string const& option)
{
    set_colour(option, crash_background_colour);
}

void BackgroundClient::set_crash_text_colour(std::string const& option)
{
    set_colour(option, crash_text_colour);
}

void BackgroundClient::set_diagnostic_path(std::string const& option)
{
    if (option.empty())
    {
        mir::log_info("No diagnostic file given. Crash reporting is disabled.");
        return;
    }
    
    auto const path = boost::filesystem::absolute(option);

    auto formatted_path_error = "\n Inputted path: " + option;
    if (path.string() != option)
    {
        formatted_path_error.append("\n Resolved path: " + path.string());
    }

    if (boost::filesystem::is_directory(path))
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(
            "Target of diagnostic path is a directory when it should be a file." + formatted_path_error));
    }
    
    if (boost::filesystem::is_regular_file(path.parent_path()))
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(
            "Parent of diagnostic path is not a directory." + formatted_path_error));
    }

    diagnostic_path = path;
}

void BackgroundClient::set_diagnostic_delay(int delay)
{
    if (delay >= 0)
    {
        diagnostic_delay = delay;
    }
    else
    {
        BOOST_THROW_EXCEPTION(std::runtime_error(
            "Diagnostic delay time (" + std::to_string(delay) + ") must not be negative"));
    }
}

void BackgroundClient::render_background(
    uint32_t width,
    uint32_t height,
    unsigned char* buffer,
    Colour const& bottom_colour,
    Colour const& top_colour)
{
    Colour new_pixel;

    for (auto current_y = 0; current_y < height; current_y++)
    {
        // Render gradient
        for (auto i = 0; i < 3; i++)
        {
            new_pixel[i] = ((current_y * bottom_colour[i] + (height - current_y) * top_colour[i]) / height);
        }
        new_pixel[3] = 0xFF;

        // Copy new_pixel to buffer
        auto const pixel_size = sizeof(new_pixel);
        for (auto current_x = buffer; current_x != buffer + pixel_size * width; current_x += pixel_size)
        {
            memcpy(current_x, new_pixel, pixel_size);
        }

        // Move pointer to next pixel
        buffer += 4*width;
    }
}

void BackgroundClient::render_background(uint32_t width, uint32_t height, unsigned char* buffer, Colour const& colour)
{
    render_background(width, height, buffer, colour, colour);
}

void BackgroundClient::operator()(wl_display* display)
{
    auto client = std::make_shared<Self>(
        display, 
        runner,
        window_manager_observer,
        wallpaper_top_colour, 
        wallpaper_bottom_colour, 
        crash_background_colour,
        crash_text_colour,
        diagnostic_path,
        diagnostic_delay);
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

void BackgroundClient::operator()(std::weak_ptr<mir::scene::Session> const& /*session*/)
{
}

BackgroundClient::Self::Self(
    wl_display* display,
    miral::MirRunner* runner,
    WindowManagerObserver* window_manager_observer,
    Colour const& wallpaper_top_colour,
    Colour const& wallpaper_bottom_colour,
    Colour const& crash_background_colour,
    Colour const& crash_text_colour,
    std::optional<Path> diagnostic_path,
    uint diagnostic_delay)
    : FullscreenClient(display, diagnostic_path, diagnostic_delay, runner, window_manager_observer),
      runner{runner},
      wallpaper_top_colour{wallpaper_top_colour},
      wallpaper_bottom_colour{wallpaper_bottom_colour},
      crash_background_colour{crash_background_colour},
      crash_text_colour{crash_text_colour},
      text_renderer{TextRenderer(get_font_path())},
      diagnostic_path{diagnostic_path}
{
    wl_display_roundtrip(display);
    wl_display_roundtrip(display);
}

void BackgroundClient::Self::render_text(
    uint32_t width,
    uint32_t height,
    unsigned char* buffer) const
{   
    auto size = geom::Size{width, height};
    
    auto stream = boost::filesystem::ifstream(diagnostic_path.value());

    auto const x_margin = width * (x_margin_percent / 100.0);
    auto const y_margin = height * (y_margin_percent / 100.0);

    auto const x_diff = width - x_margin;
    auto const y_diff = height - y_margin;
    
    auto const max_font_height_by_width = text_renderer.get_max_font_height_by_width(stream, x_diff);
    auto const max_font_height_by_height = text_renderer.get_max_font_height_by_height(stream, y_diff);

    auto const height_pixels = std::min(max_font_height_by_width, max_font_height_by_height);
    auto const line_height = height_pixels + (height_pixels / text_renderer.y_kerning);

    auto const num_lines = text_renderer.get_num_lines(stream);

    auto const x_offset = (width - text_renderer.get_max_line_width(stream, height_pixels)) / 2;
    auto const y_offset = (height - (num_lines * line_height)) / 2;
    auto top_left = geom::Point{x_offset, y_offset};

    std::string line;
    while (getline(stream, line))
    {
        text_renderer.render(buffer, size, line, top_left, geom::Height{height_pixels}, crash_text_colour);
        auto const new_top_left = geom::Point{top_left.x, top_left.y.as_int() + line_height};
        top_left = new_top_left;
    }

    stream.close();
}

void BackgroundClient::Self::draw_screen(SurfaceInfo& info, bool draws_crash) const
{
    std::lock_guard lock{buffer_mutex};

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

    auto buffer = static_cast<unsigned char*>(info.content_area);

    // Don't draw diagnostic background if file is empty or font not found
    bool file_exists;
    if (boost::filesystem::exists(diagnostic_path.value_or("")))
    {
        file_exists = boost::filesystem::file_size(diagnostic_path.value());
    }
    else
    {
        file_exists = false;
    }

    if (draws_crash && file_exists)
    {
        render_background(width, height, buffer, crash_background_colour);
        render_text(width, height, buffer);
    }
    else
    {
        render_background(width, height, buffer, wallpaper_bottom_colour, wallpaper_top_colour);
    }

    wl_surface_attach(info.surface, info.buffer, 0, 0);
    wl_surface_set_buffer_scale(info.surface, info.output->scale_factor);
    wl_surface_commit(info.surface);
}

void BackgroundClient::stop()
{
    if (auto ss = self.lock())
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
    unsigned char* buf,
    geom::Size buf_size,
    std::string const& text,
    geom::Point top_left,
    geom::Height height_pixels,
    Colour const& colour) const
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
        set_char_size(height_pixels.as_int());
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

void TextRenderer::set_char_size(uint32_t height) const
{
    if (auto const error = FT_Set_Pixel_Sizes(face, 0, height))
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
    unsigned char* buffer,
    geom::Size buf_size,
    FT_Bitmap const* glyph,
    geom::Point top_left,
    Colour const& colour) const
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
        unsigned char const* const glyph_row = glyph->buffer + glyph_y.as_int() * glyph->pitch;
        uint32_t* const buffer_row = buffer_pixels + buffer_y.as_int() * buf_size.width.as_int();

        for (geom::X buffer_x = buffer_left; buffer_x < buffer_right; buffer_x += geom::DeltaX{1})
        {
            geom::X const glyph_x = buffer_x - glyph_offset.dx;
            unsigned char const glyph_alpha = (static_cast<unsigned char>(glyph_row[glyph_x.as_int()]) * colour_alpha) / 255;
            auto* const buffer_pixels = reinterpret_cast<unsigned char*>(buffer_row + buffer_x.as_int());
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

auto TextRenderer::get_num_lines(boost::filesystem::ifstream &stream) const -> uint32_t
{
    auto num_lines = 0;

    std::string line;
    while(getline(stream, line))
    {
        num_lines++;
    }

    // Go back to top of stream
    stream.clear();
    stream.seekg(0, std::ios::beg);

    return num_lines;
}

auto TextRenderer::get_line_width(std::string const& line, uint32_t height_pixels) const -> uint32_t
{
    set_char_size(height_pixels);

    auto line_width = 0;
    for (auto const& character : line)
    {
        rasterize_glyph(character);
        auto const glyph = face->glyph;
        line_width = line_width + (glyph->advance.x >> 6);
    }

    return line_width;
}

auto TextRenderer::get_max_line_width(boost::filesystem::ifstream &stream, uint32_t height_pixels) const -> uint32_t
{
    uint32_t max_line_width = 0;

    std::string line;
    while (getline(stream, line))
    {
        max_line_width = std::max(get_line_width(line, height_pixels), max_line_width);
    }

    // Go back to top of stream
    stream.clear();
    stream.seekg(0, std::ios::beg);

    return max_line_width;
}

auto TextRenderer::get_max_font_height_by_width(boost::filesystem::ifstream &stream, uint32_t max_width) const -> uint32_t
{
    auto estimate_font_height = 50;
    auto const width_from_estimate = get_max_line_width(stream, estimate_font_height);

    auto const final_font_height = estimate_font_height * (static_cast<double>(max_width) / width_from_estimate);
    return final_font_height;
}

auto TextRenderer::get_max_font_height_by_height(boost::filesystem::ifstream &stream, uint32_t max_height) const -> uint32_t
{
    auto num_lines = get_num_lines(stream);
    auto estimate_font_height = 50;
    auto const height_from_estimate = get_total_height(num_lines, estimate_font_height);

    auto const final_font_height = estimate_font_height * (static_cast<double>(max_height) / height_from_estimate);
    return final_font_height;
}

auto TextRenderer::get_total_height(uint32_t num_lines, uint32_t height_pixels) const -> uint32_t
{
    return (height_pixels + (height_pixels / y_kerning)) * num_lines;
}
