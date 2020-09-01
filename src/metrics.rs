use prometheus;

lazy_static! {
    pub static ref REQUEST_COUNTER: prometheus::IntCounter =
        prometheus::register_int_counter!("requests", "Number of requests").unwrap();
}

pub fn init_metrics() {
    REQUEST_COUNTER.reset();
}
