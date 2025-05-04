#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libexif/exif-content.h>
#include <string>
#include <memory>
#include <vector>
#include <array>
#include <ctime>
#include <cstdint>
#include <cstdio>
#include <cstdint>
#include <jpeglib.h>
#include <libexif/exif-data.h>
#include <libexif/exif-utils.h>


#define RGB_OFFSET 1.0f

inline uint8_t clamp(int value, int low, int high) {
    return static_cast<uint8_t>(value < low ? low : (value > high ? high : value));
}

void intYUVtoRGB(uint8_t y, uint8_t u, uint8_t v, uint8_t& r, uint8_t& g, uint8_t& b) {
    int c = y - 16;
    int d = u - 128;
    int e = v - 128;

    int rt = (298 * c + 409 * e + 128) >> 8;
    int gt = (298 * c - 100 * d - 208 * e + 128) >> 8;
    int bt = (298 * c + 516 * d + 128) >> 8;

    r = clamp(rt, 0, 255);
    g = clamp(gt, 0, 255);
    b = clamp(bt, 0, 255);
}

void convertYUYtoJPEG(const std::vector<uint8_t> &input, const int width, const int height, std::vector<uint8_t>& output) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    uint8_t* outbuffer = NULL;
    uint64_t outlen = 0;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, &outbuffer, &outlen);

    // jrow is a libjpeg row of samples array of 1 row pointer
    cinfo.image_width = width & -1;
    cinfo.image_height = height & -1;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB; //libJPEG expects YUV 3bytes, 24bit

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 92, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    std::vector<uint8_t> tmprowbuf(width * 3);

    JSAMPROW row_pointer[1];
    row_pointer[0] = &tmprowbuf[0];
    while (cinfo.next_scanline < cinfo.image_height) {
        unsigned offset = cinfo.next_scanline * width * 2;

        for (int i = 0, j = 0; i < width * 2; i += 4, j += 6) {
            uint8_t y0 = input[offset + i + 0];
            uint8_t u  = input[offset + i + 1];
            uint8_t y1 = input[offset + i + 2];
            uint8_t v  = input[offset + i + 3];
            // Pixel 0
            {
    	    intYUVtoRGB(y0, u, v, tmprowbuf[j + 0], tmprowbuf[j + 1], tmprowbuf[j + 2]);
            }
            // Pixel 1
            {
    	    intYUVtoRGB(y1, u, v, tmprowbuf[j + 3], tmprowbuf[j + 4], tmprowbuf[j + 5]);
            }
        }
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }


    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    output = std::vector<uint8_t>(outbuffer, outbuffer + outlen);
    free(outbuffer);
}

// Helper to create or return existing tag
std::unique_ptr<ExifEntry> create_tag(std::shared_ptr<ExifData> ed, ExifIfd ifd, ExifTag tag, unsigned int len) {
    ExifEntry *entry;

    // Check if the tag already exists
    entry = exif_content_get_entry(ed->ifd[ifd], tag);
    if (entry) {
        return std::unique_ptr<ExifEntry>(entry);  // Return wrapped in shared_ptr
    }

    // Allocate and initialize new entry
    entry = exif_entry_new();
    entry->tag = tag;
    entry->data = (unsigned char *)calloc(1, len);
    entry->size = len;

    exif_content_add_entry(ed->ifd[ifd], entry);
    exif_entry_unref(entry); // owned by content now

    return std::unique_ptr<ExifEntry>(entry);  // Return wrapped in shared_ptr
}

void set_exif_string(std::shared_ptr<ExifData> ed, const ExifTag &tag, const std::string& value) {
    // value_size includes null terminator
    const size_t value_size = value.size();
    std::unique_ptr<ExifEntry> entry = create_tag(ed, EXIF_IFD_EXIF, tag, value.size() + 1);

    // components excludes null terminator (value_size)
    entry->format = EXIF_FORMAT_ASCII;
    entry->components = value_size;

    // Copy date string, ensure null termination
    strncpy((char*)entry->data, value.c_str(), value_size + 1);
    entry->data[value_size] = '\0';
}

