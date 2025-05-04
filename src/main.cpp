#include <csignal>

#include "camera.cpp"
#include "image.cpp"
#include "save.cpp"

int fd = -1;

void cleanup(int signum) {
    std::cout << "Signal received, cleaning up resources..." << std::endl;

    if (fd >= 0) {
        close(fd);
        fd = -1;
    }

    exit(signum);
}


int main() {
    // Register signal handler for SIGTERM and SIGINT (graceful shutdown)
    signal(SIGTERM, cleanup);
    signal(SIGINT, cleanup);

    static const int IMAGE_WIDTH = 1280;
    static const int IMAGE_HEIGHT = 720;

    fd = open_camera_fd();
    if (fd < 0) {
        return 1;
    }
    std::cout << "🗹  open camera file descriptor" << std::endl; 

    if (check_device_capability(fd) < 0) {
        close(fd);
        return 1;
    }
    std::cout << "🗹  check device capability" << std::endl; 

    if (set_image_format(fd) < 0) {
        close(fd);
        return 1;
    }
    std::cout << "🗹  set image format" << std::endl; 

    if (set_control(fd, V4L2_CID_BRIGHTNESS, 100) < 0) {
        close(fd);
        return -1;
    }
    std::cout << "🗹  set brightness" << std::endl; 

    if (set_control(fd, V4L2_CID_CONTRAST, 100) < 0) {
        close(fd);
        return -1;
    }
    std::cout << "🗹  set contrast" << std::endl; 

    if (set_control(fd, V4L2_CID_SATURATION, 128) < 0) {
        close(fd);
        return -1;
    }
    std::cout << "🗹  set saturation" << std::endl; 

    if (set_control(fd, V4L2_CID_HUE, 128) < 0) {
        close(fd);
        return -1;
    }
    std::cout << "🗹  set hue" << std::endl; 

    if (set_control(fd, V4L2_CID_AUTO_WHITE_BALANCE, 1) < 0) {
        close(fd);
        return -1;
    }
    std::cout << "🗹  set auto white balance" << std::endl; 


    if (request_device_buffer(fd) < 0) {
        close(fd);
        return 1;
    }
    std::cout << "🗹  request device buffer" << std::endl; 

    int type;
    std::shared_ptr<std::vector<uint8_t>> camerabuffer = std::make_shared<std::vector<uint8_t>>();

    static const int SKIP_FRAMES = 2;
    if (get_frame_and_skip(fd, &type, camerabuffer, SKIP_FRAMES) < 0) {
        end_stream(fd, type);
        close(fd);
        return 1;
    }
    std::cout << "🗹  got a frame" << std::endl; 

    if (end_stream(fd, type) < 0) {
        close(fd);
        return 1;
    }
    close(fd);
    std::cout << "🗹  end camera stream" << std::endl; 

    /* Save raw */
    save_as("webcam_output.yuv", *camerabuffer.get());
    std::cout << "🗹  stored as webcam_output.yuv" << std::endl; 

    /* Save image */
    std::vector<uint8_t> imagebuffer;
    convertYUYtoJPEG(*camerabuffer.get(), IMAGE_WIDTH, IMAGE_HEIGHT, imagebuffer);
    std::cout << "🗹  convert raw image to jpeg" << std::endl; 

    // static const std::vector<uint8_t> metadata = create_EXIF_metadata(
    //     	    "Apple Facetime HD",
    //     	    get_datetime(),
    //     	    IMAGE_WIDTH,
    //     	    IMAGE_HEIGHT
    //     	    );
    // if(insertEXIF(imagebuffer, metadata) < 0) {
    //     perror("Could not insert EXIF to picture");
    //     return 1;
    // }
    // std::cout << "🗹  attached metadata (EXIF) to jpeg" << std::endl; 

    save_as("webcam_output.jpeg", imagebuffer);
    std::cout << "🗹  stored as webcam_output.jpeg" << std::endl; 


    return 0;
}
