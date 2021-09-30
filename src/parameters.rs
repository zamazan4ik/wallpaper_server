use std::str::FromStr;

pub struct Parameters {
    pub bind_address: String,
    pub bind_port: u16,
    pub image_library_path: std::path::PathBuf,
}

impl Parameters {
    pub fn new() -> Self {
        let bind_address =
            std::env::var("BIND_ADDRESS").expect("BIND_ADDRESS env var is not specified");

        let bind_port = std::env::var("BIND_PORT")
            .unwrap_or("8080".to_string())
            .parse::<u16>()
            .expect("Cannot parse BIND_PORT as u16");

        let image_library_path = std::path::PathBuf::from_str(
            std::env::var("IMAGE_LIBRARY_PATH")
                .expect("IMAGE_LIBRARY_PATH env var is not specified")
                .as_str(),
        )
        .expect("Cannot parse IMAGE_LIBRARY_PATH as a valid filepath");

        Self {
            bind_address,
            bind_port,
            image_library_path,
        }
    }
}
