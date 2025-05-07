// Noah Gallego - CMPS-3600 Phase-2

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <X11/Xutil.h>

pid_t child_pid, xeyes_pid, xclock_pid = 0; // Track child process ID
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
void make_child_window(void);
void start_xclock(void);
void start_xeyes(void);

char **arguments; // Create Copy of Arguments for Global Use
char **environment;

int main(int argc, char *argv[], char *envp[]) {
    arguments = argv;
    environment = envp;

    if (argc > 2 && strcmp(arguments[2], "Child!") == 0) {
        is_child = 1;
        signal(SIGUSR1, handle_signal);
        signal(SIGUSR2, handle_signal);
    }

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

void make_child_window(void) {
    if (child_pid > 0) return; // Prevent multiple children

    child_pid = fork();
    if (child_pid == 0) { // Child process
        is_child = 1;
        signal(SIGUSR1, handle_signal);
        signal(SIGUSR2, handle_signal);
        signal(SIGTERM, handle_signal);

        char *args[] = {arguments[0], "0", "Child!", NULL};
        execve(arguments[0], args, environment);
        exit(0);
    } else {
        signal(SIGCHLD, handle_signal);
    }
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

    int x_offset = is_child ? 500 : 100;  // Move child far right
    int y_offset = is_child ? 525 : 100;   // Move child far down

    g.win = XCreateSimpleWindow(g.dpy, RootWindow(g.dpy, scr), 
                                x_offset, y_offset,  // Set window position
                                g.xres, g.yres, 0, 0x00ffffff, 0x00000000);

    XSizeHints hints;
    hints.flags = PPosition;  // Force window manager to respect position
    hints.x = x_offset;
    hints.y = y_offset;
    XSetWMNormalHints(g.dpy, g.win, &hints);

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
                    make_child_window();
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
            case XK_k:  
                if (xclock_pid > 0) {
                    kill(xclock_pid, SIGTERM);
                    xclock_pid = 0;
                } else {start_xclock();}
                break;
            case XK_x:
                if (xeyes_pid > 0) {
                    kill(xeyes_pid, SIGTERM);
                    xeyes_pid = 0;
                } else {start_xeyes();}
                break;
            case XK_Escape:
                if (child_pid > 0) {
                    kill(child_pid, SIGUSR2); // Tell child to exit
                    child_pid = 0;
                } else if (xclock_pid > 0) {
                    kill(xclock_pid, SIGTERM); // Kill xclock
                    xclock_pid = 0;
                } else if (xeyes_pid > 0) {
                    kill(xeyes_pid, SIGTERM); // Kill xeyes
                    xeyes_pid = 0;
                } else {
                    return 1;
                }

                // Reset parent window to default state
                render();
                break;
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
        XSetForeground(g.dpy, g.gc, child_color); // Green Background
        XClearWindow(g.dpy, g.win);
        render();
    } else if (sig == SIGUSR2 || sig == SIGTERM) {
        exit(0);
    } else if (sig == SIGCHLD) {
        int status;
        pid_t pid = waitpid(-1, &status, WUNTRACED | WNOHANG);

        if (pid == child_pid) {
            child_pid = 0;
        } else if (pid == xclock_pid) {
            xclock_pid = 0;
        } else if (pid == xeyes_pid) {
            xeyes_pid = 0;
        }
        render();
    }
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

    if (!is_child) {
        draw_string(20, 60, !is_child ? "Press C to Start a Child" : "Child Active");

        if (child_pid > 0) {
            draw_string(20, 80, "Press A to Change Color"); 
            draw_string(20, 100, "Press B to Terminate"); 
        } 

        // Update the text based on whether xeyes/xclock is running
        if (xeyes_pid > 0) {
            draw_string(20, 140, "Press X to close xeyes");
        } else {
            draw_string(20, 140, "Press X to start xeyes");
        }

        if (xclock_pid > 0) {
            draw_string(20, 160, "Press K to close xclock");
        } else {
            draw_string(20, 160, "Press K to start xclock");
        }

        draw_string(20, 180, "Press ESC for exit");
    } 
}

void start_xeyes() {
    xeyes_pid = fork();
    if (xeyes_pid == 0) { // Child process
        char *xeyes_args[] = {"/usr/bin/xeyes", "-fg", "red", NULL};
        execve(xeyes_args[0], xeyes_args, environment);
        perror("execve failed for xeyes");
        exit(1);
    } else {
        signal(SIGCHLD, handle_signal);
    }
}

void start_xclock() {
    xclock_pid = fork();
    if (xclock_pid == 0) { // Child process
        char *xclock_args[] = {"/usr/bin/xclock", "-update", "1", NULL};
        execve(xclock_args[0], xclock_args, environment);
        exit(1);
    } else {
        signal(SIGCHLD, handle_signal);
    }
}