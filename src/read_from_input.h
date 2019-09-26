///! This is tedious. To instant read from a by-default line buffered terminal, we
/// need to change terminal settings. A select() with timeout is used to react to a keypress.

#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>

struct termios old = {0};
void init_term() {
    if (tcgetattr(STDIN_FILENO, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
}

void restore_term() {
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old) < 0)
        perror ("tcsetattr ~ICANON");
}


char getch_async() {
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(STDIN_FILENO, &readSet);
    struct timeval tv = {2, 0};
    if (select(STDIN_FILENO + 1, &readSet, NULL, NULL, &tv) < 0) perror("select");

    if (FD_ISSET(STDIN_FILENO, &readSet)) {
        char buf = EOF;
        if (read(STDIN_FILENO, &buf, 1) < 0)
            perror ("read()");
        return buf;
    }
    return EOF;
}
