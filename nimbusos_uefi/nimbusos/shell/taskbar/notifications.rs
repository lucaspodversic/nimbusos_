//! Centro de notificaciones liviano.

#[derive(Clone, Debug)]
pub struct Notification {
    pub id: u64,
    pub app: String,
    pub title: String,
    pub body: String,
}

#[derive(Default, Debug)]
pub struct NotificationCenter {
    next: u64,
    pub items: Vec<Notification>,
}

impl NotificationCenter {
    pub fn push(&mut self, app: &str, title: &str, body: &str) -> u64 {
        let id = self.next; self.next += 1;
        self.items.push(Notification { id, app: app.into(), title: title.into(), body: body.into() });
        id
    }
    pub fn dismiss(&mut self, id: u64) { self.items.retain(|n| n.id != id); }
    pub fn clear(&mut self) { self.items.clear(); }
}
