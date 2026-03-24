#ifndef UT_SPAWN_HPP
#define UT_SPAWN_HPP


#include <algorithm>
#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iterator>
#include <optional>
#include <ranges>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#if defined(__unix__)
#    include <fcntl.h>
#    include <sys/types.h>
#    include <sys/wait.h>
#    include <unistd.h>
#else
#    errorr "ut::spawn not supported outside of POSIX"
#endif


#if __cplusplus < 202'002L
#    error this file has to be compiled with at least C++20
#endif


namespace ut {

struct Environ {
private:
    std::vector<std::string> env;
public:
    Environ() = default;

    template<typename P, typename... PRest>
    explicit Environ(P &&p, PRest &&...rest) {
        (*this)[p.first] = p.second;
        (void)(((*this)[rest.first] = rest.second), ...);
    }

    static Environ current() {
        extern char **environ;
        Environ out;
        for (auto it = environ; *it; ++it)
            out.env.push_back(*it);
        return out;
    }

    std::vector<char *> toPtrVec() {
        std::vector<char *> out;
        out.reserve(env.size() + 1);
        std::transform(env.begin(), env.end(), std::back_inserter(out), [](std::string &s) noexcept { return s.data(); });
        out.push_back(nullptr);
        return out;
    }

    template<bool is_const>
    struct EnvironProxy {
        friend Environ;
    private:
        using Parent = std::conditional_t<is_const, Environ const *, Environ *>;

        EnvironProxy(Parent p, std::string_view k) noexcept
                : parent(p)
                , key(k) { }

        Parent parent;
        std::string_view key;

        std::pair<std::vector<std::string>::iterator, std::string::size_type> find() const noexcept {
            auto actual_split = std::string::npos;
            return {std::find_if(parent->env.begin(),
                        parent->env.end(),
                        [&](std::string const &entry) noexcept {
                auto split = entry.find('=');
                assert(split != entry.npos);
                if (std::string_view {entry}.substr(0, split) == key) {
                    actual_split = split;
                    return true;
                } else
                    return false;
            }),
                actual_split};
        }

    public:
        void operator=(std::string_view value) requires(!is_const) {
            auto [pos, split] = find();
            if (pos != parent->env.end())
                *pos = std::format("{}={}", key, value);
            else
                parent->env.push_back(std::format("{}={}", key, value));
        }

        operator char const *() const noexcept {
            auto [pos, split] = find();
            if (pos != parent->env.end()) return nullptr;
            return pos->c_str() + split + 1;
        }
    };

    EnvironProxy<false> operator[](std::string_view key) noexcept {
        return {this, key};
    }

    EnvironProxy<true> operator[](std::string_view key) const noexcept {
        return {this, key};
    }
};

enum struct OutPipe { Ignore, DevNull, String };
enum struct InPipe { Ignore = int(OutPipe::Ignore), DevNull = int(OutPipe::DevNull) };

struct SpawnConfig {
    std::optional<Environ> env = std::nullopt;
    std::variant<InPipe, std::string> stdin = InPipe::Ignore;
    OutPipe stdout = OutPipe::Ignore;
    OutPipe stderr = OutPipe::Ignore;
    bool path_lookup = true;
};

inline bool commandExists(std::string_view cmd) {
    namespace fs = std::filesystem;
    using enum fs::perms;
    auto const path = std::getenv("PATH");
    if (!path) return false;
    auto cmdpath = fs::path {cmd};
    if (cmdpath.has_parent_path())
        return fs::is_regular_file(cmdpath) && (fs::status(cmdpath).permissions() & owner_exec) == owner_exec;
    for (auto const &component : std::views::split(std::string_view {path}, ':')) {
        if (component.empty()) continue;
        [[maybe_unused]]
        std::error_code ec;  // never checked, silently skips non-existent directories
        for (auto const &file : fs::directory_iterator(std::string_view(component.begin(), component.end()),
                 fs::directory_options::skip_permission_denied,
                 ec)) {
            if (!file.is_regular_file()) continue;
            if ((fs::status(file).permissions() & owner_exec) != owner_exec) continue;
            if (file.path().filename() == cmdpath) return true;
        }
    }
    return false;
}

enum struct StopKind { Exit, Signal, Unknown };
enum struct Sig {
    // Standard
    Int = SIGINT,    // Interactive attention signal.
    Ill = SIGILL,    // Illegal instruction.
    Abrt = SIGABRT,  // Abnormal termination.
    Fpe = SIGFPE,    // Erroneous arithmetic operation.
    Segv = SIGSEGV,  // Invalid access to storage.
    Term = SIGTERM,  // Termination request.

