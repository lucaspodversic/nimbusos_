//! Cliente DHCP.
#[derive(Default, Debug)]
pub struct DhcpLease { pub ip: [u8;4], pub gateway: [u8;4] }
impl DhcpLease {
  pub fn example() -> Self { Self { ip:[10,0,0,2], gateway:[10,0,0,1] } }
}
