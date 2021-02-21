/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "Compositor.h"
#include "Connection.h"
#include "Screen.h"
#include "WSConnection.h"
#include "WSEvent.h"
#include "WSServerDecoder.h"
#include "Window.h"
#include <libfoundation/EventLoop.h>
#include <libfoundation/EventReceiver.h>
#include <libipc/ServerConnection.h>
#include <std/LinkedList.h>
#include <std/Vector.h>
#include <syscalls.h>

class WindowManager : public LFoundation::EventReceiver {
public:
    static WindowManager& the();
    WindowManager();

    inline void add_window(Window* window)
    {
        if (window->type() == WindowType::Dock) {
            setup_dock(window);
        }
        m_windows.push_back(window);
        bring_to_front(*window);
        notify_window_status_changed(window->id(), WindowStatusUpdateType::Created);
    }

    inline void remove_window(Window* window)
    {
        if (movable_window().ptr() == window) {
            m_movable_window = nullptr;
        }
        if (active_window().ptr() == window) {
            m_active_window = nullptr;
        }
        if (hovered_window().ptr() == window) {
            m_hovered_window = nullptr;
        }
        m_windows.remove(window);
        m_compositor.invalidate(window->bounds());
        notify_window_status_changed(window->id(), WindowStatusUpdateType::Removed);
        delete window;
    }

    void close_window(Window& window)
    {
        m_event_loop.add(m_connection, new SendEvent(new WindowCloseRequestMessage(window.connection_id(), window.id())));
    }

    inline Window* window(int id)
    {
        for (auto& window : m_windows) {
            if (window.id() == id) {
                return &window;
            }
        }
        return nullptr;
    }

    inline void do_bring_to_front(Window& window)
    {
        auto* window_ptr = &window;
        m_windows.remove(window_ptr);
        m_windows.push_front(window_ptr);
    }

    Window* get_top_standard_window_in_view() const
    {
        auto* prev_window = m_windows.head();
        if (m_dock_window) {
            if (prev_window) {
                return prev_window->next();
            }
        }
        return prev_window;
    }

    void bring_to_front(Window& window)
    {
        auto* prev_window = get_top_standard_window_in_view();
        do_bring_to_front(window);
        if (m_dock_window) {
            do_bring_to_front(*m_dock_window);
        }
        window.frame().set_active(true);
        m_compositor.invalidate(window.bounds());
        if (prev_window && prev_window->id() != window.id()) {
            prev_window->frame().set_active(false);
            prev_window->frame().invalidate(m_compositor);
        }
    }

    inline LinkedList<Window>& windows() { return m_windows; }
    inline const LinkedList<Window>& windows() const { return m_windows; }
    inline int next_win_id() { return ++m_next_win_id; }

    void receive_event(UniquePtr<LFoundation::Event> event) override;

    inline int mouse_x() const { return m_mouse_x; }
    inline int mouse_y() const { return m_mouse_y; }
    inline bool is_mouse_left_button_pressed() const { return m_mouse_left_button_pressed; }

    void setup_dock(Window* window);

    // Notifiers
    bool notify_listner_about_window_status(const Window& window, int changed_window_id, WindowStatusUpdateType type);
    bool notify_listner_about_changed_icon(const Window&, int changed_window_id);

    void notify_window_status_changed(int changed_window_id, WindowStatusUpdateType type);
    void notify_window_icon_changed(int changed_window_id);

private:
    void start_window_move(Window& window);
    bool continue_window_move(MouseEvent* mouse_event);

    void update_mouse_position(MouseEvent* mouse_event);
    void receive_mouse_event(UniquePtr<LFoundation::Event> event);
    void receive_keyboard_event(UniquePtr<LFoundation::Event> event);

    inline WeakPtr<Window>& movable_window() { return m_movable_window; }
    inline const WeakPtr<Window>& movable_window() const { return m_movable_window; }
    inline WeakPtr<Window>& hovered_window() { return m_hovered_window; }
    inline const WeakPtr<Window>& hovered_window() const { return m_hovered_window; }
    inline WeakPtr<Window>& active_window() { return m_active_window; }
    inline const WeakPtr<Window>& active_window() const { return m_active_window; }

    LinkedList<Window> m_windows;

    Screen& m_screen;
    Connection& m_connection;
    Compositor& m_compositor;
    LFoundation::EventLoop& m_event_loop;

    WeakPtr<Window> m_dock_window {}; // TODO: may be remove it from here?
    WeakPtr<Window> m_movable_window {};
    WeakPtr<Window> m_active_window {};
    WeakPtr<Window> m_hovered_window {};
    int m_next_win_id { 0 };

    int m_mouse_x { 0 };
    int m_mouse_y { 0 };
    bool m_mouse_left_button_pressed { false };
    bool m_mouse_changed_button_status { false };
};