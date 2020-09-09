#include <shadow.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    const char* user_name = "tommao";

    struct spwd* result = NULL;
    result = getspnam(user_name);
    if (result == NULL) {
        printf("Get passwd failed!\n");
        return -1;
    }
    printf("Name[%s] passwd[%s]\n", result->sp_namp, result->sp_pwdp);

    return 0;
}