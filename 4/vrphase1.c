// Noah Gallego - CMPS-3600 Phase-1

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <signal.h>

int child_pid = 0; // Track child process ID
int is_child = 0;
unsigned long child_color = 0x0000ff00; // Default Child Color

struct Global {
    Display *dpy;
    Window win;
    GC gc;
    int xres, yres;
} g;

void x11_cleanup_xwindows(void);
void x11_init_xwindows(void);
void x11_clear_window(void);
int check_keys(XEvent *e);
void render(void);
void handle_signal(int sig);
void draw_string(int left, int top, char* str);

int main(void) {
    srand(time(NULL)); // Initialize and seed with Time
    XEvent e;
    int done = 0;
    x11_init_xwindows();
    while (!done) {
        while (XPending(g.dpy)) {
            XNextEvent(g.dpy, &e);
            done = check_keys(&e);
            render();
        }
        usleep(4000);
    }
    x11_cleanup_xwindows();
    return 0;
}

void x11_cleanup_xwindows(void) {
    XDestroyWindow(g.dpy, g.win);
    XCloseDisplay(g.dpy);
    if (!is_child && child_pid > 0) {
        kill(child_pid, SIGUSR2); // Ensure child exits
    }
}

void x11_init_xwindows(void) {
    int scr;
    if (!(g.dpy = XOpenDisplay(NULL))) {
        fprintf(stderr, "ERROR: could not open display!\n");
        exit(EXIT_FAILURE);
    }
    scr = DefaultScreen(g.dpy);
    g.xres = 400;
    g.yres = 200;
    g.win = XCreateSimpleWindow(g.dpy, RootWindow(g.dpy, scr), 1, 1,
                            g.xres, g.yres, 0, 0x00ffffff, 0x00000000);
    XStoreName(g.dpy, g.win, is_child ? "Child Window" : "Parent Window");
    g.gc = XCreateGC(g.dpy, g.win, 0, NULL);
    XMapWindow(g.dpy, g.win);
    XSelectInput(g.dpy, g.win, ExposureMask | StructureNotifyMask |
                                PointerMotionMask | ButtonPressMask |
                                ButtonReleaseMask | KeyPressMask);
}

int check_keys(XEvent *e) {
    int key;
    if (e->type != KeyPress)
        return 0;
    key = XLookupKeysym(&e->xkey, 0);
    if (e->type == KeyPress) {
        switch (key) {
            case XK_c:
                if (child_pid == 0) {
                    child_pid = fork();
                    if (child_pid == 0) {
                        is_child = 1;
                        signal(SIGUSR1, handle_signal);
                        signal(SIGUSR2, handle_signal);
                        main(); // Child runs main but listens for signals
                        exit(0);
                    }
                }
                break;
            case XK_a:
                if (child_pid > 0) {
                    kill(child_pid, SIGUSR1); // Change child window color
                }
                break;
            case XK_b:
                if (child_pid > 0) {
                    kill(child_pid, SIGUSR2); // Terminate child
                    child_pid = 0;
                }
                break;
            case XK_Escape:
                kill(child_pid, SIGCHLD);
        }
    }
    return 0;
}

int gen_random_color() {
    unsigned long colors[] = {
        0x00ff0000,  // Red
        0x0000ff00,  // Green
        0x000000ff,  // Blue
        0x00ffff00,  // Yellow
        0x00ff00ff,  // Magenta
        0x0000ffff,  // Cyan
        0x00ffffff,  // White
        0x00808080,  // Gray
        0x00ff8000   // Orange
    };

    return colors[rand() % 9]; // Returns Color at Index 0 - 9 
}

void handle_signal(int sig) {
    if (sig == SIGUSR1) {
        child_color = gen_random_color();
        XSetForeground(g.dpy, g.gc, child_color); // Green background
        XClearWindow(g.dpy, g.win);
    } else if (sig == SIGUSR2) {
        exit(0);
    }
    render();
}

void draw_string(int left, int top, char *str) {
    XDrawString(g.dpy, g.win, g.gc, left, top, str, strlen(str));
}

void render(void) {
    XSetForeground(g.dpy, g.gc, is_child ? child_color : 0x00ff0000);
    XFillRectangle(g.dpy, g.win, g.gc, 0, 0, g.xres, g.yres);
    
    // Set text color (White)
    XSetForeground(g.dpy, g.gc, 0x00ffffff);
    XSetFont(g.dpy, g.gc, XLoadFont(g.dpy, "9x15bold"));
    draw_string(20, 30, is_child ? "Child Window" : "Parent Window");
    
    if (!is_child && child_pid > 0) {
        draw_string(20, 60, "Child Active");
        draw_string(20, 80, "Press A to Change Color");
        draw_string(20, 100, "Press B to Terminate");
    }
}
