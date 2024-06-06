#include <func.h>

//sendFd(): 父进程向子进程传递文件描述符
int sendFd(int pipefd, int fd)
{
    char buff[4] = {0};
    //构建第二组成员
    struct iovec iov;
    iov.iov_base = buff;
    iov.iov_len = sizeof(buff);
    //构建第三组成员
    size_t len = CMSG_LEN(sizeof(fd));
    struct cmsghdr * cmsg = calloc(1, len);
    cmsg->cmsg_len = len;
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    int * p = (int *)CMSG_DATA(cmsg);
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
    free(cmsg);//回收堆空间
    return 0;
}

//recvFd(): 子进程接收父进程传递的文件描述符
int recvFd(int pipefd, int * pfd)
{
    char buff[4] = {0};
    //构建第二组成员
    struct iovec iov;
    iov.iov_base = buff;
    iov.iov_len = sizeof(buff);
    //构建第三组成员
    size_t len = CMSG_LEN(sizeof(int));
    struct cmsghdr * cmsg = calloc(1, len);
    cmsg->cmsg_len = len;
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;

    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsg;
    msg.msg_controllen = len;

    //接收数据, 默认情况下是阻塞的
    int ret = recvmsg(pipefd, &msg, 0);
    printf("recvFd %d bytes.\n", ret);
    *pfd = *(int*)CMSG_DATA(cmsg);
    free(cmsg);//回收堆空间
    return 0;
}
