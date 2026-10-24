//! Menú Inicio: anclados + recomendados + energía.

#[derive(Default, Debug)]
pub struct StartMenu {
    pub open: bool,
    pub pinned: Vec<String>,
}

impl StartMenu {
    pub fn toggle(&mut self) { self.open = !self.open; }
    pub fn pin(&mut self, app: &str) {
        if !self.pinned.iter().any(|a| a == app) { self.pinned.push(app.into()); }
    }
}