    // POSIX
    Hup = SIGHUP,    // Hangup.
    Quit = SIGQUIT,  // Quit.
    Trap = SIGTRAP,  // Trace/breakpoint trap.
    Kill = SIGKILL,  // Killed.
    Pipe = SIGPIPE,  // Broken pipe.
    Alrm = SIGALRM,  // Alarm clock.
};

struct StoppedProcess {
    friend struct RunningProcess;
private:
    StoppedProcess() = default;
    StopKind m_kind;
    int m_code;
    std::optional<std::string> m_stdout;
    std::optional<std::string> m_stderr;
public:
    [[nodiscard]]
    StopKind kind() const noexcept {
        return m_kind;
    }

    [[nodiscard]]
    int code() const noexcept {
        return m_code;
    }

    [[nodiscard]]
    std::optional<std::string> const &stdout() const noexcept {
        return m_stdout;
    }

    [[nodiscard]]
    std::optional<std::string> const &stderr() const noexcept {
        return m_stderr;
    }
};

struct [[nodiscard]] RunningProcess {

    friend RunningProcess spawn(char **args, SpawnConfig const &conf);

    friend RunningProcess shell(std::string_view cmd, SpawnConfig const &cfg);
private:
    static constexpr auto read_idx = 0;
    static constexpr auto write_idx = 1;

    pid_t m_pid = -1;
    std::optional<std::string> m_error;
    int m_stdout_pipe = -1;
    int m_stderr_pipe = -1;

    RunningProcess() = default;

    static RunningProcess fromError(std::string e) {
        RunningProcess out;
        out.m_error.emplace(std::move(e));
        return out;
    }

    static RunningProcess fromPidAndPipes(pid_t pid, int stdout, int stderr) {
        RunningProcess out;
        out.m_pid = pid;
        out.m_stdout_pipe = stdout;
        out.m_stderr_pipe = stderr;
        return out;
    }

    static int devNull() noexcept {
        return open("/dev/null", O_RDWR | O_CLOEXEC);
    }

    static int setupPipe(int (&pipes)[2], OutPipe how, int direction) noexcept {
        switch (how) {
            case OutPipe::Ignore: return 0;
            case OutPipe::DevNull: pipes[direction] = devNull(); return pipes[direction];
            case OutPipe::String:
                if (pipe(pipes)) return -1;
                return pipes[direction];
            default:;
        }
        return -1;
    }

    static int setupPipe(int (&pipes)[2], OutPipe how) noexcept {
        return setupPipe(pipes, how, write_idx);
    }

    static int setupPipe(int (&pipes)[2], std::variant<InPipe, std::string> const &how) noexcept {
        if (auto in_pipe = std::get_if<InPipe>(&how))
            return setupPipe(pipes, OutPipe(int(*in_pipe)), read_idx);
        else
            return setupPipe(pipes, OutPipe::String, read_idx);
    }

    static int dupIf(int fd, int fd2) noexcept {
        if (fd >= 0 && fd2 >= 0) return dup2(fd, fd2);
        return 0;
    }

    static void closeIf(int fd) noexcept {
        if (fd >= 0) close(fd);
    }

    static int writeToFd(int fd, std::variant<InPipe, std::string> const &v) noexcept {
        auto s = std::get_if<std::string>(&v);
        if (!s) return 0;
        std::string::const_pointer ptr = s->data();
        std::string::size_type remaining = s->size();
        ssize_t written = 0;
        do {
            errno = 0;
            written = write(fd, ptr, remaining);
            if (written < 0) {
                if (errno == EINTR) continue;
                return -1;
            }
            remaining -= static_cast<std::string_view::size_type>(written);
            ptr += static_cast<size_t>(written);
        } while (written);
        if (remaining) return -2;
        return 0;
    }

    static int readFromFd(int fd, std::optional<std::string> &str) {
        if (fd < 0) return 0;
        str.emplace();
        char buf[BUFSIZ];
        ssize_t bytes_read = 0;
        do {
            bytes_read = read(fd, buf, sizeof(buf));
            if (bytes_read < 0) {
                if (errno == EINTR) continue;
                return -1;
            }
            str->append(std::string_view {buf, static_cast<std::size_t>(bytes_read)});
        } while (bytes_read);
        return 0;
    }

