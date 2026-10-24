//! Rollback A/B.
use crate::apply_ab::AbState;
impl AbState {
    pub fn rollback(&mut self) -> Result<(), String> {
        self.active = if self.active == 'A' { 'B' } else { 'A' };
        Ok(())
    }
}
