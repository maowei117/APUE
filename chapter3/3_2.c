#include <stdio.h>

int my_dup2(int oldfd, int newfd) {
    if (newfd == oldfd) {
        return newfd;
    }

}

int main() {
    return 0;
}