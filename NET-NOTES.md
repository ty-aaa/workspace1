# 网络编程基础 · 速查笔记

> 起点：`~/netgame/netgame.cpp`（UDP 版联机）
> 目标：看懂它、然后自己写一个 TCP 回显服务器
>
> 这份笔记按「先理解概念 → 再查函数」的顺序写。
> 函数参数记不住没关系，翻下面的表格就行。

---

# 第一部分：概念

## 1.1 socket 是什么

**socket 就是一个文件描述符（fd），一个整数。**

```c
int sock = socket(...);     // 返回 3、4、5 这种小数字
```

Unix 有句名言叫 **「一切皆文件」**：

- `open()` 一个文件 → 得到一个 fd
- `socket()` 一个套接字 → 得到一个 fd

**是同一个编号空间，同一个概念。** 你对它 `read`/`write` 就像读写文件。

区别只在于：文件那头是硬盘，socket 那头是**网络**。

> **名字来源**：socket 意思是「插座」。两根电线插在一起 = 通信的两端。
> 你在一头写，另一头能读。

## 1.2 两个流（Stream）

socket 是**双向**的，同时有两条流：

```
      我                                    对方
       │  ──────── 我 send() 的数据 ────────>  │
       │  <─────── 对方 send() 的数据 ───────  │
```

所以 `send` 和 `recv` 是**独立**的，互不干扰。你发你的，我发我的。

## 1.3 IP + 端口 = 一个地址

```
192.168.1.5 : 7777
└────┬────┘   └─┬─┘
  找到哪台机器   找到机器上的哪个程序
```

- **IP** = 哪台电脑
- **端口** = 这台电脑上的哪个程序（0~65535）

一台电脑同时跑着浏览器、游戏、聊天软件，靠端口区分。
**你自己的程序必须绑一个别的程序没占用的端口。**

---

# 第二部分：打电话类比（核心）

**这一套类比能一直用到最后，包括解释那个最容易懵的「为什么有两个 fd」。**

## 2.1 服务器 vs 客户端

| | 服务器（等人打进来） | 客户端（主动打出去） |
|---|---|---|
| 类比 | 开一家电话局 | 拿起手机拨号 |
| 流程 | socket → bind → listen → **accept** → send/recv | socket → **connect** → send/recv |

**关键差异：**

- 服务器**没有** `connect`（它不主动找谁）
- 客户端**没有** `bind`/`listen`/`accept`（它不需要等人来）

两边用的函数不一样，因为**做的事情不一样**。

## 2.2 函数对照表

| 函数 | 类比 | 实际做的事 | 服务器 | 客户端 |
|---|---|:--:|:--:|
| `socket()` | 买一台电话机 | 创建通信端点 | ✓ | ✓ |
| `bind()` | 申请一个电话号码 | 绑定 IP + 端口 | ✓ | 可选 |
| `listen()` | 开通「可接来电」 | 变成监听套接字 | ✓ | ✗ |
| `accept()` | 铃响，接起来 | 接受一个连接 | ✓ | ✗ |
| `connect()` | 拨号给别人 | 主动发起连接 | ✗ | ✓ |
| `send()` | 对着话筒说 | 发数据 | ✓ | ✓ |
| `recv()` | 听对方说 | 收数据 | ✓ | ✓ |
| `close()` | 挂电话 | 关闭套接字 | ✓ | ✓ |

> 客户端**可选** `bind`：不 bind 的话系统会随便给它一个临时端口。
> 你去打电话，电话号码是运营商分配的，不用自己申请 —— 一个道理。
> `~/netgame/netgame.cpp` 里的 `OpenClient()` 就是这么做的（`sin_port = 0`）。

---

# 第三部分：那个最容易懵的点 —— 为什么有两个 fd

```c
int listen_fd = socket(...);        // 电话总机号码
bind(listen_fd, ...);
listen(listen_fd, 10);

int conn_fd = accept(listen_fd, ...);   // ← 返回【新的】fd！
// 之后 send/recv 全都用 conn_fd，不是 listen_fd
```

## 为什么？

- **`listen_fd`** 代表**那个号码本身**，职责只有一个：**等电话铃响**
- **`conn_fd`** 代表**这一次具体的通话**，收发数据用它

