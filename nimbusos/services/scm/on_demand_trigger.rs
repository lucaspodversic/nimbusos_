//! Triggers de activación bajo demanda: socket, ruta, evento, RPC.

use std::collections::HashMap;

#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub enum TriggerKind {
    /// Escucha TCP/UDP: al llegar un paquete se arranca el servicio.
    Socket { port: u16 },
    /// Primera apertura de un path (p.ej. \\.\Pipe\Spooler).
    Path { path: String },
    /// Evento ETW-lite / named event.
    Event { name: String },
    /// Primera llamada RPC/ALPC al puerto del servicio.
    Rpc { port_name: String },
}

#[derive(Default, Debug)]
pub struct TriggerTable {
    /// trigger → nombre de servicio
    map: HashMap<TriggerKind, String>,
}

impl TriggerTable {
    pub fn new() -> Self { Self::default() }
    pub fn bind(&mut self, trigger: TriggerKind, service: &str) {
        self.map.insert(trigger, service.to_string());
    }
    pub fn resolve(&self, trigger: &TriggerKind) -> Option<&str> {
        self.map.get(trigger).map(|s| s.as_str())
    }
    pub fn seed_defaults(&mut self) {
        self.bind(TriggerKind::Socket { port: 445 }, "installer"); // ejemplo
        self.bind(TriggerKind::Path { path: r"\\.\Pipe\spoolss".into() }, "spooler");
        self.bind(TriggerKind::Event { name: "BluetoothDeviceArrival".into() }, "bluetooth");
        self.bind(TriggerKind::Rpc { port_name: "AudioEndpoint".into() }, "audio");
        self.bind(TriggerKind::Path { path: r"\\.\Pipe\UpdateSvc".into() }, "update");
        self.bind(TriggerKind::Event { name: "FileWriteScan".into() }, "defender");
    }
}
