#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
    uint32_t vga_ctl = inl(VGACTL_ADDR);
    uint16_t height = (uint16_t)vga_ctl;
    uint16_t width = (uint16_t)(vga_ctl >> 16);
    *cfg = (AM_GPU_CONFIG_T) {
        .present = true, .has_accel = false,
        .width = width, .height = height,
        .vmemsz = 0
    };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
    int x = ctl->x, y = ctl->y;
    uint32_t *pixels = ctl->pixels;
    int width = ctl->w, height = ctl->h;

    uint32_t *fb = (uint32_t*)FB_ADDR;
    uint32_t screen_width = inl(VGACTL_ADDR) >> 16;

    size_t i_pixels = 0;
    for (int i = y; i < y + height; i ++) {
        for (int j = x; j < x + width; j ++) {
            fb[i * screen_width + j] = pixels[i_pixels];
            i_pixels ++;
        }
    }
    if (ctl->sync) {
        outl(SYNC_ADDR, 1);
    }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
