// Hive Viriatum Web Server
// Copyright (c) 2008-2026 Hive Solutions Lda.
//
// The reference of the proxying workload that ships as a framework
// instead of as a server, modelled on the load balancer example of
// the crate and pointed at the upstream the harness holds constant
// across every server it puts in front of it.

use std::env;
use std::sync::Arc;

use async_trait::async_trait;
use pingora::prelude::*;

pub struct Balancer(Arc<LoadBalancer<RoundRobin>>);

#[async_trait]
impl ProxyHttp for Balancer {
    type CTX = ();

    fn new_ctx(&self) -> Self::CTX {}

    async fn upstream_peer(
        &self,
        _session: &mut Session,
        _ctx: &mut Self::CTX,
    ) -> Result<Box<HttpPeer>> {
        // the single upstream of the run is the one every server
        // under test is put in front of, so that what is measured
        // is the proxying and never the thing behind it
        let upstream = self
            .0
            .select(b"", 256)
            .ok_or_else(|| Error::new_str("no upstream is available"))?;
        Ok(Box::new(HttpPeer::new(upstream, false, String::new())))
    }
}

fn main() {
    // the addresses are taken off the command line rather than being
    // written into the binary, the ports of a run are not fixed
    let arguments: Vec<String> = env::args().collect();
    let listen = arguments
        .get(1)
        .cloned()
        .unwrap_or_else(|| "127.0.0.1:9411".to_string());
    let upstream = arguments
        .get(2)
        .cloned()
        .unwrap_or_else(|| "127.0.0.1:9412".to_string());

    let mut server = Server::new(None).expect("the server could not be created");
    server.bootstrap();

    let balancer = LoadBalancer::try_from_iter([upstream]).expect("the upstream is not valid");

    let mut proxy = http_proxy_service(&server.configuration, Balancer(Arc::new(balancer)));
    proxy.add_tcp(&listen);

    server.add_service(proxy);
    server.run_forever();
}
