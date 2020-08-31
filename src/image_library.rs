use once_cell_regex::regex;
use regex::Regex;

use std::collections::HashMap;
use std::fs;
use std::path::{Path, PathBuf};
use rand::seq::SliceRandom;
use std::result::Result;
use std::vec::Vec;

#[derive(Clone, PartialEq, Eq, Hash)]
pub struct Resolution {
    width: u64,
    height: u64
}

impl Resolution {
    pub fn new(path: &str) -> Result<Self, String> {
        // Parse file path which must has the following format: widthxheight
        // Width and height must be a positive integers
        let re : &Regex = regex!(r#"(?P<width>\d+)x(?P<height>\d+)"#);
        let cap = re.captures(path).ok_or("Cannot parse string with resolution")?;

        Ok(Resolution {
            width: cap["width"].parse::<u64>().map_err(|e| e.to_string())?,
            height: cap["height"].parse::<u64>().map_err(|e| e.to_string())?
        })
    }
}

pub struct ImageLibrary {
    storage: HashMap<Resolution, Vec<PathBuf>>
}

impl ImageLibrary {
    pub fn new(directory: &Path) -> Result<Self, String> {
        if directory.is_dir() {
            let mut library = ImageLibrary::default();

            for entry in fs::read_dir(directory).map_err(|e| e.to_string())? {
                let entry = entry.map_err(|e| e.to_string())?;
                let path = entry.path();
                if path.is_dir() {
                    let resolution =
                        Resolution::new(path.to_str().ok_or("Resolution cannot be created")?)?;
                    library.add_resolution(resolution.clone());

                    library.load_images(resolution, path.as_path())?;
                }
            }

            Ok(library)
        }
        else {
            Err("Provided path is not a directory path".to_string())
        }
    }

    pub fn add_resolution(&mut self, resolution: Resolution) {
        if !self.storage.contains_key(&resolution) {
            self.storage.insert(resolution, Vec::<PathBuf>::new());
        }
    }

    pub fn add_image(&mut self, resolution: &Resolution, image_path: PathBuf) {
        self.storage.get_mut(resolution).unwrap().push(image_path);
    }

    pub fn default() -> Self {
        ImageLibrary {storage: HashMap::new()}
    }

    pub fn load_images(&mut self, resolution: Resolution, path: &Path) -> Result<(), String> {
        if path.is_dir() {
            for entry in fs::read_dir(path).map_err(|e| e.to_string())? {
                let entry = entry.map_err(|e| e.to_string())?;
                let path = entry.path();
                if path.is_file() {
                    self.add_image(&resolution, path);
                }
            }

            Ok(())
        }
        else {
            Err("Provided path is not a directory path".to_string())
        }
    }

    pub fn get_random_image(&self) -> Result<&PathBuf, String> {
        let keys: Vec<&Resolution> = self.storage.keys().collect();
        let chosen_resolution =
            keys.choose(&mut rand::thread_rng())
                .ok_or("Cannot choose resolution randomly")?;
        let image_paths =
            self.storage.get(chosen_resolution)
                        .ok_or("Cannot extract images for resolution")?;
        image_paths.choose(&mut rand::thread_rng()).ok_or("Cannot choose image randomly".to_string())
    }
}
