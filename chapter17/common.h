int send_err(int fd, int errcode, const char* msg);
int send_fd(int fd, int fd_to_send);
int recv_fd(int fd, ssize_t (*userfunc)(int, const void*, size_t));

int send_fds1(int fd, int* fds_to_send, int n);
int recv_fds1(int fd, int n);

int send_fds2(int fd, int* fds_to_send, int n);
int recv_fds2(int fd, int n);