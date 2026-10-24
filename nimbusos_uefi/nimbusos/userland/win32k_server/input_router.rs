//! Enruta mouse/teclado a la ventana bajo el cursor / con foco.

use crate::window_manager::{Hwnd, WindowManager};
use crate::hit_testing::hit_test;

#[derive(Clone, Copy, Debug)]
pub enum InputEvent {
    MouseMove { x: i32, y: i32 },
    MouseDown { x: i32, y: i32, button: u8 },
    MouseUp { x: i32, y: i32, button: u8 },
    KeyDown { vk: u32 },
    KeyUp { vk: u32 },
}

#[derive(Default, Debug)]
pub struct InputRouter {
    pub focus: Option<Hwnd>,
    pub hover: Option<Hwnd>,
}

impl InputRouter {
    pub fn dispatch(&mut self, wm: &WindowManager, ev: InputEvent) -> Option<Hwnd> {
        match ev {
            InputEvent::MouseMove { x, y } | InputEvent::MouseDown { x, y, .. } | InputEvent::MouseUp { x, y, .. } => {
                let target = hit_test(wm, x, y);
                self.hover = target;
                if matches!(ev, InputEvent::MouseDown { .. }) {
                    self.focus = target;
                }
                target
            }
            InputEvent::KeyDown { .. } | InputEvent::KeyUp { .. } => self.focus,
        }
    }
}
