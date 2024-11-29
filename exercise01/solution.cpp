#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

void project_points(
    const std::string& full_path_input_mesh,
    double optical_center_x,
    double optical_center_y, 
    double optical_center_z,
    double optical_axis_x,
    double optical_axis_y,
    double optical_axis_z,
    int output_width_in_pixels,
    int output_height_in_pixels,
    const std::string& full_path_output,
    double focal_distance = 1.0) {

    // 1. Leer archivo OFF
    std::ifstream file(full_path_input_mesh);
    std::string header;
    int num_vertices, num_faces, num_edges;
    file >> header >> num_vertices >> num_faces >> num_edges;

    std::vector<cv::Point3d> vertices;
    for(int i = 0; i < num_vertices; i++) {
        double x, y, z;
        file >> x >> y >> z;
        vertices.push_back(cv::Point3d(x, y, z));
    }

    // 2. Configurar sistema de coordenadas de la cámara
    cv::Point3d camera_center(optical_center_x, optical_center_y, optical_center_z);
    cv::Point3d optical_axis(optical_axis_x, optical_axis_y, optical_axis_z);
    
    // Normalizar el eje óptico
    double norm = sqrt(optical_axis.x*optical_axis.x + 
                      optical_axis.y*optical_axis.y + 
                      optical_axis.z*optical_axis.z);
    optical_axis = optical_axis * (1.0/norm);

    // 3. Crear imagen de salida
    cv::Mat image = cv::Mat::zeros(output_height_in_pixels, output_width_in_pixels, CV_8UC3);

    // 4. Proyectar cada vértice
    for(const auto& vertex : vertices) {
        // Vector desde el centro óptico al vértice
        cv::Point3d ray = vertex - camera_center;
        
        // Proyección del punto
        double dot_product = ray.x*optical_axis.x + 
                           ray.y*optical_axis.y + 
                           ray.z*optical_axis.z;
        
        // Solo proyectar puntos frente a la cámara
        if(dot_product > 0) {
            // Calcular coordenadas en el plano de imagen
            double scale = focal_distance / dot_product;
            int pixel_x = output_width_in_pixels/2 + 
                         static_cast<int>(scale * (ray.x - dot_product*optical_axis.x) * output_width_in_pixels);
            int pixel_y = output_height_in_pixels/2 - 
                         static_cast<int>(scale * (ray.y - dot_product*optical_axis.y) * output_height_in_pixels);

            // Dibujar punto si está dentro de la imagen
            if(pixel_x >= 0 && pixel_x < output_width_in_pixels && 
               pixel_y >= 0 && pixel_y < output_height_in_pixels) {
                cv::circle(image, cv::Point(pixel_x, pixel_y), 1, cv::Scalar(255,255,255), -1);
            }
        }
    }

    // 5. Guardar imagen
    cv::imwrite(full_path_output, image);
}


int main() {
    project_points("bunny.off", -60, -60, -195, 1, 1.3, 5, 512, 512, "bunny.png");

    return 0;
}