    template<typename Fn>
    static RunningProcess spawnWith(Fn &&fn, [[maybe_unused]] SpawnConfig const &conf) {
        int stdout_fds[2] = {-1, -1};
        int stderr_fds[2] = {-1, -1};
        int stdin_fds[2] = {-1, -1};
        if (setupPipe(stdin_fds, conf.stdin) < 0)
            return RunningProcess::fromError(std::string("Failed to open stdin pipe: ") + std::strerror(errno));
        if (setupPipe(stdout_fds, conf.stdout) < 0)
            return RunningProcess::fromError(std::string("Failed to open stdout pipe: ") + std::strerror(errno));
        if (setupPipe(stderr_fds, conf.stderr) < 0)
            return RunningProcess::fromError(std::string("Failed to open stderr pipe: ") + std::strerror(errno));

        switch (auto pid = fork()) {
            case 0:
                closeIf(stdout_fds[read_idx]);
                closeIf(stderr_fds[read_idx]);
                closeIf(stdin_fds[write_idx]);
                dupIf(stdout_fds[write_idx], STDOUT_FILENO);
                dupIf(stderr_fds[write_idx], STDERR_FILENO);
                dupIf(stdin_fds[read_idx], STDIN_FILENO);
                std::invoke(std::forward<Fn>(fn));
                std::fputs("Failed to exec", stderr);
                _exit(127);
            case -1:
                return RunningProcess::fromError(std::string("Failed to start the process: ") + std::strerror(errno));
            default:
                closeIf(stdout_fds[write_idx]);
                closeIf(stderr_fds[write_idx]);
                closeIf(stdin_fds[read_idx]);
                switch (writeToFd(stdin_fds[write_idx], conf.stdin)) {
                    case -1:
                        return RunningProcess::fromError(
                            std::string("Failed to write string to pipe: ") + std::strerror(errno));
                    case -2: return RunningProcess::fromError(std::string("Failed to write all bytes to pipe"));
                    default:;
                }
                closeIf(stdin_fds[write_idx]);
                return RunningProcess::fromPidAndPipes(pid, stdout_fds[read_idx], stderr_fds[read_idx]);
        }
    }
public:

    RunningProcess(RunningProcess const &) = delete;
    RunningProcess &operator=(RunningProcess const &) = delete;

    RunningProcess(RunningProcess &&other) {
        *this = std::move(other);
    }

    RunningProcess &operator=(RunningProcess &&other) {
        if (this != &other) {
            (void)wait();
            m_pid = std::exchange(other.m_pid, -1);
            m_stdout_pipe = std::exchange(m_stdout_pipe, -1);
            m_stderr_pipe = std::exchange(m_stderr_pipe, -1);
            m_error = std::move(other.m_error);
        }
        return *this;
    }

    ~RunningProcess() {
        (void)wait();
    }

    [[nodiscard]]
    std::optional<std::string> const &error() const noexcept {
        return m_error;
    }

    [[nodiscard]]
    operator bool() const noexcept {
        return !(m_pid == -1 || m_error.has_value());
    }

    std::optional<StoppedProcess> kill(Sig sig) {
        if (!*this) return std::nullopt;
        ::kill(m_pid, int(sig));
        return wait();
    }

    std::optional<StoppedProcess> wait() {
        if (!*this) return std::nullopt;

        int status;
        pid_t res = -1;
        errno = 0;
        while ((res = waitpid(m_pid, &status, 0)) < 0 && errno == EINTR) { }
        if (res < 0) {
            closeIf(m_stdout_pipe);
            closeIf(m_stderr_pipe);
            m_pid = -1;
            return std::nullopt;
        }
        StoppedProcess out;
        if (WIFEXITED(status)) {
            out.m_code = WEXITSTATUS(status);
            out.m_kind = StopKind::Exit;
        } else if (WIFSIGNALED(status)) {
            out.m_code = WTERMSIG(status);
            out.m_kind = StopKind::Signal;
        } else {
            out.m_code = status;
            out.m_kind = StopKind::Unknown;
        }
        readFromFd(m_stdout_pipe, out.m_stdout);
        readFromFd(m_stderr_pipe, out.m_stderr);

        closeIf(m_stdout_pipe);
        closeIf(m_stderr_pipe);
        m_pid = -1;
        return out;
    }
};

template<typename T>
concept SpawnArg = false;

namespace detail {

    template<typename Arg, typename D = std::decay_t<Arg>>
    inline constexpr bool is_spawn_arg_v = std::is_same_v<D, char *>            //
                                        || std::is_same_v<D, char const *>      //
                                        || std::is_same_v<D, std::string>       //
                                        || std::is_same_v<D, std::string_view>  //
                                        || std::is_integral_v<D>                //
                                        || std::is_floating_point_v<D>;

