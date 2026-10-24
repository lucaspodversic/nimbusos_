//! Binario del window server (host: loop vacío demostrativo).
//! NimbusOS win32k_server — clean-room.

use win32k_server::window_manager::WindowManager;
use win32k_server::clipboard_server::Clipboard;
use win32k_server::focus_z_order::ZOrder;
use win32k_server::input_router::{InputEvent, InputRouter};
use win32k_server::session_desktop::Session;

fn main() {
    let mut wm = WindowManager::new();
    let mut z = ZOrder::default();
    let mut input = InputRouter::default();
    let mut clip = Clipboard::default();
    let mut session = Session::new(1);

    let hwnd = wm.create("NimbusOS Shell", 0, 0, 1280, 800);
    z.insert(hwnd);
    session.active_desktop().windows.push(hwnd);
    clip.set(Clipboard::CF_TEXT, b"hola".to_vec());

    let _ = input.dispatch(&wm, InputEvent::MouseDown { x: 10, y: 10, button: 1 });
    println!("win32k_server listo: hwnd={}, focus={:?}, clip_seq={}",
             hwnd.0, input.focus, clip.sequence);
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn create_and_hit() {
        let mut wm = WindowManager::new();
        let h = wm.create("t", 50, 50, 100, 100);
        let mut ir = InputRouter::default();
        let t = ir.dispatch(&wm, InputEvent::MouseDown { x: 60, y: 60, button: 1 });
        assert_eq!(t, Some(h));
        assert_eq!(ir.focus, Some(h));
    }
}
