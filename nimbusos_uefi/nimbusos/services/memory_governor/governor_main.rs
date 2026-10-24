//! Orquesta presupuestos, presión e hibernación.

use crate::budget_policy::BudgetPolicy;
use crate::pressure_events::{classify, PressureLevel};
use crate::process_hibernator::ProcessHibernator;

pub struct Governor {
    pub policy: BudgetPolicy,
    pub hibernator: ProcessHibernator,
    pub used_system_mb: u32,
}

impl Governor {
    pub fn new() -> Self {
        Self { policy: BudgetPolicy::default(), hibernator: ProcessHibernator::default(), used_system_mb: 0 }
    }
    pub fn pressure(&self) -> PressureLevel {
        classify(self.used_system_mb, 1024)
    }
    pub fn on_minimize(&mut self, pid: u32, name: &str, state: &[u8]) {
        self.hibernator.hibernate(pid, name, state);
        self.used_system_mb = self.used_system_mb.saturating_sub(8);
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn idle_budget_under_1gb() {
        let g = Governor::new();
        assert!(g.policy.under_ceiling());
        assert!(g.policy.total_idle_mb() <= 575 + 50); // ≈575
        assert_eq!(g.pressure(), PressureLevel::Normal);
    }
}