**一个总机号码，可以同时接通几十个来电，每个来电各有自己的线路。**

```
listen_fd  ──等待连接──>  accept() ──> conn_fd_1  ← 客户端 A 的线路
                              │
                              ├────────> conn_fd_2  ← 客户端 B 的线路
                              │
                              └────────> conn_fd_3  ← 客户端 C 的线路
```

**这就是为什么一个服务器能同时服务很多客户端。** 这是服务器编程的地基。

接受完一个连接后，`listen_fd` **继续留在原地等人来** ——
所以 `accept` 通常放在一个 `while` 循环里。

---

# 第四部分：TCP vs UDP

## 4.1 一句话区别

```
TCP 像打电话：先接通，双方对着话筒说，最后挂断
UDP 像寄明信片：写好地址，扔进邮筒，谁也不知道能不能到
```

## 4.2 对比表

| | TCP | UDP |
|---|---|---|
| 连接 | **有连接**（要先握手） | **无连接** |
| 可靠性 | 保证送达、保证顺序 | 不保证，可能丢、可能乱序 |
| 消息边界 | **没有**（字节流）⚠️ | **有**（一次一个包） |
| API 复杂度 | 多 `listen`/`accept`/`connect` | 少，直接收发 |
| 收发的地址 | **不用带**（线路已确定） | **要带**（每张明信片都得写地址） |
| 典型用途 | Web、API、文件传输 | 实时游戏、视频、DNS |
| 你写过的 | — | `~/netgame/netgame.cpp` |

## 4.3 为什么 API 长得不一样

**UDP 没有「连接」这个概念** —— 没有通话，自然就没有「接起来」(`accept`)
和「拨号」(`connect`) 这回事。

因为没连接，UDP 每发一个包都得写明**发给谁**：

```c
sendto(sock, buf, len, 0, &peer_addr, sizeof(peer_addr));   // ← 带地址
recvfrom(sock, buf, len, 0, &from_addr, &addrlen);          // ← 带地址
```

而 TCP 一旦连上，`send`/`recv` **不用带地址**，因为「这条线路通向谁」已经确定了：

```c
send(conn_fd, buf, len, 0);      // ← 不带地址
recv(conn_fd, buf, len, 0);      // ← 不带地址
```

**这个对比，就是 UDP 和 TCP API 差异的全部来源。**

## 4.4 你在 netgame.cpp 里已经用过这些

去 `~/netgame/netgame.cpp` 里找 `Net` 类：

| 代码 | 在哪 | 干什么 |
|---|---|---|
| `socket(AF_INET, SOCK_DGRAM, 0)` | `MakeSocket()` | 创建套接字 |
| `bind(sock, (sockaddr*)&me, ...)` | `OpenHost()` | 绑定地址端口 |
| `sendto(sock, &p, ..., &peer)` | `SendState()` | 往某地址发包 |
| `recvfrom(sock, &p, ..., &from)` | `Poll()` | 从某地址收包 |
| `fcntl(sock, F_SETFL, ... \| O_NONBLOCK)` | `MakeSocket()` | 设非阻塞 |
| `close(sock)` | `Close()` | 关闭 |

**你已经用过 6 个 socket 函数了。** TCP 只是多 4 个。

---

# 第五部分：函数详解

## 5.1 签名 + 参数

### socket() —— 买电话机

```c
int socket(int domain, int type, int protocol);
```

| 参数 | 填什么 | 说明 |
|---|---|---|
| `domain` | `AF_INET` | IPv4。（`AF_INET6` 是 IPv6） |
| `type` | `SOCK_STREAM` | **TCP** |
| | `SOCK_DGRAM` | **UDP** |
| `protocol` | `0` | 自动选（TCP/UDP 不需要额外指定） |

**返回**：新的 fd；出错返回 `-1`

```c
int fd = socket(AF_INET, SOCK_STREAM, 0);   // TCP
int fd = socket(AF_INET, SOCK_DGRAM, 0);    // UDP（你在 netgame 里写的）
```

---

### bind() —— 申请电话号码

