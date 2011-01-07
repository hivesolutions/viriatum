# checks for the epoll
AC_CHECK_HEADER(sys/epoll.h, have_epoll_include=yes, have_epoll_include=no)

# checks if the user wants epoll
AC_ARG_ENABLE(epoll, AC_HELP_STRING([--disable-epoll], [Disable epoll() support]), wants_epoll="$enableval", wants_epoll="yes")

# in case the user wants epoll and the system has epoll
if test "x$have_epoll_include" = "xyes" && test "x$wants_epoll" = "xyes"; then
	AC_MSG_CHECKING(for epoll system call)
	AC_RUN_IFELSE([
                   #include <stdint.h>
                   #include <sys/param.h>
                   #include <sys/types.h>
                   #include <sys/syscall.h>
                   #include <sys/epoll.h>
                   #include <unistd.h>

                   int epoll_create(int size) {
                      return (syscall(__NR_epoll_create, size));
                   }

                   int main (int argc, char **argv) {
                       int epfd;
                       epfd = epoll_create(256);
                       exit(epfd == -1 ? 1 : 0);
                   }
                  ], have_epoll=yes, have_epoll=no, have_epoll=yes)
    AC_MSG_RESULT($have_epoll)
fi
