//! Gestor de ventanas: create/destroy/move/resize.

use std::collections::HashMap;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct Hwnd(pub u32);

#[derive(Clone, Debug)]
pub struct Window {
    pub hwnd: Hwnd,
    pub title: String,
    pub x: i32, pub y: i32, pub w: i32, pub h: i32,
    pub visible: bool,
    pub parent: Option<Hwnd>,
}

#[derive(Default, Debug)]
pub struct WindowManager {
    next: u32,
    pub windows: HashMap<Hwnd, Window>,
}

impl WindowManager {
    pub fn new() -> Self { Self { next: 1, windows: HashMap::new() } }
    pub fn create(&mut self, title: &str, x: i32, y: i32, w: i32, h: i32) -> Hwnd {
        let hwnd = Hwnd(self.next); self.next += 1;
        self.windows.insert(hwnd, Window {
            hwnd, title: title.into(), x, y, w, h, visible: true, parent: None,
        });
        hwnd
    }
    pub fn destroy(&mut self, hwnd: Hwnd) -> bool { self.windows.remove(&hwnd).is_some() }
    pub fn move_resize(&mut self, hwnd: Hwnd, x: i32, y: i32, w: i32, h: i32) -> bool {
        if let Some(win) = self.windows.get_mut(&hwnd) {
            win.x = x; win.y = y; win.w = w; win.h = h; true
        } else { false }
    }
    pub fn set_title(&mut self, hwnd: Hwnd, title: &str) -> bool {
        if let Some(win) = self.windows.get_mut(&hwnd) {
            win.title = title.into(); true
        } else { false }
    }
}
