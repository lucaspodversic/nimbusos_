use crate::signatures::SignatureDb;
pub fn scan_bytes(db: &SignatureDb, data: &[u8]) -> bool {
    db.matches(data)
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn detect_eicar_like() {
        let mut db = SignatureDb::new();
        db.add(b"EICAR");
        assert!(scan_bytes(&db, b"....EICAR...."));
        assert!(!scan_bytes(&db, b"clean"));
    }
}
