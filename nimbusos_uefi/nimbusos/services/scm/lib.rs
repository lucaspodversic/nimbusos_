//! Service Control Manager de NimbusOS.
//! Activa servicios bajo demanda (socket/evento/ruta) para mantener RAM ≤ 1 GB.

#![allow(dead_code)]

#[path = "scm_main.rs"]
mod scm_main;
#[path = "on_demand_trigger.rs"]
mod on_demand_trigger;
#[path = "service_db.rs"]
mod service_db;
#[path = "resource_limits.rs"]
mod resource_limits;

pub use scm_main::*;
pub use on_demand_trigger::*;
pub use service_db::*;
pub use resource_limits::*;
