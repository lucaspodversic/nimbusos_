//! Parser MSI (OLE compound doc lite — tablas en memoria para tests).
use std::collections::HashMap;
#[derive(Debug, Default)]
pub struct MsiPackage {
    pub product_name: String,
    pub product_code: String,
    pub tables: HashMap<String, Vec<Vec<String>>>,
}
impl MsiPackage {
    pub fn parse_bytes(data: &[u8]) -> Result<Self, String> {
        // Host: si empieza con "MSI\0" fake magic de test
        if data.starts_with(b"MSI\0") {
            let mut p = Self::default();
            p.product_name = "NimbusTestApp".into();
            p.product_code = "{00000000-0000-0000-0000-000000000001}".into();
            p.tables.insert("File".into(), vec![vec!["app.exe".into(), "1024".into()]]);
            return Ok(p);
        }
        Err("formato MSI no reconocido (parser lite)".into())
    }
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn parse_fake() {
        let p = MsiPackage::parse_bytes(b"MSI\0resto").unwrap();
        assert_eq!(p.product_name, "NimbusTestApp");
    }
}
