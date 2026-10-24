//! Límites de memoria/CPU por servicio (aplica el presupuesto §4).

#[derive(Clone, Debug)]
pub struct ResourceLimit {
    pub max_rss_mb: u32,
    pub max_cpu_percent: u8,
    pub nice: i8,
}

impl ResourceLimit {
    pub fn from_budget_mb(mb: u32) -> Self {
        Self {
            max_rss_mb: mb,
            max_cpu_percent: if mb > 40 { 25 } else { 10 },
            nice: 5,
        }
    }
    pub fn allows_rss(&self, current_mb: u32) -> bool {
        current_mb <= self.max_rss_mb
    }
}

/// Suma de presupuestos OnDemand no debe superar el margen de picos (~425 MB).
pub fn total_ondemand_budget(budgets: &[u32]) -> u32 {
    budgets.iter().sum()
}
