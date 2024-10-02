#pragma once

#include <string>
enum class Syscall {
    read = 0,
    write = 1,
    open = 2,
    close = 3,
    stat = 4,
    fstat = 5,
    lstat = 6,
    poll = 7,
    lseek = 8,
    mmap = 9,
    mprotect = 10,
    munmap = 11,
    brk = 12,
    rt_sigaction = 13,
    rt_sigprocmask = 14,
    rt_sigreturn = 15,
    ioctl = 16,
    pread64 = 17,
    pwrite64 = 18,
    readv = 19,
    writev = 20,
    access = 21,
    pipe = 22,
    select = 23,
    sched_yield = 24,
    mremap = 25,
    msync = 26,
    mincore = 27,
    madvise = 28,
    shmget = 29,
    shmat = 30,
    shmctl = 31,
    dup = 32,
    dup2 = 33,
    pause = 34,
    nanosleep = 35,
    getitimer = 36,
    alarm = 37,
    setitimer = 38,
    getpid = 39,
    sendfile = 40,
    socket = 41,
    connect = 42,
    accept = 43,
    sendto = 44,
    recvfrom = 45,
    sendmsg = 46,
    recvmsg = 47,
    shutdown = 48,
    bind = 49,
    listen = 50,
    getsockname = 51,
    getpeername = 52,
    socketpair = 53,
    setsockopt = 54,
    getsockopt = 55,
    clone = 56,
    fork = 57,
    vfork = 58,
    execve = 59,
    exit = 60,
    wait4 = 61,
    kill = 62,
    uname = 63,
    semget = 64,
    semop = 65,
    semctl = 66,
    shmdt = 67,
    msgget = 68,
    msgsnd = 69,
    msgrcv = 70,
    msgctl = 71,
    fcntl = 72,
    flock = 73,
    fsync = 74,
    fdatasync = 75,
    truncate = 76,
    ftruncate = 77,
    getdents = 78,
    getcwd = 79,
    chdir = 80,
    fchdir = 81,
    rename = 82,
    mkdir = 83,
    rmdir = 84,
    creat = 85,
    link = 86,
    unlink = 87,
    symlink = 88,
    readlink = 89,
    chmod = 90,
    fchmod = 91,
    chown = 92,
    fchown = 93,
    lchown = 94,
    umask = 95,
    gettimeofday = 96,
    getrlimit = 97,
    getrusage = 98,
    sysinfo = 99,
    times = 100,
    ptrace = 101,
    getuid = 102,
    syslog = 103,
    getgid = 104,
    setuid = 105,
    setgid = 106,
    geteuid = 107,
    getegid = 108,
    setpgid = 109,
    getppid = 110,
    getpgrp = 111,
    setsid = 112,
    setreuid = 113,
    setregid = 114,
    getgroups = 115,
    setgroups = 116,
    setresuid = 117,
    getresuid = 118,
    setresgid = 119,
    getresgid = 120,
    getpgid = 121,
    setfsuid = 122,
    setfsgid = 123,
    getsid = 124,
    capget = 125,
    capset = 126,
    rt_sigpending = 127,
    rt_sigtimedwait = 128,
    rt_sigqueueinfo = 129,
    rt_sigsuspend = 130,
    sigaltstack = 131,
    utime = 132,
    mknod = 133,
    uselib = 134,
    personality = 135,
    ustat = 136,
    statfs = 137,
    fstatfs = 138,
    sysfs = 139,
    getpriority = 140,
    setpriority = 141,
    sched_setparam = 142,
    sched_getparam = 143,
    sched_setscheduler = 144,
    sched_getscheduler = 145,
    sched_get_priority_max = 146,
    sched_get_priority_min = 147,
    sched_rr_get_interval = 148,
    mlock = 149,
    munlock = 150,
    mlockall = 151,
    munlockall = 152,
    vhangup = 153,
    modify_ldt = 154,
    pivot_root = 155,
    _sysctl = 156,
    prctl = 157,
    arch_prctl = 158,
    adjtimex = 159,
    setrlimit = 160,
    chroot = 161,
    sync = 162,
    acct = 163,
    settimeofday = 164,
    mount = 165,
    umount2 = 166,
    swapon = 167,
    swapoff = 168,
    reboot = 169,
    sethostname = 170,
    setdomainname = 171,
    iopl = 172,
    ioperm = 173,
    create_module = 174,
    init_module = 175,
    delete_module = 176,
    get_kernel_syms = 177,
    query_module = 178,
    quotactl = 179,
    nfsservctl = 180,
    getpmsg = 181,
    putpmsg = 182,
    afs_syscall = 183,
    tuxcall = 184,
    security = 185,
    gettid = 186,
    readahead = 187,
    setxattr = 188,
    lsetxattr = 189,
    fsetxattr = 190,
    getxattr = 191,
    lgetxattr = 192,
    fgetxattr = 193,
    listxattr = 194,
    llistxattr = 195,
    flistxattr = 196,
    removexattr = 197,
    lremovexattr = 198,
    fremovexattr = 199,
    tkill = 200,
    time = 201,
    futex = 202,
    sched_setaffinity = 203,
    sched_getaffinity = 204,
    set_thread_area = 205,
    io_setup = 206,
    io_destroy = 207,
    io_getevents = 208,
    io_submit = 209,
    io_cancel = 210,
    get_thread_area = 211,
    lookup_dcookie = 212,
    epoll_create = 213,
    epoll_ctl_old = 214,
    epoll_wait_old = 215,
    remap_file_pages = 216,
    getdents64 = 217,
    set_tid_address = 218,
    restart_syscall = 219,
    semtimedop = 220,
    fadvise64 = 221,
    timer_create = 222,
    timer_settime = 223,
    timer_gettime = 224,
    timer_getoverrun = 225,
    timer_delete = 226,
    clock_settime = 227,
    clock_gettime = 228,
    clock_getres = 229,
    clock_nanosleep = 230,
    exit_group = 231,
    epoll_wait = 232,
    epoll_ctl = 233,
    tgkill = 234,
    utimes = 235,
    vserver = 236,
    mbind = 237,
    set_mempolicy = 238,
    get_mempolicy = 239,
    mq_open = 240,
    mq_unlink = 241,
    mq_timedsend = 242,
    mq_timedreceive = 243,
    mq_notify = 244,
    mq_getsetattr = 245,
    kexec_load = 246,
    waitid = 247,
    add_key = 248,
    request_key = 249,
    keyctl = 250,
    ioprio_set = 251,
    ioprio_get = 252,
    inotify_init = 253,
    inotify_add_watch = 254,
    inotify_rm_watch = 255,
    migrate_pages = 256,
    openat = 257,
    mkdirat = 258,
    mknodat = 259,
    fchownat = 260,
    futimesat = 261,
    newfstatat = 262,
    unlinkat = 263,
    renameat = 264,
    linkat = 265,
    symlinkat = 266,
    readlinkat = 267,
    fchmodat = 268,
    faccessat = 269,
    pselect6 = 270,
    ppoll = 271,
    unshare = 272,
    set_robust_list = 273,
    get_robust_list = 274,
    splice = 275,
    tee = 276,
    sync_file_range = 277,
    vmsplice = 278,
    move_pages = 279,
    utimensat = 280,
    epoll_pwait = 281,
    signalfd = 282,
    timerfd_create = 283,
    eventfd = 284,
    fallocate = 285,
    timerfd_settime = 286,
    timerfd_gettime = 287,
    accept4 = 288,
    signalfd4 = 289,
    eventfd2 = 290,
    epoll_create1 = 291,
    dup3 = 292,
    pipe2 = 293,
    inotify_init1 = 294,
    preadv = 295,
    pwritev = 296,
    rt_tgsigqueueinfo = 297,
    perf_event_open = 298,
    recvmmsg = 299,
    fanotify_init = 300,
    fanotify_mark = 301,
    prlimit64 = 302,
    name_to_handle_at = 303,
    open_by_handle_at = 304,
    clock_adjtime = 305,
    syncfs = 306,
    sendmmsg = 307,
    setns = 308,
    getcpu = 309,
    process_vm_readv = 310,
    process_vm_writev = 311,
    kcmp = 312,
    finit_module = 313,
    sched_setattr = 314,
    sched_getattr = 315,
    renameat2 = 316,
    seccomp = 317,
    getrandom = 318,
    memfd_create = 319,
    kexec_file_load = 320,
    bpf = 321,
    execveat = 322,
    userfaultfd = 323,
    membarrier = 324,
    mlock2 = 325,
    copy_file_range = 326,
    preadv2 = 327,
    pwritev2 = 328,
    pkey_mprotect = 329,
    pkey_alloc = 330,
    pkey_free = 331,
    statx = 332,
};

