#include <func.h>

//子进程将文件描述符fd传输给父进程，父进程用此fd修改文件内容

int sendFd(int pipefd, int fd){
    char buff[4] = {0};
    //构建第二组成员
    struct iovec iov;
    iov.iov_base = buff;
    iov.iov_len = sizeof(buff);
    //构建第三组成员
    size_t len = CMSG_LEN(sizeof(fd));
    struct cmsghdr* cmsg = (struct cmsghdr*)calloc(1, len);
    cmsg->cmsg_len = len;
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    int* p = (int*)CMSG_DATA(cmsg);
    *p = fd;

    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsg;
    msg.msg_controllen = len;

    //发送数据
    int ret = sendmsg(pipefd, &msg, 0);
    printf("sendFd %d bytes.\n", ret);
    free(cmsg); //回收堆空间
    return 0;
}

int recvFd(int pipefd, int* pfd){
    char buff[4] = {0};
    //构建第二组成员
    struct iovec iov;
    iov.iov_base = buff;
    iov.iov_len = sizeof(buff);
    //构建第三组成员
    size_t len = CMSG_LEN(sizeof(int));
    struct cmsghdr* cmsg = (struct cmsghdr*)calloc(1, len);
    cmsg->cmsg_len = len;
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;

    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsg;
    msg.msg_controllen = len;

    //接收数据
    int ret = recvmsg(pipefd, &msg, 0);
    printf("recvFd %d bytes.\n", ret);
    *pfd = *(int*)CMSG_DATA(cmsg);
    printf("Received data: %s\n", buff); //打印接收到的数据
    free(cmsg);  //回收堆空间
    return 0;
}

int main(void)
{
    //创建套接字对
    int fds[2];
    socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);

    //创建子进程
    pid_t pid = fork();
    if(pid == -1){
        error(1, errno, "fork");
    }else if(pid == 0){  //子进程
        //子进程打开文件，通过套接字对发送文件描述符给父进程
        close(fds[0]);  //子进程关闭读端
        int fd = open("test.txt",O_RDWR);
        printf("child fd: %d\n", fd);
        if(fd == -1)    error(1, errno, "open");
        sendFd(fds[1], fd);
    }else{ //父进程
        //父进程接收文件描述符，并使用该描述符向文件写入字符串
        close(fds[1]);  //父进程关闭写端
        int parentFd;
        recvFd(fds[0], &parentFd);
        printf("parentFd:%d\n", parentFd);
        /* char buff[64]  = {0}; */
        char str[] = "hello from parent\n";
        write(parentFd, str, strlen(str));
    }
    
    return 0;
}
