mod image_library;
mod parameters;

async fn get_random_picture(
    data: actix_web::web::Data<image_library::ImageLibrary>,
) -> std::io::Result<actix_files::NamedFile> {
    let path = data.get_random_image().unwrap();
    Ok(actix_files::NamedFile::open(path).unwrap())
}

#[actix_rt::main]
async fn main() -> std::io::Result<()> {
    let parameters = parameters::Parameters::new();

    // Load image library
    let image_library = actix_web::web::Data::new(
        image_library::ImageLibrary::new(parameters.image_library_path.as_path()).unwrap(),
    );

    actix_web::HttpServer::new(move || {
        actix_web::App::new().app_data(image_library.clone()).route(
            "/api/v1/wallpaper/random",
            actix_web::web::get().to(get_random_picture),
        )
    })
    .bind(format!(
        "{}:{}",
        parameters.bind_address, parameters.bind_port
    ))?
    .run()
    .await
}