```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

| 参数 | 填什么 |
|---|---|
| `sockfd` | `socket()` 返回的 fd |
| `addr` | 填好的 `sockaddr_in`，**强转成 `(sockaddr*)`** |
| `addrlen` | `sizeof(sockaddr_in)` |

**返回**：0 成功；-1 失败

**为什么要强转？** `bind` 的接口设计成通用的（IPv4、IPv6、Unix socket 都能用），
你填具体类型 `sockaddr_in`，再转成通用类型传进去。**历史包袱，照抄就行。**

```c
sockaddr_in addr{};
addr.sin_family      = AF_INET;
addr.sin_addr.s_addr = INADDR_ANY;      // 绑本机所有网卡
addr.sin_port        = htons(7777);     // ⚠️ 必须转字节序！

bind(fd, (sockaddr*)&addr, sizeof(addr));
```

---

### listen() —— 开通「可接来电」

```c
int listen(int sockfd, int backlog);
```

| 参数 | 填什么 | 说明 |
|---|---|---|
| `sockfd` | bind 过的 fd | |
| `backlog` | 比如 `10` | **等位队列长度**：同时最多几个人排队等你接 |

**返回**：0 成功；-1 失败

> `backlog` 不是「最多连接数」，是「**还没来得及 accept 的**连接最多排几个」。
> 填 5~128 都行，随便填个正数，写游戏和练手用不着纠结这个。

**调用 `listen` 之后，这个 fd 就变成了「监听套接字」，只能用来 `accept`，不能收发数据。**

---

### accept() —— 铃响，接起来 ⭐

```c
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

| 参数 | 填什么 | 说明 |
|---|---|---|
| `sockfd` | **监听** fd（不是连接 fd） | |
| `addr` | `sockaddr_in` 的地址，用来**接收对方地址** | 不关心就填 `NULL` |
| `addrlen` | `&len`（**注意是传指针**） | 不关心就填 `NULL` |

**返回**：**新的 fd**（代表这次连接）；出错返回 `-1`

**⚠️⚠️ 最容易搞错的地方：**

1. **它返回一个新 fd**，不是传进去的那个
2. **它默认会阻塞** —— 没人连进来就卡在这儿不动
3. `addrlen` 是**指针**！因为它是输入输出参数：
   进去时是你给的缓冲区大小，出来时是实际写入的大小
4. 传 `NULL, NULL` 表示「我不关心对方是谁」，完全合法，练手时最常用

```c
// 最简写法：不关心对方是谁
int conn_fd = accept(listen_fd, NULL, NULL);
if (conn_fd < 0) { /* 出错 */ }
// 之后 send/recv 全用 conn_fd
```

---

### connect() —— 拨号

```c
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

| 参数 | 填什么 |
|---|---|
| `sockfd` | `socket()` 返回的 fd |
| `addr` | **对方**的地址（不是自己的！） |
| `addrlen` | `sizeof(sockaddr_in)` |

**返回**：0 成功；-1 失败

**这一步会自动完成 TCP 三次握手**，连不上就返回 -1（比如对方没开、端口不对）。
**客户端不需要 `bind`** —— 系统会自动分配一个临时端口。

```c
sockaddr_in server{};
server.sin_family = AF_INET;
server.sin_port   = htons(7777);
inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);   // ⚠️ 用这个转换 IP

connect(fd, (sockaddr*)&server, sizeof(server));
```

---

### send() / recv() —— 说 / 听

```c
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
```

| 参数 | 填什么 | 说明 |
|---|---|---|
| `sockfd` | **连接** fd（accept 返回的那个） | |
| `buf` | 数据缓冲区 | `send` 是 `const`，`recv` 不是 |
| `len` | 字节数 | |
| `flags` | `0` | 练手填 0 就行 |

**返回值**：实际读/写的**字节数**；出错返回 `-1`

**⚠️⚠️ 三个必背的坑：**

1. **`recv` 返回 `0` = 对方关闭了连接**（不是错误！）
   - `> 0` ：实际收到的字节数
   - `== 0`：对端正常关闭 → 你该 `close` 了
   - `< 0` ：出错

2. **`send` 不保证一次发完** —— 返回值可能小于 `len`，得循环发
   ```c
   size_t sent = 0;
   while (sent < len) {
       ssize_t n = send(fd, buf + sent, len - sent, 0);
       if (n <= 0) { /* 出错 */ break; }
       sent += n;
   }
   ```

3. **`recv` 也不保证一次收完你要的**，同理

---

### sendto() / recvfrom() —— UDP 专用

```c
ssize_t sendto(int fd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest, socklen_t addrlen);
ssize_t recvfrom(int fd, void *buf, size_t len, int flags,
                 struct sockaddr *src, socklen_t *addrlen);
