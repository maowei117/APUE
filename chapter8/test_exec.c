#include <unistd.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    printf("In test_exec, args:\n");
    for (int i = 0; i < argc; i++) {
        printf("arg[%d]:%s\n", i, argv[i]);
    }
    printf("test_exec end\n");

    const char* file_name = "print_args_interpreter.sh";
    if (execvp(file_name, argv) == -1) {
        printf("Exec failed!\n");
    }

    return 0;
}