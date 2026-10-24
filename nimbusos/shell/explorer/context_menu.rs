//! Menú contextual del explorador.

#[derive(Clone, Debug)]
pub struct MenuItem { pub id: u32, pub label: String }

pub fn file_menu(is_dir: bool) -> Vec<MenuItem> {
    let mut m = vec![
        MenuItem { id: 1, label: "Abrir".into() },
        MenuItem { id: 2, label: "Cortar".into() },
        MenuItem { id: 3, label: "Copiar".into() },
        MenuItem { id: 4, label: "Eliminar".into() },
        MenuItem { id: 5, label: "Propiedades".into() },
    ];
    if is_dir {
        m.insert(1, MenuItem { id: 6, label: "Abrir en nueva pestaña".into() });
    }
    m
}