```

多出来的 `dest` / `src` 就是**地址** —— 因为 UDP 没连接，每张明信片都得写地址。

你在 `~/netgame/netgame.cpp` 里已经用过了。

---

### close() —— 挂电话

```c
int close(int fd);
```

**⚠️ 别忘了关两个 fd：** `accept` 返回的 `conn_fd` **和** `listen_fd` 都要关。

---

## 5.2 sockaddr_in 结构体

```c
struct sockaddr_in {
    sa_family_t    sin_family;   // 地址族，填 AF_INET
    in_port_t      sin_port;     // 端口 —— ⚠️ 必须是网络字节序（htons）
    struct in_addr sin_addr;     // IP   —— ⚠️ 必须是网络字节序
    char           sin_zero[8];  // 填充字节，填 0（编译器会自动）
};
```

填法：

```c
sockaddr_in addr{};
addr.sin_family = AF_INET;
addr.sin_port   = htons(7777);            // 端口
addr.sin_addr.s_addr = INADDR_ANY;        // 任一本机网卡（服务器常用）
// 或者指定 IP：
// inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
```

| 值 | 含义 |
|---|---|
| `INADDR_ANY` | 本机所有网卡都收（服务器一般用这个） |
| `INADDR_LOOPBACK` | 127.0.0.1，只有本机能连（练手够用） |

---

## 5.3 字节序转换（htons 那些）

**为什么需要：** 网络规定统一用**大端序**，但你的机器可能是小端序（x86 都是）。
所以端口和 IP 送进结构体之前要转一下。

| 函数 | 全称 | 用途 |
|---|---|---|
| `htons(x)` | **h**ost **to** **n**etwork **s**hort | 端口（16 位） |
| `htonl(x)` | host to network long | 32 位整数 |
| `ntohs(x)` | **n**etwork **to** **h**ost **s**hort | 端口（收包时转回来） |
| `ntohl(x)` | network to host long | 32 位整数 |

**记忆：** `htons` = 主机转网络，短整型。`n` = network，`h` = host，`s` = short。

**⚠️ 忘了 `htons` 的后果：** 端口 7777 会变成 19969（字节颠倒），
你绑到一个莫名其妙的端口上，然后连不上、也看不出哪儿错了。

**IP 和端口的处理方式不同：**

| | 怎么处理 |
|---|---|
| 端口 | `htons(7777)` —— 手动转 |
| IP | `inet_pton()` —— 顺便就转了，不用手动 |

---

## 5.4 IP 地址转换：inet_pton / inet_ntop

```c
int inet_pton(int af, const char *src, void *dst);        // 字符串 → 二进制
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);  // 二进制 → 字符串
```

| 函数 | 方向 | 例子 |
|---|---|---|
| `inet_pton` | `"127.0.0.1"` → `struct in_addr` | 填地址时用 |
| `inet_ntop` | `struct in_addr` → `"127.0.0.1"` | 打印对方 IP 时用 |

**`pton` = presentation to network，`ntop` = network to presentation。**

```c
// 填地址
sockaddr_in addr{};
inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

// 打印对方 IP（服务器 accept 之后常这么干）
sockaddr_in peer{};
socklen_t len = sizeof(peer);
int conn = accept(listen_fd, (sockaddr*)&peer, &len);

char ip[64];
inet_ntop(AF_INET, &peer.sin_addr, ip, sizeof(ip));
printf("客户端来了: %s:%d\n", ip, ntohs(peer.sin_port));   // ⚠️ 端口要 ntohs
```

> 你在 `netgame.cpp` 的 `Poll()` 里已经写过 `inet_ntop` 打印对端地址了。

**`inet_pton` 和 `inet_aton` 的区别：** 老 API 是 `inet_aton`，新的是 `inet_pton`
（`p` = 兼容 IPv6）。**用新的 `inet_pton`。**

---

# 第六部分：完整流程骨架

## 6.1 TCP 服务器

```
socket()     创建套接字
  ↓
