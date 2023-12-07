#include "device.h"
#include "device3.h"
#include "driver.h"
#include "imu.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define XREAL_TS_TO_MS_FACTOR 1000000
#define XREAL_DOWNSAMPLE_FACTOR 5 // we don't need to process updates at the full XREAL frequency

const imu_quat_type nwu_conversion_quat = {.x = 0, .y = 0, .z = 0.7071, .w = 0.7071};

const device_properties_type xreal_air_properties = {
    .hid_vendor_id                      = 0,
    .hid_product_id                     = 0,
    .resolution_w                       = 1920,
    .resolution_h                       = 1080,
    .fov                                = 46.0,
    .lens_distance_ratio                = 0.035,
    .calibration_wait_s                 = 15,
    .imu_cycles_per_s                   = 1000,
    .imu_buffer_size                    = 10,
    .look_ahead_constant                = 10.0,
    .look_ahead_frametime_multiplier    = 0.3
};

int frequency_downsample_tracker = 0;
imu_event_handler xreal_event_handler;
void handle_xreal_event(uint64_t timestamp,
		   device3_event_type event,
		   const device3_ahrs_type* ahrs) {
    if (!xreal_event_handler) {
        fprintf(stderr, "xreal_event_handler not initialized, device_connect should be called first\n");
    } else {
        if (event == DEVICE3_EVENT_UPDATE && ++frequency_downsample_tracker % XREAL_DOWNSAMPLE_FACTOR == 0) {
            device3_quat_type quat = device3_get_orientation(ahrs);
            imu_quat_type imu_quat = { .w = quat.w, .x = quat.x, .y = quat.y, .z = quat.z };
            imu_quat_type nwu_quat = multiply_quaternions(imu_quat, nwu_conversion_quat);
            imu_vector_type nwu_euler = quaternion_to_euler(nwu_quat);

            xreal_event_handler((uint32_t) (timestamp / XREAL_TS_TO_MS_FACTOR), nwu_quat, nwu_euler);

            frequency_downsample_tracker == 0;
        }
    }
}

device3_type* glasses_imu;
device_properties_type* xreal_device_connect(imu_event_handler handler) {
    if (!glasses_imu) {
        glasses_imu = malloc(sizeof(device3_type));
    }

    int device_error = device3_open(glasses_imu, handle_xreal_event);

    bool success = device_error == DEVICE3_ERROR_NO_ERROR;
    if (success) {
        xreal_event_handler = handler;
        device_properties_type* device = malloc(sizeof(device_properties_type));
        *device = xreal_air_properties;

        device->imu_cycles_per_s /= XREAL_DOWNSAMPLE_FACTOR;
        device->imu_buffer_size /= XREAL_DOWNSAMPLE_FACTOR;
        device->hid_product_id = glasses_imu->product_id;
        device->hid_vendor_id = glasses_imu->vendor_id;

        return device;
    }

    return NULL;
};

void xreal_device_cleanup() {
    device3_close(glasses_imu);
};

void xreal_block_on_device(should_disconnect_callback should_disconnect_func) {
    device3_clear(glasses_imu);
    while (!should_disconnect_func()) {
        if (device3_read(glasses_imu, 1) != DEVICE3_ERROR_NO_ERROR) {
            break;
        }
    }

    xreal_device_cleanup();
};

bool xreal_device_is_sbs_mode() {
    return false;
};

bool xreal_device_set_sbs_mode(bool enabled) {
    return false;
};

const device_driver_type xreal_driver = {
    .device_connect_func                = xreal_device_connect,
    .block_on_device_func               = xreal_block_on_device,
    .device_is_sbs_mode_func            = xreal_device_is_sbs_mode,
    .device_set_sbs_mode_func           = xreal_device_set_sbs_mode,
    .device_cleanup_func                = xreal_device_cleanup
};