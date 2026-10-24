//! Barra de tareas: iconos anclados, ventanas abiertas, reloj.

#[derive(Clone, Debug)]
pub struct TaskItem {
    pub app_id: String,
    pub title: String,
    pub hwnd: u32,
}

#[derive(Default, Debug)]
pub struct Taskbar {
    pub pinned: Vec<String>,
    pub open: Vec<TaskItem>,
    pub height_px: u32,
}

impl Taskbar {
    pub fn new() -> Self { Self { pinned: vec![], open: vec![], height_px: 48 } }
    pub fn pin(&mut self, app_id: &str) {
        if !self.pinned.iter().any(|p| p == app_id) {
            self.pinned.push(app_id.into());
        }
    }
    pub fn unpin(&mut self, app_id: &str) {
        self.pinned.retain(|p| p != app_id);
    }
    pub fn register_window(&mut self, app_id: &str, title: &str, hwnd: u32) {
        self.open.push(TaskItem { app_id: app_id.into(), title: title.into(), hwnd });
    }
}
