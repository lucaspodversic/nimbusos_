//! Bandeja del sistema (iconos de notificación).

#[derive(Clone, Debug)]
pub struct TrayIcon { pub id: u32, pub tooltip: String }

#[derive(Default, Debug)]
pub struct Tray { pub icons: Vec<TrayIcon> }

impl Tray {
    pub fn add(&mut self, id: u32, tooltip: &str) {
        self.icons.push(TrayIcon { id, tooltip: tooltip.into() });
    }
    pub fn remove(&mut self, id: u32) { self.icons.retain(|i| i.id != id); }
}
