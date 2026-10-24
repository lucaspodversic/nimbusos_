//! Z-order y foco.

use crate::window_manager::Hwnd;

#[derive(Default, Debug)]
pub struct ZOrder {
    /// Índice 0 = fondo; último = topmost.
    pub stack: Vec<Hwnd>,
}

impl ZOrder {
    pub fn bring_to_front(&mut self, hwnd: Hwnd) {
        self.stack.retain(|h| *h != hwnd);
        self.stack.push(hwnd);
    }
    pub fn insert(&mut self, hwnd: Hwnd) {
        if !self.stack.contains(&hwnd) { self.stack.push(hwnd); }
    }
    pub fn remove(&mut self, hwnd: Hwnd) {
        self.stack.retain(|h| *h != hwnd);
    }
    pub fn top(&self) -> Option<Hwnd> { self.stack.last().copied() }
}
