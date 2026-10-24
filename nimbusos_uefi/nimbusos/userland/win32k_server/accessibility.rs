//! Accesibilidad básica (UI Automation-lite stub).

#[derive(Clone, Debug)]
pub struct AccNode {
    pub name: String,
    pub role: String,
    pub children: Vec<AccNode>,
}

impl AccNode {
    pub fn window(title: &str) -> Self {
        Self { name: title.into(), role: "window".into(), children: vec![] }
    }
}
