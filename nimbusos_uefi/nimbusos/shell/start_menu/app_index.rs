//! Índice de aplicaciones instaladas (sin telemetría).

#[derive(Clone, Debug)]
pub struct AppEntry {
    pub name: String,
    pub exe: String,
    pub app_id: String,
}

#[derive(Default, Debug)]
pub struct AppIndex { pub apps: Vec<AppEntry> }

impl AppIndex {
    pub fn seed() -> Self {
        Self {
            apps: vec![
                AppEntry { name: "Explorador de archivos".into(), exe: "explorer.exe".into(), app_id: "explorer".into() },
                AppEntry { name: "Configuración".into(), exe: "settings.exe".into(), app_id: "settings".into() },
                AppEntry { name: "Bloc de notas".into(), exe: "notepad.exe".into(), app_id: "notepad".into() },
                AppEntry { name: "Símbolo del sistema".into(), exe: "cmd.exe".into(), app_id: "cmd".into() },
            ],
        }
    }
    pub fn search(&self, q: &str) -> Vec<&AppEntry> {
        let q = q.to_lowercase();
        self.apps.iter().filter(|a| a.name.to_lowercase().contains(&q) || a.app_id.contains(&q)).collect()
    }
}
