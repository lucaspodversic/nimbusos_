//! Página Sistema de Configuración.
#[derive(Clone, Debug)]
pub struct SystemInfo {
    pub os_name: String,
    pub version: String,
    pub ram_mb: u32,
    pub cpu: String,
}
impl Default for SystemInfo {
    fn default() -> Self {
        Self {
            os_name: "NimbusOS".into(),
            version: "0.1.0".into(),
            ram_mb: 1024,
            cpu: "x86_64".into(),
        }
    }
}