bind()       绑到 IP:端口
  ↓
listen()     变成监听套接字
  ↓
  ├─> accept()      ← 阻塞在这里等人来，返回【新 fd】
  │     ↓
  │   recv()/send() ← 用新 fd 收发
  │     ↓
  │   close(conn_fd)
  │
  └─> 循环回去继续 accept（可以服务很多客户端）
```

```c
int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

// 让端口能立刻重用（避免 "Address already in use"）
int opt = 1;
setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

sockaddr_in addr{};
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = INADDR_ANY;
addr.sin_port = htons(7777);
bind(listen_fd, (sockaddr*)&addr, sizeof(addr));

listen(listen_fd, 10);
printf("监听 7777 端口...\n");

while (1) {                                   // 一直等人来
    int conn_fd = accept(listen_fd, NULL, NULL);   // ← 阻塞
    if (conn_fd < 0) continue;

    char buf[1024];
    ssize_t n;
    while ((n = recv(conn_fd, buf, sizeof(buf), 0)) > 0) {
        send(conn_fd, buf, n, 0);             // 回显：原样发回去
    }
    // n == 0 表示对方关闭了，跳出循环

    close(conn_fd);                           // ⚠️ 别忘
}
```

> **注意**：上面这个版本一次只能服务一个客户端（`while` 里被占住了）。
> **多客户端并发**是下一个课题（`select`/`poll`/`epoll` 或 多线程/fork）。

## 6.2 TCP 客户端

```
socket()     创建套接字
  ↓
connect()    连到服务器（自动三次握手）
  ↓
send()/recv()  收发（不用带地址）
  ↓
close()
```

```c
int fd = socket(AF_INET, SOCK_STREAM, 0);

sockaddr_in server{};
server.sin_family = AF_INET;
server.sin_port = htons(7777);
inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

if (connect(fd, (sockaddr*)&server, sizeof(server)) < 0) {
    perror("connect");       // 打印出错原因
    return 1;
}

const char* msg = "hello\n";
send(fd, msg, strlen(msg), 0);

char buf[1024];
ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
if (n > 0) {
    buf[n] = '\0';
    printf("收到: %s", buf);
}
close(fd);
```

---

# 第七部分：错误处理

**所有 socket 函数出错都返回 `-1`，并且把原因写在全局变量 `errno` 里。**

```c
if (bind(fd, ...) < 0) {
    perror("bind");           // 自动打印 "bind: Address already in use"
    // 或者手动：
    // printf("bind 失败: %s\n", strerror(errno));
    return 1;
}
```

`perror()` 最方便 —— 传一个标签，它自动拼上 `errno` 的描述。

**需要 `#include <errno.h>` 和 `#include <cstring>`（用 strerror 时）。**

## 常见 errno

| errno | 意思 | 怎么办 |
|---|---|---|
| `EADDRINUSE` | 端口被占用 | 上次的进程没退干净；用 `SO_REUSEADDR` 或换端口 |
| `EAGAIN` / `EWOULDBLOCK` | 非阻塞模式下暂时没数据 | **不是错误**，跳出循环就行 |
| `ECONNRESET` | 对方强制断开了 | 关闭这个连接 |
| `EINTR` | 被信号打断 | 重试 |
| `EACCES` | 权限不足 | 1024 以下的端口要 root；换个大端口 |

---

# 第八部分：必背的坑（先看一遍）

| # | 坑 | 后果 |
|---|---|---|
| 1 | **`accept` 返回新 fd** | 用错的 fd 收发 → 收到奇怪的东西/没反应 |
| 2 | **`recv` 返回 0 = 对方关闭** | 当成错误处理 → 逻辑乱套 |
| 3 | **`send` 可能发不完** | 大数据只发出去一半 |
| 4 | **TCP 没有消息边界** | 发两次 "hello" "world"，对方一次收到 "helloworld" |
| 5 | **端口要 `htons`** | 绑到 19969 而不是 7777 |
| 6 | **两个 fd 都要 `close`** | 文件描述符泄漏，跑久了程序崩 |
| 7 | **`accept` 默认阻塞** | 没客户端时整个程序卡住 |
| 8 | **`sockaddr_in` 要清零** | `sockaddr_in addr{};` ← 别漏了 `{}` |
| 9 | **`IP` 用 `inet_pton`** | 直接赋值字符串给 `sin_addr` 编译不过 |
| 10 | **端口被占** | `bind` 报 `Address already in use` → 加 `SO_REUSEADDR` |

