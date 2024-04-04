#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 
                 0, 1, 0, -eye_pos[1], 
                 0, 0, 1, -eye_pos[2], 
                 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    // arg rotation_angle is like 20, 60 and 90, need to convert it as xxPI
    float Pi = 3.1415926;
    model << cos(rotation_angle/180*Pi), -sin(rotation_angle/180*Pi), 0, 0, // Note that model is initialized as 4*4 matrix
             sin(rotation_angle/180*Pi),  cos(rotation_angle/180*Pi), 0, 0,
                                      0,                           0, 1, 0,
                                      0,                           0, 0, 1;
                           

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    // Note that zFar < zNear < 0
    // eye_fov: Vertical field pf view --- tan(eye_fov/2) = y_length/-zNear
    // aspect_ratio = width(x_length) / height(y_length)

    // Here the projection is perspective projection
    // M_perspective = M_orthogonal * M_persp->ortho
    float y_l = 2 * -zNear * tan(eye_fov/2);
    float x_l = y_l * aspect_ratio;
    Eigen::Matrix<float, 4, 4> scale; // second scale into a 2*2*2 cubic
    scale << 2/x_l,     0,                0, 0,
                 0, 2/y_l,                0, 0,
                 0,     0, 2/(zNear - zFar), 0,
                 0,     0,                0, 1;
    Eigen::Matrix<float, 4, 4> translate; // first translate to the origin
    translate << 1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, -(zFar + zNear)/2,
                 0, 0, 0, 1;
    // Above matrix should be:
    //  1, 0, 0, -(x_right + x_left)/2,
    //  0, 1, 0, -(y_right + y_left)/2,
    //  0, 0, 1, -(zFar + zNear)/2,
    //  0, 0, 0, 1；
    // Here we see x_right == x__left and y_right == y_left
    Eigen::Matrix<float, 4, 4> M_o = scale*translate;
    Eigen::Matrix<float, 4, 4> M_p2o;
    M_p2o << zNear,     0,          0,           0,
                 0, zNear,          0,           0,
                 0,     0, zNear+zFar, -zNear*zFar,
                 0,     0,          1,           0;
    projection = M_o * M_p2o;

    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