// Set EXIF DateTimeOriginal (format: "YYYY:MM:DD HH:MM:SS")
int set_exif_datetime(std::shared_ptr<ExifData> ed, const std::string& dateTimeStr) {
    // Must be exactly 19 bytes for EXIF datetime
    const int FORMAT_DATETIME_SIZE = 19;
    
    if (dateTimeStr.size() != FORMAT_DATETIME_SIZE) {
        perror("set_exif_datetime_original parameter error. dateTimeStr must be exact 19 characters!");
        return -1;
    }
    set_exif_string(ed, EXIF_TAG_DATE_TIME_ORIGINAL, dateTimeStr);

    return 0;
}

std::string get_datetime() {
    time_t now = time(nullptr);
    tm local_tm;
    localtime_r(&now, &local_tm); // safer than std::localtime

    char buffer[20]; // "YYYY:MM:DD HH:MM:SS" + '\0'
    std::strftime(buffer, sizeof(buffer), "%Y:%m:%d %H:%M:%S", &local_tm);
    return std::string(buffer);
}

void fill_mandatory_fields(std::shared_ptr<ExifData> ed, int width, int height) {
    std::unique_ptr<ExifEntry> entry;
    /* Create the mandatory EXIF fields with default data */
    exif_data_fix(ed.get());
    /* All these tags are created with default values by exif_data_fix() */
    /* Change the data to the correct values for this image. */
    entry = std::unique_ptr<ExifEntry>(exif_content_get_entry(ed->ifd[EXIF_IFD_EXIF], EXIF_TAG_PIXEL_X_DIMENSION));
    exif_set_long(entry->data, EXIF_BYTE_ORDER_INTEL, width);
    entry = std::unique_ptr<ExifEntry>(exif_content_get_entry(ed->ifd[EXIF_IFD_EXIF], EXIF_TAG_PIXEL_Y_DIMENSION));
    exif_set_long(entry->data, EXIF_BYTE_ORDER_INTEL, height);
    entry = std::unique_ptr<ExifEntry>(exif_content_get_entry(ed->ifd[EXIF_IFD_EXIF], EXIF_TAG_COLOR_SPACE));
    exif_set_short(entry->data, EXIF_BYTE_ORDER_INTEL, 1);
}

std::vector<uint8_t> create_EXIF_metadata(const std::string& cameraName, const std::string& datetime, int width, int height) {
    std::shared_ptr<ExifData> ed(exif_data_new());
    exif_data_set_option(ed.get(), EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);
    exif_data_set_data_type(ed.get(), EXIF_DATA_TYPE_COMPRESSED);
    exif_data_set_byte_order(ed.get(), EXIF_BYTE_ORDER_INTEL);

    exif_data_fix(ed.get());

    fill_mandatory_fields(ed, width, height);

    // Set camera make/model
    // exif_data_fix(ed);
    // exif_entry_initialize(exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_MAKE), EXIF_TAG_MAKE);
    // exif_entry_initialize(exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_MODEL), EXIF_TAG_MODEL);

    set_exif_string(ed, EXIF_TAG_MODEL, cameraName);

    // Set current time
    set_exif_datetime(ed, datetime);

    // Save EXIF block to memory
    unsigned char *exif_data = nullptr;
    unsigned int exif_data_len = 0;
    exif_data_save_data(ed.get(), &exif_data, &exif_data_len);

    std::vector<uint8_t> result(exif_data, exif_data + exif_data_len);
    free(exif_data);
    exif_data_unref(ed.get());
    return result;
}

/* insert the EXIF after the SOI marker - ignore existing EXIF APP1 markers */
int insertEXIF(std::vector<uint8_t>& image, const std::vector<uint8_t> &exif) {
    static const std::array<uint8_t, 2> MARKER_SOI{{0xFF, 0xD8}}; // SOI MARKER := Start of Image
    if (image.size() < 2) {
        return -1;
    }
    if (image[0] == MARKER_SOI[0] && image[1] == MARKER_SOI[1]) {
        // Insert EXIF right after SOI
        image.insert(image.begin() + 2, exif.begin(), exif.end());
        return 0;
    }

    return -1;
}
