//! Panel de navegación (Este equipo, Acceso rápido).

#[derive(Clone, Debug)]
pub struct NavNode { pub label: String, pub path: String }

pub fn default_tree() -> Vec<NavNode> {
    vec![
        NavNode { label: "Acceso rápido".into(), path: "shell:QuickAccess".into() },
        NavNode { label: "Este equipo".into(), path: "C:\\".into() },
        NavNode { label: "Documentos".into(), path: "C:\\Users\\Documents".into() },
        NavNode { label: "Descargas".into(), path: "C:\\Users\\Downloads".into() },
    ]
}
