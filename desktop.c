#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* START BUTTON */
typedef struct {
    int x, y, w, h;
    uint8_t r, g, b;
    int pressed;
} Button;

int main() {
    int fb = open("/dev/fb0", O_RDWR);
    if(fb < 0){ perror("fb open"); return 1; }

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);
    ioctl(fb, FBIOGET_FSCREENINFO, &finfo);

    int width = vinfo.xres, height = vinfo.yres, bpp = vinfo.bits_per_pixel / 8;
    size_t screensize = width * height * bpp;
    uint8_t *fbp = mmap(0, screensize, PROT_READ|PROT_WRITE, MAP_SHARED, fb, 0);
    if((int)fbp==-1){ perror("mmap"); return 1; }

    /* ---- TŁO (gradient niebieski) ---- */
    for(int y=0;y<height;y++){
        for(int x=0;x<width;x++){
            int p = (x + y*width)*bpp;
            fbp[p+0] = 200;       // B
            fbp[p+1] = 120 + y/4; // G
            fbp[p+2] = 40;        // R
        }
    }

    /* ---- PASEK ZADAŃ ---- */
    int bar_h = 40;
    for(int y=height-bar_h;y<height;y++){
        for(int x=0;x<width;x++){
            int p = (x + y*width)*bpp;
            fbp[p+0]=50; fbp[p+1]=50; fbp[p+2]=50;
        }
    }

    /* ---- START BUTTON ---- */
    Button start = {10, height-bar_h+5, 100, bar_h-10, 0, 180, 0, 0};
    for(int y=start.y;y<start.y+start.h;y++){
        for(int x=start.x;x<start.x+start.w;x++){
            int p = (x + y*width)*bpp;
            fbp[p+0]=start.b; fbp[p+1]=start.g; fbp[p+2]=start.r;
        }
    }

    /* ---- Event loop (mysz) ---- */
    int mouse_fd = open("/dev/input/mice", O_RDONLY);
    if(mouse_fd < 0){ perror("mice"); return 1; }

    unsigned char data[3];
    while(1){
        read(mouse_fd, data, 3);
        int dx = (signed char)data[1];
        int dy = -(signed char)data[2]; // y-axis flipped
        static int mx=0,my=0;
        mx += dx; if(mx<0)mx=0; if(mx>=width)mx=width-1;
        my += dy; if(my<0)my=0; if(my>=height)my=height-1;

        // klik left button
        int left = data[0] & 0x1;
        if(left){
            if(mx>=start.x && mx<=start.x+start.w && my>=start.y && my<=start.y+start.h){
                if(!start.pressed){
                    start.pressed=1;
                    // zmiana koloru buttona
                    for(int yb=start.y;yb<start.y+start.h;yb++){
                        for(int xb=start.x;xb<start.x+start.w;xb++){
                            int p = (xb + yb*width)*bpp;
                            fbp[p+0]=0; fbp[p+1]=255; fbp[p+2]=0; // zielony jasny
                        }
                    }
                }
            }
        }else start.pressed=0;
    }

    munmap(fbp,screensize);
    close(fb);
    close(mouse_fd);
    return 0;
}
d
