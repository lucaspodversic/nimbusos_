//! Hit-testing: qué ventana contiene (x,y), respetando z-order implícito por inserción.

use crate::window_manager::{Hwnd, WindowManager};

pub fn hit_test(wm: &WindowManager, x: i32, y: i32) -> Option<Hwnd> {
    // Última ventana creada encima (simple); win32k real usaría z-order.
    let mut hit = None;
    for (hwnd, w) in &wm.windows {
        if !w.visible { continue; }
        if x >= w.x && y >= w.y && x < w.x + w.w && y < w.y + w.h {
            hit = Some(*hwnd);
        }
    }
    hit
}
