#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <memory>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>


int open_camera_fd(std::string device="/dev/video0") {
    int fd = open(device.c_str(), O_RDWR);
    if(fd < 0){
        perror("Failed to open device, OPEN");
        return -1;
    }
    return fd;
}

int check_device_capability(int fd) {
    v4l2_capability capability;
    if(ioctl(fd, VIDIOC_QUERYCAP, &capability) < 0){
        perror("Failed to get device capabilities, VIDIOC_QUERYCAP");
        return -1;
    }
    return 0;
}

int set_image_format(int fd) {
    v4l2_format imageFormat{};
    imageFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    imageFormat.fmt.pix.width = 1280;
    imageFormat.fmt.pix.height = 720;
    imageFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    imageFormat.fmt.pix.field = V4L2_FIELD_NONE;
    if(ioctl(fd, VIDIOC_S_FMT, &imageFormat) < 0){
        perror("Device could not set format, VIDIOC_S_FMT");
        return -1;
    }
    return 0;
}

int set_control(int fd, int control_id, int value) {
    v4l2_control control;
    control.id = control_id;
    control.value = value;

    if (ioctl(fd, VIDIOC_S_CTRL, &control) < 0) {
        perror("Failed to set control");
        return -1;
    }

    std::cout << "Control " << control_id << " set to " << value << std::endl;
    return 0;
}

int request_device_buffer(int fd) {
    // 4. Request Buffers from the device
    v4l2_requestbuffers requestBuffer = {};
    requestBuffer.count = 1;
    requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    requestBuffer.memory = V4L2_MEMORY_MMAP;

    if(ioctl(fd, VIDIOC_REQBUFS, &requestBuffer) < 0){
        perror("Could not request buffer from device, VIDIOC_REQBUFS");
        return -1;
    }
    return 0;
}

int request_query_buffer(int fd, std::shared_ptr<v4l2_buffer> queryBuffer) {
    queryBuffer->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    queryBuffer->memory = V4L2_MEMORY_MMAP;
    queryBuffer->index = 0;
    if(ioctl(fd, VIDIOC_QUERYBUF, queryBuffer.get()) < 0){
        perror("Device did not return the buffer information, VIDIOC_QUERYBUF");
        return -1;
    }
    return 0;
}

struct FrameBuffer {
    std::shared_ptr<uint8_t> buffer;
    size_t size;
};

int init_frame_buffer(int fd, std::shared_ptr<FrameBuffer> frameBuffer) {
    std::shared_ptr<v4l2_buffer> queryBuffer = std::make_shared<v4l2_buffer>();
    if (request_query_buffer(fd, queryBuffer) < 0) {
        return -1;
    }

    uint8_t* buffer = static_cast<uint8_t*>(mmap(NULL, queryBuffer->length, PROT_READ | PROT_WRITE, MAP_SHARED,
                        fd, queryBuffer->m.offset));
    if (buffer == MAP_FAILED) {
        perror("init_frame_buffer: mmap failed");
        return -1;
    }
    memset(buffer, 0, queryBuffer->length);

    frameBuffer->buffer = std::shared_ptr<uint8_t>(
        static_cast<uint8_t*>(buffer),
        [length = queryBuffer->length](uint8_t* p) {
            munmap(p, length);
        }
    );
    frameBuffer->size = queryBuffer->length;

    return 0;
}

int start_stream(int fd, std::shared_ptr<v4l2_buffer> bufferInfo) {
    bufferInfo->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferInfo->memory = V4L2_MEMORY_MMAP;
    bufferInfo->index = 0;

    // Activate streaming
    if(ioctl(fd, VIDIOC_STREAMON, &bufferInfo->type) < 0){
        perror("Could not start streaming, VIDIOC_STREAMON");
        return -1;
    }
    // Give the device time to get ready
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    return 0;
}

int queue_single_frame(int fd, std::shared_ptr<v4l2_buffer> bufferInfo, std::shared_ptr<FrameBuffer> frameBuffer, std::shared_ptr<std::vector<uint8_t>> frame) {
    // Queue the buffer
    if(ioctl(fd, VIDIOC_QBUF, bufferInfo.get()) < 0){
        perror("Could not queue buffer, VIDIOC_QBUF");
        return -1;
    }
    // Dequeue the buffer
    if(ioctl(fd, VIDIOC_DQBUF, bufferInfo.get()) < 0){
        perror("Could not dequeue the buffer, VIDIOC_DQBUF");
        return -1;
    }
    // Frames get written after dequeuing the buffer
    frame->assign(frameBuffer->buffer.get(), frameBuffer->buffer.get() + bufferInfo->bytesused);
    return 0;
}


int get_frame(int fd, int *buffertype, std::shared_ptr<std::vector<uint8_t>> frame) {
    std::shared_ptr<FrameBuffer> frameBuffer = std::make_shared<FrameBuffer>();
    frameBuffer->buffer = std::make_shared<uint8_t>();
    std::shared_ptr<v4l2_buffer> bufferInfo = std::make_shared<v4l2_buffer>();

    init_frame_buffer(fd, frameBuffer);
    start_stream(fd, bufferInfo);
    queue_single_frame(fd, bufferInfo, frameBuffer, frame);
    *buffertype = bufferInfo->type;

    std::cout << "Buffer has: " << (double)bufferInfo->bytesused / 1024 << " KB of data" << std::endl;
    return 0;
}

int get_frame_and_skip(int fd, int *buffertype, std::shared_ptr<std::vector<uint8_t>> frame, int nof_skips) {
    std::shared_ptr<FrameBuffer> frameBuffer = std::make_shared<FrameBuffer>();
    frameBuffer->buffer = std::make_shared<uint8_t>();
    std::shared_ptr<v4l2_buffer> bufferInfo = std::make_shared<v4l2_buffer>();

    init_frame_buffer(fd, frameBuffer);
    start_stream(fd, bufferInfo);
    for (int i = 0; i < nof_skips; ++i) {
        queue_single_frame(fd, bufferInfo, frameBuffer, frame);
    }
    *buffertype = bufferInfo->type;

    std::cout << "Buffer has: " << (double)bufferInfo->bytesused / 1024 << " KB of data" << std::endl;
    return 0;
}

int end_stream(int fd, int type) {
    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
        perror("Could not end streaming, VIDIOC_STREAMOFF");
        return -1;
    }
    return 0;
}

