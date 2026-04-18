#include "pch.h"

#include "Compressor.h"

#include "../src/compress.h"
#include "../src/checksum.h"
#include "uv.h"

#include <cinttypes>

#if !_WINDLL
#define sprintf_s(a, b, c, d, e) sprintf(a, b, c, d, e);
#include <cstring>
#endif

inline uv_buf_t uv_buf_init(const char* str) {
    return uv_buf_init(const_cast<char*>(str), static_cast<unsigned int>(strlen(str)));
}

inline uv_buf_t uv_buf_init(const std::string& str) {
    return uv_buf_init(const_cast<char*>(str.c_str()), static_cast<unsigned int>(str.size()));
}

namespace maxcsolib {

    void default_args(Arguments& args) {
        args.threads = 0;
        args.block_size = maxcso::DEFAULT_BLOCK_SIZE;

        args.flags_fmt = 0;
        args.flags_use = 0;
        args.flags_no = 0;
        args.flags_only = 0;
        args.flags_final = 0;

        args.orig_cost_percent = 0.0;
        args.lz4_cost_percent = 0.0;

        args.fast = false;
        args.smallest = false;
        args.quiet = false;
        args.crc = false;
        args.decompress = false;
        args.measure = false;
    }

#ifdef _WIN32
    int setenv(const char* name, const char* value, int) {
        return _putenv_s(name, value);
    }
#endif

    void update_threadpool(const Arguments& args) {
        char threadpool_size[32];
        sprintf(threadpool_size, "%d", args.threads);
        setenv("UV_THREADPOOL_SIZE", threadpool_size, 1);
    }

    void Compressor::Compress(Arguments args) {

        update_threadpool(args);

        uv_loop_t loop;
        uv_tty_t tty;
        uv_loop_init(&loop);
        uv_tty_init(&loop, &tty, 2, 0);
        uv_tty_set_mode(&tty, 0);
        bool formatting = uv_guess_handle(2) == UV_TTY && !args.quiet;

        int64_t next = uv_hrtime();
        //50ms
        static const int64_t interval_ns = 50000000LL;
        static const double b_to_mb = 1.0 / (1024 * 1024);
        static const double ns_to_s = 1.0 / (1000 * 1000 * 1000);
        struct History { int64_t pos, time; } history[20]; //50ms * 20 = 1s
        constexpr int historyLen = sizeof(history) / sizeof(*history);
        int historyPos = 0;
        std::fill(std::begin(history), std::end(history), History{ 0, next });

        std::string statusInfo;
        uv_write_t write_req;
        uv_buf_t bufs[2];

        maxcso::ProgressCallback progress = [&](const maxcso::Task* task, maxcso::TaskStatus status, int64_t pos, int64_t total, int64_t written) {
            if (!formatting) {
                //Don't show progress if piping output, but do show final result
                if (status != maxcso::TASK_SUCCESS || args.quiet) {
                    return;
                }
            }

            statusInfo.clear();

            if (status == maxcso::TASK_INPROGRESS) {
                int64_t now = uv_hrtime();
                if (now >= next) {
                    double percent = total == 0 ? 0.0 : (pos * 100.0) / total;
                    double ratio = pos == 0 ? 0.0 : (written * 100.0) / pos;
                    History& entry = history[historyPos];
                    int64_t diff = pos - entry.pos;
                    int64_t elapsed = now - entry.time;
                    entry = { pos, now };
                    double speed = elapsed == 0 ? 0 : (diff * b_to_mb) / (elapsed * ns_to_s);

                    char temp[128];
                    sprintf_s(temp, "%3.0f%%, ratio=%3.0f%%, speed=%5.2d MB/s", percent, ratio, speed);
                    statusInfo = temp;

                    next = now + interval_ns;
                    historyPos++;
                    if (historyPos >= historyLen)
                        historyPos = 0;
                }
            }
            else if (status == maxcso::TASK_SUCCESS) {
                double ratio = total == 0 ? 0.0 : (written * 100.0) / total;
                char temp[128];
                sprintf_s(temp, "%" PRId64 " -> %" PRId64 "bytes (%.0f%%)\n", total, written, ratio);
                statusInfo = temp;
            }
            else
                //This shouldn't happen
                statusInfo = "Something went wrong.\n";

            if (statusInfo.empty())
                return;

            if (task->input.size() > 38 && formatting)
                statusInfo = "..." + task->input.substr(task->input.size() - 35) + ": " + statusInfo;
            else
                statusInfo = task->input + ": " + statusInfo;

            unsigned int nbufs = 0;
            if (formatting) {
                bufs[nbufs++] = uv_buf_init(ANSI_RESET_LINE);
                bufs[nbufs++] = uv_buf_init(statusInfo);
                uv_write(&write_req, reinterpret_cast<uv_stream_t*>(&tty), bufs, nbufs, nullptr);
            }
            else
                fprintf(stderr, "%s", statusInfo.c_str());
            };

        maxcso::ErrorCallback error = [&](const maxcso::Task* task, maxcso::TaskStatus status, const char* reason) {
            //Change result to indicate failure
            if (status != maxcso::TASK_SUCCESS)
                return; //TODO: Throw exception

            if (args.quiet)
                return;

            const std::string prefix = status == maxcso::TASK_SUCCESS ? "" : "Error while processing";
            statusInfo = (formatting ? ANSI_RESET_LINE : "") + prefix + task->input + ": " + reason + "\n";
            if (formatting) {
                bufs[0] = uv_buf_init(statusInfo);
                uv_write(&write_req, reinterpret_cast<uv_stream_t*>(&tty), bufs, 1, nullptr);
            }
            else
                fprintf(stderr, "%s", statusInfo.c_str());

            };

        std::vector<maxcso::Task> tasks;
        for (size_t i = 0; i < args.inputs.size(); ++i) {
            maxcso::Task task;
            task.input = args.inputs[i];
            if (!args.crc && !args.measure)
                task.output = args.outputs[i];
            task.progress = progress;
            task.error = error;
            task.block_size = args.block_size;
            task.flags = args.flags_final;
            task.orig_max_cost_percent = args.orig_cost_percent;
            task.lz4_max_cost_percent = args.lz4_cost_percent;
            tasks.push_back(std::move(task));
        }

        if (args.crc)
            maxcso::Checksum(tasks);
        else
            maxcso::Compress(tasks);

        uv_tty_reset_mode();
        uv_loop_close(&loop);
    }

    Compressor::Compressor() {}
    Compressor::~Compressor() {}
}