inline std::string syscallToString(Syscall sc) {
    switch (sc) {
        case Syscall::read:                   return "read";
        case Syscall::write:                  return "write";
        case Syscall::open:                   return "open";
        case Syscall::close:                  return "close";
        case Syscall::stat:                   return "stat";
        case Syscall::fstat:                  return "fstat";
        case Syscall::lstat:                  return "lstat";
        case Syscall::poll:                   return "poll";
        case Syscall::lseek:                  return "lseek";
        case Syscall::mmap:                   return "mmap";
        case Syscall::mprotect:               return "mprotect";
        case Syscall::munmap:                 return "munmap";
        case Syscall::brk:                    return "brk";
        case Syscall::rt_sigaction:           return "rt_sigaction";
        case Syscall::rt_sigprocmask:         return "rt_sigprocmask";
        case Syscall::rt_sigreturn:           return "rt_sigreturn";
        case Syscall::ioctl:                  return "ioctl";
        case Syscall::pread64:                return "pread64";
        case Syscall::pwrite64:               return "pwrite64";
        case Syscall::readv:                  return "readv";
        case Syscall::writev:                 return "writev";
        case Syscall::access:                 return "access";
        case Syscall::pipe:                   return "pipe";
        case Syscall::select:                 return "select";
        case Syscall::sched_yield:            return "sched_yield";
        case Syscall::mremap:                 return "mremap";
        case Syscall::msync:                  return "msync";
        case Syscall::mincore:                return "mincore";
        case Syscall::madvise:                return "madvise";
        case Syscall::shmget:                 return "shmget";
        case Syscall::shmat:                  return "shmat";
        case Syscall::shmctl:                 return "shmctl";
        case Syscall::dup:                    return "dup";
        case Syscall::dup2:                   return "dup2";
        case Syscall::pause:                  return "pause";
        case Syscall::nanosleep:              return "nanosleep";
        case Syscall::getitimer:              return "getitimer";
        case Syscall::alarm:                  return "alarm";
        case Syscall::setitimer:              return "setitimer";
        case Syscall::getpid:                 return "getpid";
        case Syscall::sendfile:               return "sendfile";
        case Syscall::socket:                 return "socket";
        case Syscall::connect:                return "connect";
        case Syscall::accept:                 return "accept";
        case Syscall::sendto:                 return "sendto";
        case Syscall::recvfrom:               return "recvfrom";
        case Syscall::sendmsg:                return "sendmsg";
        case Syscall::recvmsg:                return "recvmsg";
        case Syscall::shutdown:               return "shutdown";
        case Syscall::bind:                   return "bind";
        case Syscall::listen:                 return "listen";
        case Syscall::getsockname:            return "getsockname";
        case Syscall::getpeername:            return "getpeername";
        case Syscall::socketpair:             return "socketpair";
        case Syscall::setsockopt:             return "setsockopt";
        case Syscall::getsockopt:             return "getsockopt";
        case Syscall::clone:                  return "clone";
        case Syscall::fork:                   return "fork";
        case Syscall::vfork:                  return "vfork";
        case Syscall::execve:                 return "execve";
        case Syscall::exit:                   return "exit";
        case Syscall::wait4:                  return "wait4";
        case Syscall::kill:                   return "kill";
        case Syscall::uname:                  return "uname";
        case Syscall::semget:                 return "semget";
        case Syscall::semop:                  return "semop";
        case Syscall::semctl:                 return "semctl";
        case Syscall::shmdt:                  return "shmdt";
        case Syscall::msgget:                 return "msgget";
        case Syscall::msgsnd:                 return "msgsnd";
        case Syscall::msgrcv:                 return "msgrcv";
        case Syscall::msgctl:                 return "msgctl";
        case Syscall::fcntl:                  return "fcntl";
        case Syscall::flock:                  return "flock";
        case Syscall::fsync:                  return "fsync";
        case Syscall::fdatasync:              return "fdatasync";
        case Syscall::truncate:               return "truncate";
        case Syscall::ftruncate:              return "ftruncate";
        case Syscall::getdents:               return "getdents";
        case Syscall::getcwd:                 return "getcwd";
        case Syscall::chdir:                  return "chdir";
        case Syscall::fchdir:                 return "fchdir";
        case Syscall::rename:                 return "rename";
        case Syscall::mkdir:                  return "mkdir";
        case Syscall::rmdir:                  return "rmdir";
        case Syscall::creat:                  return "creat";
        case Syscall::link:                   return "link";
        case Syscall::unlink:                 return "unlink";
        case Syscall::symlink:                return "symlink";
        case Syscall::readlink:               return "readlink";
        case Syscall::chmod:                  return "chmod";
        case Syscall::fchmod:                 return "fchmod";
        case Syscall::chown:                  return "chown";
        case Syscall::fchown:                 return "fchown";
        case Syscall::lchown:                 return "lchown";
        case Syscall::umask:                  return "umask";
        case Syscall::gettimeofday:           return "gettimeofday";
        case Syscall::getrlimit:              return "getrlimit";
        case Syscall::getrusage:              return "getrusage";
        case Syscall::sysinfo:                return "sysinfo";
        case Syscall::times:                  return "times";
        case Syscall::ptrace:                 return "ptrace";
        case Syscall::getuid:                 return "getuid";
        case Syscall::syslog:                 return "syslog";
        case Syscall::getgid:                 return "getgid";
        case Syscall::setuid:                 return "setuid";
        case Syscall::setgid:                 return "setgid";
        case Syscall::geteuid:                return "geteuid";
        case Syscall::getegid:                return "getegid";
        case Syscall::setpgid:                return "setpgid";
        case Syscall::getppid:                return "getppid";
        case Syscall::getpgrp:                return "getpgrp";
        case Syscall::setsid:                 return "setsid";
        case Syscall::setreuid:               return "setreuid";
        case Syscall::setregid:               return "setregid";
        case Syscall::getgroups:              return "getgroups";
        case Syscall::setgroups:              return "setgroups";
        case Syscall::setresuid:              return "setresuid";
        case Syscall::getresuid:              return "getresuid";
        case Syscall::setresgid:              return "setresgid";
        case Syscall::getresgid:              return "getresgid";
        case Syscall::getpgid:                return "getpgid";
        case Syscall::setfsuid:               return "setfsuid";
        case Syscall::setfsgid:               return "setfsgid";
        case Syscall::getsid:                 return "getsid";
        case Syscall::capget:                 return "capget";
        case Syscall::capset:                 return "capset";
        case Syscall::rt_sigpending:          return "rt_sigpending";
        case Syscall::rt_sigtimedwait:        return "rt_sigtimedwait";
        case Syscall::rt_sigqueueinfo:        return "rt_sigqueueinfo";
        case Syscall::rt_sigsuspend:          return "rt_sigsuspend";
        case Syscall::sigaltstack:            return "sigaltstack";
        case Syscall::utime:                  return "utime";
        case Syscall::mknod:                  return "mknod";
        case Syscall::uselib:                 return "uselib";
        case Syscall::personality:            return "personality";
        case Syscall::ustat:                  return "ustat";
        case Syscall::statfs:                 return "statfs";
        case Syscall::fstatfs:                return "fstatfs";
        case Syscall::sysfs:                  return "sysfs";
        case Syscall::getpriority:            return "getpriority";
        case Syscall::setpriority:            return "setpriority";
        case Syscall::sched_setparam:         return "sched_setparam";
        case Syscall::sched_getparam:         return "sched_getparam";
        case Syscall::sched_setscheduler:     return "sched_setscheduler";
        case Syscall::sched_getscheduler:     return "sched_getscheduler";
        case Syscall::sched_get_priority_max: return "sched_get_priority_max";
        case Syscall::sched_get_priority_min: return "sched_get_priority_min";
        case Syscall::sched_rr_get_interval:  return "sched_rr_get_interval";
        case Syscall::mlock:                  return "mlock";
        case Syscall::munlock:                return "munlock";
        case Syscall::mlockall:               return "mlockall";
        case Syscall::munlockall:             return "munlockall";
        case Syscall::vhangup:                return "vhangup";
        case Syscall::modify_ldt:             return "modify_ldt";
        case Syscall::pivot_root:             return "pivot_root";
        case Syscall::_sysctl:                return "_sysctl";
        case Syscall::prctl:                  return "prctl";
        case Syscall::arch_prctl:             return "arch_prctl";
        case Syscall::adjtimex:               return "adjtimex";
        case Syscall::setrlimit:              return "setrlimit";
        case Syscall::chroot:                 return "chroot";
        case Syscall::sync:                   return "sync";
        case Syscall::acct:                   return "acct";
        case Syscall::settimeofday:           return "settimeofday";
        case Syscall::mount:                  return "mount";
        case Syscall::umount2:                return "umount2";
        case Syscall::swapon:                 return "swapon";
        case Syscall::swapoff:                return "swapoff";
        case Syscall::reboot:                 return "reboot";
        case Syscall::sethostname:            return "sethostname";
        case Syscall::setdomainname:          return "setdomainname";
        case Syscall::iopl:                   return "iopl";
        case Syscall::ioperm:                 return "ioperm";
        case Syscall::create_module:          return "create_module";
        case Syscall::init_module:            return "init_module";
        case Syscall::delete_module:          return "delete_module";
        case Syscall::get_kernel_syms:        return "get_kernel_syms";
        case Syscall::query_module:           return "query_module";
        case Syscall::quotactl:               return "quotactl";
        case Syscall::nfsservctl:             return "nfsservctl";
        case Syscall::getpmsg:                return "getpmsg";
        case Syscall::putpmsg:                return "putpmsg";
        case Syscall::afs_syscall:            return "afs_syscall";
        case Syscall::tuxcall:                return "tuxcall";
        case Syscall::security:               return "security";
        case Syscall::gettid:                 return "gettid";
        case Syscall::readahead:              return "readahead";
        case Syscall::setxattr:               return "setxattr";
        case Syscall::lsetxattr:              return "lsetxattr";
        case Syscall::fsetxattr:              return "fsetxattr";
        case Syscall::getxattr:               return "getxattr";
        case Syscall::lgetxattr:              return "lgetxattr";
        case Syscall::fgetxattr:              return "fgetxattr";
        case Syscall::listxattr:              return "listxattr";
        case Syscall::llistxattr:             return "llistxattr";
        case Syscall::flistxattr:             return "flistxattr";
        case Syscall::removexattr:            return "removexattr";
        case Syscall::lremovexattr:           return "lremovexattr";
        case Syscall::fremovexattr:           return "fremovexattr";
        case Syscall::tkill:                  return "tkill";
        case Syscall::time:                   return "time";
        case Syscall::futex:                  return "futex";
        case Syscall::sched_setaffinity:      return "sched_setaffinity";
        case Syscall::sched_getaffinity:      return "sched_getaffinity";
        case Syscall::set_thread_area:        return "set_thread_area";
        case Syscall::io_setup:               return "io_setup";
        case Syscall::io_destroy:             return "io_destroy";
        case Syscall::io_getevents:           return "io_getevents";
        case Syscall::io_submit:              return "io_submit";
        case Syscall::io_cancel:              return "io_cancel";
        case Syscall::get_thread_area:        return "get_thread_area";
        case Syscall::lookup_dcookie:         return "lookup_dcookie";
        case Syscall::epoll_create:           return "epoll_create";
        case Syscall::epoll_ctl_old:          return "epoll_ctl_old";
        case Syscall::epoll_wait_old:         return "epoll_wait_old";
        case Syscall::remap_file_pages:       return "remap_file_pages";
        case Syscall::getdents64:             return "getdents64";
        case Syscall::set_tid_address:        return "set_tid_address";
        case Syscall::restart_syscall:        return "restart_syscall";
        case Syscall::semtimedop:             return "semtimedop";
        case Syscall::fadvise64:              return "fadvise64";
        case Syscall::timer_create:           return "timer_create";
        case Syscall::timer_settime:          return "timer_settime";
        case Syscall::timer_gettime:          return "timer_gettime";
        case Syscall::timer_getoverrun:       return "timer_getoverrun";
        case Syscall::timer_delete:           return "timer_delete";
        case Syscall::clock_settime:          return "clock_settime";
        case Syscall::clock_gettime:          return "clock_gettime";
        case Syscall::clock_getres:           return "clock_getres";
        case Syscall::clock_nanosleep:        return "clock_nanosleep";
        case Syscall::exit_group:             return "exit_group";
        case Syscall::epoll_wait:             return "epoll_wait";
        case Syscall::epoll_ctl:              return "epoll_ctl";
        case Syscall::tgkill:                 return "tgkill";
        case Syscall::utimes:                 return "utimes";
        case Syscall::vserver:                return "vserver";
        case Syscall::mbind:                  return "mbind";
        case Syscall::set_mempolicy:          return "set_mempolicy";
        case Syscall::get_mempolicy:          return "get_mempolicy";
        case Syscall::mq_open:                return "mq_open";
        case Syscall::mq_unlink:              return "mq_unlink";
        case Syscall::mq_timedsend:           return "mq_timedsend";
        case Syscall::mq_timedreceive:        return "mq_timedreceive";
        case Syscall::mq_notify:              return "mq_notify";
        case Syscall::mq_getsetattr:          return "mq_getsetattr";
        case Syscall::kexec_load:             return "kexec_load";
        case Syscall::waitid:                 return "waitid";
        case Syscall::add_key:                return "add_key";
        case Syscall::request_key:            return "request_key";
        case Syscall::keyctl:                 return "keyctl";
        case Syscall::ioprio_set:             return "ioprio_set";
        case Syscall::ioprio_get:             return "ioprio_get";
        case Syscall::inotify_init:           return "inotify_init";
        case Syscall::inotify_add_watch:      return "inotify_add_watch";
        case Syscall::inotify_rm_watch:       return "inotify_rm_watch";
        case Syscall::migrate_pages:          return "migrate_pages";
        case Syscall::openat:                 return "openat";
        case Syscall::mkdirat:                return "mkdirat";
        case Syscall::mknodat:                return "mknodat";
        case Syscall::fchownat:               return "fchownat";
        case Syscall::futimesat:              return "futimesat";
        case Syscall::newfstatat:             return "newfstatat";
        case Syscall::unlinkat:               return "unlinkat";
        case Syscall::renameat:               return "renameat";
        case Syscall::linkat:                 return "linkat";
        case Syscall::symlinkat:              return "symlinkat";
        case Syscall::readlinkat:             return "readlinkat";
        case Syscall::fchmodat:               return "fchmodat";
        case Syscall::faccessat:              return "faccessat";
        case Syscall::pselect6:               return "pselect6";
        case Syscall::ppoll:                  return "ppoll";
        case Syscall::unshare:                return "unshare";
        case Syscall::set_robust_list:        return "set_robust_list";
        case Syscall::get_robust_list:        return "get_robust_list";
        case Syscall::splice:                 return "splice";
        case Syscall::tee:                    return "tee";
        case Syscall::sync_file_range:        return "sync_file_range";
        case Syscall::vmsplice:               return "vmsplice";
        case Syscall::move_pages:             return "move_pages";
        case Syscall::utimensat:              return "utimensat";
        case Syscall::epoll_pwait:            return "epoll_pwait";
        case Syscall::signalfd:               return "signalfd";
        case Syscall::timerfd_create:         return "timerfd_create";
        case Syscall::eventfd:                return "eventfd";
        case Syscall::fallocate:              return "fallocate";
        case Syscall::timerfd_settime:        return "timerfd_settime";
        case Syscall::timerfd_gettime:        return "timerfd_gettime";
        case Syscall::accept4:                return "accept4";
        case Syscall::signalfd4:              return "signalfd4";
        case Syscall::eventfd2:               return "eventfd2";
        case Syscall::epoll_create1:          return "epoll_create1";
        case Syscall::dup3:                   return "dup3";
        case Syscall::pipe2:                  return "pipe2";
        case Syscall::inotify_init1:          return "inotify_init1";
        case Syscall::preadv:                 return "preadv";
        case Syscall::pwritev:                return "pwritev";
        case Syscall::rt_tgsigqueueinfo:      return "rt_tgsigqueueinfo";
        case Syscall::perf_event_open:        return "perf_event_open";
        case Syscall::recvmmsg:               return "recvmmsg";
        case Syscall::fanotify_init:          return "fanotify_init";
        case Syscall::fanotify_mark:          return "fanotify_mark";
        case Syscall::prlimit64:              return "prlimit64";
        case Syscall::name_to_handle_at:      return "name_to_handle_at";
        case Syscall::open_by_handle_at:      return "open_by_handle_at";
        case Syscall::clock_adjtime:          return "clock_adjtime";
        case Syscall::syncfs:                 return "syncfs";
        case Syscall::sendmmsg:               return "sendmmsg";
        case Syscall::setns:                  return "setns";
        case Syscall::getcpu:                 return "getcpu";
        case Syscall::process_vm_readv:       return "process_vm_readv";
        case Syscall::process_vm_writev:      return "process_vm_writev";
        case Syscall::kcmp:                   return "kcmp";
        case Syscall::finit_module:           return "finit_module";
        case Syscall::sched_setattr:          return "sched_setattr";
        case Syscall::sched_getattr:          return "sched_getattr";
        case Syscall::renameat2:              return "renameat2";
        case Syscall::seccomp:                return "seccomp";
        case Syscall::getrandom:              return "getrandom";
        case Syscall::memfd_create:           return "memfd_create";
        case Syscall::kexec_file_load:        return "kexec_file_load";
        case Syscall::bpf:                    return "bpf";
        case Syscall::execveat:               return "execveat";
        case Syscall::userfaultfd:            return "userfaultfd";
        case Syscall::membarrier:             return "membarrier";
        case Syscall::mlock2:                 return "mlock2";
        case Syscall::copy_file_range:        return "copy_file_range";
        case Syscall::preadv2:                return "preadv2";
        case Syscall::pwritev2:               return "pwritev2";
        case Syscall::pkey_mprotect:          return "pkey_mprotect";
        case Syscall::pkey_alloc:             return "pkey_alloc";
        case Syscall::pkey_free:              return "pkey_free";
        case Syscall::statx:                  return "statx";
        default:                              return "UNKNOWN";
    }
}