## 重点解释第 4 条（TCP 没有消息边界）

这是**新手最大的坑**，也是 TCP/UDP 最重要的区别之一。

```c
// 发送方
send(fd, "hello", 5, 0);
send(fd, "world", 5, 0);

// 接收方 —— 下面三种情况【都可能发生】！
recv(...) → "helloworld"   // 两次合成一次
recv(...) → "hello"        // 正常
recv(...) → "hel"          // 还被切开了
```

**原因**：TCP 是**字节流**，不是消息流。它只保证**字节按顺序到达**，
不保证「你发几次，对方就收几次」。

**UDP 没这个问题** —— 你发一个包，对方收一个包，边界天然存在。
（你在 `netgame.cpp` 里发的是固定大小的 `Packet` 结构体，天然对齐，所以没踩到这个坑。）

**解决方案**（下一个课题）：在数据前面加一个「长度头」，
或者用固定分隔符（比如 `\n`）。这个叫**应用层协议设计**。

---

# 第九部分：下一步练习

## 练习 1：TCP 回显服务器 ⭐ 现在做这个

**目标**：客户端发什么，服务器原样发回去。

**服务器 5 步**：`socket → bind → listen → accept → recv/send`
**客户端 4 步**：`socket → connect → send/recv`

**测试方式**：
```bash
# 服务器跑起来后
nc 127.0.0.1 7777       # 敲一行看有没有回显
# 或者自己写个客户端（更推荐，能同时练两边）
```

**预期会卡的地方**（对号入座）：
- `accept` 返回 -1 → 多半忘了 `listen`
- `bind` 报 `Address already in use` → 上次进程没退干净；加 `SO_REUSEADDR`
- 连上了但收不到 → **`recv` 用错了 fd**（用了 `listen_fd` 而不是 `conn_fd`）

## 练习 2：支持多客户端

现在这个版本一次只能服务一个客户端。下一个课题：
- 方案 A：`fork()` 一个连接一个进程（简单，适合理解）
- 方案 B：多线程
- 方案 C：`select` / `poll` / `epoll`（**后端的核心技术**）

## 练习 3：你真实的作业

自己写一个客户端 + 能让**两个客户端互相聊天**的服务器。

## 练习 4：用真知识重写 netgame

等你懂了 TCP 和并发，回头看 `~/netgame/netgame.cpp` 会清晰很多。
再考虑要不要加**服务器权威**（真正的多人游戏架构）。

---

# 附：头文件清单

```c
#include <sys/socket.h>     // socket, bind, listen, accept, connect, send, recv
#include <netinet/in.h>     // sockaddr_in, INADDR_ANY, htons
#include <arpa/inet.h>      // inet_pton, inet_ntop
#include <unistd.h>         // close, read, write
#include <fcntl.h>          // fcntl, O_NONBLOCK
#include <errno.h>          // errno
#include <cstring>          // strerror, memset
#include <cstdio>           // printf, perror
```

编译：`g++ -std=c++17 -Wall -o server server.cpp`（**不需要链 raylib**）

---

# 附：快速自测

学完这份笔记，你应该能不看答案回答：

1. `socket()` 返回的是什么？（提示：一个整数，什么编号空间？）
2. 为什么服务器有两个 fd？各自职责？
3. `recv` 返回 0 是什么意思？
4. 为什么端口要 `htons`？不做会怎样？
5. TCP 和 UDP 的 API 为什么长得不一样？（关键在哪个概念？）
6. 客户端的 `connect` 里填的是**谁**的地址？
7. 发两次 "hello" "world"，对方一次收到 "helloworld" 是 bug 吗？

**答不上来的回去翻对应章节。**
