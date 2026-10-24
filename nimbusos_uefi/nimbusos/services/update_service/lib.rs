//! Update service A/B.
#![allow(dead_code)]
#[path = "check.rs"] mod check;
#[path = "delta_download.rs"] mod delta_download;
#[path = "apply_ab.rs"] mod apply_ab;
#[path = "hotpatch.rs"] mod hotpatch;
#[path = "rollback.rs"] mod rollback;
pub use check::*;
pub use apply_ab::*;
pub use rollback::*;

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn ab_slot_toggle() {
        let mut s = AbState::new();
        assert_eq!(s.active(), "A");
        s.apply_to_inactive().unwrap();
        s.commit_swap().unwrap();
        assert_eq!(s.active(), "B");
        s.rollback().unwrap();
        assert_eq!(s.active(), "A");
    }
}
