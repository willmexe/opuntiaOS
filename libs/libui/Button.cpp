/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libui/Button.h>
#include <libui/Context.h>
#include <libfoundation/EventLoop.h>

namespace UI {

Button::Button(const LG::Rect& frame)
    : View(frame, std_background_color())
{
}

void Button::display(const LG::Rect& rect)
{
    Context ctx(*this);

    ctx.set_fill_color(background_color());
    ctx.fill(bounds());

    LG::Point<int> text_start { content_edge_insets().left(), content_edge_insets().top() };

    auto& f = font();
    ctx.set_fill_color(title_color());
    for (int i = 0; i < m_title.size(); i++) {
        ctx.draw(text_start, f.glyph_bitmap(m_title[i]));
        text_start.offset_by(f.glyph_width(m_title[i]) + 2, 0);
    }
}

void Button::hover_begin(const LG::Point<int>& location)
{
    m_background_color_storage = background_color();
    set_background_color(background_color().darken(8));
}

void Button::hover_end()
{
    set_background_color(m_background_color_storage);
}

void Button::recalc_bounds()
{
    size_t new_width = text_width() + content_edge_insets().left() + content_edge_insets().right();
    size_t new_height = text_height() + content_edge_insets().top() + content_edge_insets().bottom();
    set_width(new_width);
    set_height(new_height);
}

size_t Button::text_width()
{
    size_t width = 0;
    auto& f = font();
    for (int i = 0; i < m_title.size(); i++) {
        width += f.glyph_width(m_title[i]) + 2; // FIXME
    }
    return width;
}

size_t Button::text_height() const
{
    return font().glyph_height();
}

}