//! Presupuestos duros por componente (§4 del diseño).

#[derive(Clone, Debug)]
pub struct BudgetPolicy {
    pub kernel_mb: u32,
    pub cache_mb: u32,
    pub registry_mb: u32,
    pub services_mb: u32,
    pub window_server_mb: u32,
    pub shell_mb: u32,
    pub audio_net_mb: u32,
    pub security_mb: u32,
    pub gpu_buffers_mb: u32,
}

impl Default for BudgetPolicy {
    fn default() -> Self {
        Self {
            kernel_mb: 90, cache_mb: 60, registry_mb: 20, services_mb: 60,
            window_server_mb: 120, shell_mb: 70, audio_net_mb: 35,
            security_mb: 40, gpu_buffers_mb: 80,
        }
    }
}

impl BudgetPolicy {
    pub fn total_idle_mb(&self) -> u32 {
        self.kernel_mb + self.cache_mb + self.registry_mb + self.services_mb
        + self.window_server_mb + self.shell_mb + self.audio_net_mb
        + self.security_mb + self.gpu_buffers_mb
    }
    pub fn under_ceiling(&self) -> bool { self.total_idle_mb() <= 1024 }
}