    template<typename... Args>
    consteval bool validateSpawnArgs() noexcept {
        static_assert(sizeof...(Args), "spawn requires an argument");
        static_assert(!(sizeof...(Args) == 1 && std::conjunction_v<std::is_same<std::decay_t<Args>, SpawnConfig>...>),
            "SpawnConfig cannot be the only argument to spawn");
        if (sizeof...(Args) == 0) return false;
        return []<std::size_t... i>(std::index_sequence<i...>) noexcept {
            return ([]() noexcept {
                if (is_spawn_arg_v<Args>) return true;
                if (i && i + 1 == sizeof...(i) && std::is_same_v<std::decay_t<Args>, SpawnConfig>) return true;
                return false;
            }() && ...);
        }(std::make_index_sequence<sizeof...(Args)>());
    }

    template<typename T>
    std::string spawnArgTostring(T &&t) requires(is_spawn_arg_v<T>) {
        using D = std::decay_t<T>;

        if constexpr (std::is_integral_v<D> || std::is_floating_point_v<D> || std::is_same_v<D, std::string_view>
                      || std::is_same_v<D, char *> || std::is_same_v<D, char const *>)
            return std::format("{}", t);
        else
            return std::forward<T>(t);
    }

}  // namespace detail

inline RunningProcess spawn(char **args, SpawnConfig const &conf = {}) {
    if (conf.env) {
        auto const fn = [&]() noexcept {
            if (conf.path_lookup)
                return execvpe;
            else
                return execve;
        }();
        return RunningProcess::spawnWith([&]() {
            auto env_cpy = *conf.env;
            auto env_ptr = env_cpy.toPtrVec();
            fn(args[0], args, env_ptr.data());
        }, conf);
    } else {
        auto const fn = [&]() noexcept {
            if (conf.path_lookup)
                return execvp;
            else
                return execv;
        }();
        return RunningProcess::spawnWith([&]() noexcept { fn(args[0], args); }, conf);
    }
}

inline RunningProcess spawn(std::vector<char *> args, SpawnConfig const &conf = {}) {
    args.push_back(nullptr);
    return spawn(args.data(), conf);
}

inline RunningProcess spawn(std::vector<std::string> args, SpawnConfig const &conf = {}) {
    std::vector<char *> v;
    v.reserve(args.size());
    std::transform(args.begin(), args.end(), std::back_inserter(v), [](std::string &s) noexcept { return s.data(); });
    return spawn(std::move(v), conf);
}

template<typename... Args>
RunningProcess spawn(std::tuple<Args...> args, SpawnConfig const &conf = {}) {
    std::vector<std::string> v;
    v.reserve(sizeof...(Args));
    [&]<std::size_t... i>(std::index_sequence<i...>) {
        (v.push_back(detail::spawnArgTostring(std::forward<Args>(std::get<i>(args)))), ...);
    }(std::make_index_sequence<sizeof...(Args)>());

    return spawn(std::move(v), conf);
}

namespace detail {
    // This is a helper to separate out the config from the args
    template<typename... Args>
    RunningProcess spawnTupleImpl(std::tuple<Args...> &&args) requires(detail::validateSpawnArgs<Args...>()) {
        static constexpr auto is_last_config =
            std::is_same_v<std::decay_t<std::tuple_element_t<sizeof...(Args) - 1, std::tuple<Args...>>>, SpawnConfig>;

        SpawnConfig cfg = [&]() noexcept((std::is_move_constructible_v<Args> &&...)) {
            if constexpr (is_last_config)
                return std::move(std::get<sizeof...(Args) - 1>(args));
            else
                return SpawnConfig {};
        }();

        return spawn([&]<std::size_t... i>(std::index_sequence<i...>) {
            return std::tuple {std::forward<std::tuple_element_t<i, std::tuple<Args...>>>(std::get<i>(args))...};
        }(std::make_index_sequence<sizeof...(Args) - (is_last_config ? 1 : 0)>()), cfg);
    }
}  // namespace detail

template<typename... Args>
RunningProcess spawn(Args &&...args) requires(detail::validateSpawnArgs<Args...>()) {
    return detail::spawnTupleImpl(std::forward_as_tuple(args...));
}

inline RunningProcess shell(std::string_view cmd, SpawnConfig const &cfg = {}) {
    char const *shell = std::getenv("SHELL");
    if (!shell) shell = "sh";
    if (!commandExists(shell)) return RunningProcess::fromError(std::format("Command {} is not executable", shell));
    return spawn(shell, "-c", cmd, cfg);
}


}  // namespace ut

// namespace ut

#endif  // UT_SPAWN_HPP
