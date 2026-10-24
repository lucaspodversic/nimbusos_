//! Lógica principal del SCM: arranque, stop, triggers.

use crate::on_demand_trigger::{TriggerKind, TriggerTable};
use crate::resource_limits::ResourceLimit;
use crate::service_db::{ServiceDb, ServiceState, StartType};

#[derive(Debug)]
pub struct Scm {
    pub db: ServiceDb,
    pub triggers: TriggerTable,
    next_pid: u32,
}

impl Scm {
    pub fn new() -> Self {
        let mut db = ServiceDb::new();
        db.seed_minimal();
        let mut triggers = TriggerTable::new();
        triggers.seed_defaults();
        Self { db, triggers, next_pid: 1000 }
    }

    /// Arranca todos los Boot/System/Auto al inicio de sesión.
    pub fn boot_autostart(&mut self) -> Vec<String> {
        let mut started = Vec::new();
        let names: Vec<String> = self.db.services.iter()
            .filter(|(_, s)| matches!(s.start, StartType::Boot | StartType::System | StartType::Auto))
            .map(|(n, _)| n.clone())
            .collect();
        for n in names {
            if self.start(&n).is_ok() {
                started.push(n);
            }
        }
        started
    }

    pub fn start(&mut self, name: &str) -> Result<u32, String> {
        let rec = self.db.get_mut(name).ok_or_else(|| format!("servicio desconocido: {name}"))?;
        if rec.start == StartType::Disabled {
            return Err("disabled".into());
        }
        if rec.state == ServiceState::Running {
            return Ok(rec.pid.unwrap_or(0));
        }
        rec.state = ServiceState::StartPending;
        let pid = self.next_pid;
        self.next_pid += 1;
        // En host simulamos el spawn; en bare-metal sería NtCreateUserProcess.
        let rec = self.db.get_mut(name).unwrap();
        rec.pid = Some(pid);
        rec.state = ServiceState::Running;
        Ok(pid)
    }

    pub fn stop(&mut self, name: &str) -> Result<(), String> {
        let rec = self.db.get_mut(name).ok_or_else(|| "no existe".to_string())?;
        rec.state = ServiceState::Stopped;
        rec.pid = None;
        Ok(())
    }

    /// Activa un servicio si un trigger coincide.
    pub fn on_trigger(&mut self, trigger: &TriggerKind) -> Result<Option<u32>, String> {
        let Some(svc) = self.triggers.resolve(trigger).map(|s| s.to_string()) else {
            return Ok(None);
        };
        let pid = self.start(&svc)?;
        Ok(Some(pid))
    }

    pub fn running_count(&self) -> usize {
        self.db.services.values().filter(|s| s.state == ServiceState::Running).count()
    }

    pub fn total_budget_running_mb(&self) -> u32 {
        self.db.services.values()
            .filter(|s| s.state == ServiceState::Running)
            .map(|s| s.memory_budget_mb)
            .sum()
    }

    pub fn limit_for(&self, name: &str) -> Option<ResourceLimit> {
        self.db.get(name).map(|s| ResourceLimit::from_budget_mb(s.memory_budget_mb))
    }
}

impl Default for Scm {
    fn default() -> Self { Self::new() }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn autostart_keeps_ondemand_stopped() {
        let mut scm = Scm::new();
        let started = scm.boot_autostart();
        assert!(started.contains(&"win32k_server".into()));
        assert!(!started.contains(&"spooler".into()));
        assert_eq!(scm.db.get("spooler").unwrap().state, ServiceState::Stopped);
    }

    #[test]
    fn trigger_starts_spooler() {
        let mut scm = Scm::new();
        let t = TriggerKind::Path { path: r"\\.\Pipe\spoolss".into() };
        let pid = scm.on_trigger(&t).unwrap().unwrap();
        assert!(pid >= 1000);
        assert_eq!(scm.db.get("spooler").unwrap().state, ServiceState::Running);
    }

    #[test]
    fn budget_sum_under_ceiling() {
        let mut scm = Scm::new();
        scm.boot_autostart();
        let mb = scm.total_budget_running_mb();
        // Auto+System+Boot del seed caben holgados en 575 MB
        assert!(mb < 400, "budget running={mb}");
    }
}
