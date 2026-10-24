//! Explorador de archivos nativo.

use super::tabs::TabStrip;

#[derive(Debug)]
pub struct Explorer {
    pub cwd: String,
    pub tabs: TabStrip,
    pub show_hidden: bool,
}

impl Explorer {
    pub fn new() -> Self {
        let mut tabs = TabStrip::new();
        tabs.open("C:\\");
        Self { cwd: "C:\\".into(), tabs, show_hidden: false }
    }
    pub fn navigate(&mut self, path: &str) {
        self.cwd = path.into();
        if let Some(t) = self.tabs.active_mut() {
            t.path = path.into();
        }
    }
}
