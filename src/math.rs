use serde::Serialize;

#[derive(Clone, Debug, Default, Serialize)]
pub struct Vec3 {
    pub x: f32,
    pub y: f32,
    pub z: f32,
}
