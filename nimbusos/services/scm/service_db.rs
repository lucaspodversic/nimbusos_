//! Base de datos de servicios registrados (equivalente ligero a la SCM DB).

use std::collections::HashMap;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum StartType {
    Boot,
    System,
    Auto,
    /// Solo se arranca cuando un trigger lo pide (clave para el presupuesto RAM).
    OnDemand,
    Disabled,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ServiceState {
    Stopped,
    StartPending,
    Running,
    StopPending,
}

#[derive(Clone, Debug)]
pub struct ServiceRecord {
    pub name: String,
    pub display: String,
    pub binary: String,
    pub start: StartType,
    pub state: ServiceState,
    pub pid: Option<u32>,
    pub memory_budget_mb: u32,
    pub depends_on: Vec<String>,
}

#[derive(Default, Debug)]
pub struct ServiceDb {
    pub services: HashMap<String, ServiceRecord>,
}

impl ServiceDb {
    pub fn new() -> Self { Self::default() }

    pub fn register(&mut self, rec: ServiceRecord) {
        self.services.insert(rec.name.clone(), rec);
    }

    pub fn get(&self, name: &str) -> Option<&ServiceRecord> {
        self.services.get(name)
    }

    pub fn get_mut(&mut self, name: &str) -> Option<&mut ServiceRecord> {
        self.services.get_mut(name)
    }

    pub fn list_by_start(&self, st: StartType) -> Vec<&ServiceRecord> {
        self.services.values().filter(|s| s.start == st).collect()
    }

    /// Semilla mínima de servicios del sistema (imagen base sin bloat).
    pub fn seed_minimal(&mut self) {
        let specs = [
            ("smss", "Session Manager", "smss.exe", StartType::Boot, 8),
            ("csrss", "Client Server Runtime", "csrss_lite.exe", StartType::System, 12),
            ("wininit", "Windows-like Init", "wininit.exe", StartType::System, 6),
            ("winlogon", "Logon UI", "winlogon.exe", StartType::Auto, 16),
            ("lsass", "Auth", "lsass_lite.exe", StartType::Auto, 20),
            ("win32k_server", "Window Server", "win32k_server", StartType::Auto, 40),
            ("compositor", "Compositor", "compositor", StartType::Auto, 80),
            ("audio", "Audio Engine", "audio_engine", StartType::OnDemand, 20),
            ("bluetooth", "Bluetooth", "bluetooth_service", StartType::OnDemand, 12),
            ("spooler", "Print Spooler", "print_spooler", StartType::OnDemand, 10),
            ("search", "Search Indexer", "search_indexer", StartType::OnDemand, 24),
            ("defender", "Defender-lite", "defender_lite", StartType::OnDemand, 32),
            ("update", "Update Service", "update_service", StartType::OnDemand, 16),
            ("installer", "Installer", "installer_service", StartType::OnDemand, 28),
            ("memgov", "Memory Governor", "memory_governor", StartType::Auto, 8),
        ];
        for (name, disp, bin, start, mb) in specs {
            self.register(ServiceRecord {
                name: name.into(),
                display: disp.into(),
                binary: bin.into(),
                start,
                state: ServiceState::Stopped,
                pid: None,
                memory_budget_mb: mb,
                depends_on: vec![],
            });
        }
    }
}
