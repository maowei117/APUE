#include <sys/utsname.h>
#include <stdio.h>

int main() {
    struct utsname uts_name;
    if (uname(&uts_name) != 0) {
        printf("Get uname failed!\n");
        return -1;
    }

    printf("sysname:%s\n", uts_name.sysname);
    printf("nodename:%s\n", uts_name.nodename);
    printf("release:%s\n", uts_name.release);
    printf("version:%s\n", uts_name.version);
    printf("machine:%s\n", uts_name.machine);
    printf("domainname:%s\n", uts_name.__domainname);
    return 0;
}