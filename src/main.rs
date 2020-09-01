mod image_library;

use actix_web::{web, App, HttpServer};
use actix_files::NamedFile;
use clap::Clap;
use std::path::PathBuf;
use std::io;

/// This doc string acts as a help message when the user runs '--help'
/// as do all doc strings on fields
#[derive(Clap)]
#[clap(version = "1.0", author = "Alexander Zaitsev <zamazan4ik@tut.by>")]
struct Opts {
    /// Image library path
    #[clap(short, long)]
    library_path: PathBuf,
    #[clap(short, long, default_value = "0.0.0.0")]
    address: String,
    #[clap(short, long, default_value = "8080")]
    port: u16
}

async fn get_random_picture(data: web::Data<image_library::ImageLibrary>) -> io::Result<NamedFile> {
    let path = data.get_random_image().unwrap();
    Ok(NamedFile::open(path).unwrap())
}

#[actix_rt::main]
async fn main() -> std::io::Result<()> {
    let opts: Opts = Opts::parse();

    // Load image library
    let image_library =
        web::Data::new(image_library::ImageLibrary::new(opts.library_path.as_path())
            .unwrap());

    HttpServer::new(move || {
        App::new()
            .app_data(image_library.clone())
            .route("/api/v1/wallpaper/random", web::get()
                .to(get_random_picture))
    })
        .bind(format!("{}:{}", opts.address, opts.port))?
        .run()
        .await
}
