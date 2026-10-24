//! UI de búsqueda del menú Inicio (solo local).

use super::app_index::AppIndex;

pub fn query(index: &AppIndex, text: &str) -> Vec<String> {
    index.search(text).into_iter().map(|a| a.name.clone()).collect()
}
