#pragma once

// to use with https://ui.perfetto.dev/ or the legacy chrome://tracing/
// from https://gist.github.com/enzo418/e0d3ab1b3cd7e0dcd92de4b51db39c51

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include "nldb/LOG/log.hpp"

namespace nldb {

    using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

    struct ProfileResult {
        std::string Name;

        FloatingPointMicroseconds Start;
        std::chrono::microseconds ElapsedTime;
        std::thread::id ThreadID;
    };

    struct InstrumentationSession {
        std::string Name;
    };

    class Instrumentor {
       public:
        Instrumentor(const Instrumentor&) = delete;
        Instrumentor(Instrumentor&&) = delete;

        void BeginSession(const std::string& name,
                          const std::string& filepath = "results.json") {
            std::lock_guard lock(m_Mutex);
            if (m_CurrentSession) {
                if (LogManager::GetLogger()) {
                    NLDB_ERROR(
                        "Instrumentor::BeginSession('{0}') when session '{1}' "
                        "already open.",
                        name, m_CurrentSession->Name);
                }
                InternalEndSession();
            }
            m_OutputStream.open(filepath);

            if (m_OutputStream.is_open()) {
                m_CurrentSession = new InstrumentationSession({name});
                WriteHeader();
            } else if (LogManager::GetLogger()) {
                NLDB_ERROR("Instrumentor could not open results file '{0}'.",
                           filepath);
            }
        }

        void EndSession() {
            std::lock_guard lock(m_Mutex);
            InternalEndSession();
        }

        void WriteProfile(const ProfileResult& result) {
            std::lock_guard lock(m_Mutex);
            m_Results.push_back(result);
        }

        static Instrumentor& Get() {
            static Instrumentor instance;
            return instance;
        }

       private:
        Instrumentor() : m_CurrentSession(nullptr) {}

        ~Instrumentor() { EndSession(); }

        void WriteHeader() {
            m_OutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
            m_OutputStream.flush();
        }

        void WriteFooter() {
            m_OutputStream << "]}";
            m_OutputStream.flush();
        }

        void InternalEndSession() {
            if (m_CurrentSession) {
                for (auto& result : m_Results) {
                    m_OutputStream << std::setprecision(3) << std::fixed;
                    m_OutputStream << ",{";
                    m_OutputStream << "\"cat\":\"function\",";
                    m_OutputStream << "\"dur\":" << (result.ElapsedTime.count())
                                   << ',';
                    m_OutputStream << "\"name\":\"" << result.Name << "\",";
                    m_OutputStream << "\"ph\":\"X\",";
                    m_OutputStream << "\"pid\":0,";
                    m_OutputStream << "\"tid\":" << result.ThreadID << ",";
                    m_OutputStream << "\"ts\":" << result.Start.count();
                    m_OutputStream << "}";
                }

                WriteFooter();
                m_OutputStream.close();
                delete m_CurrentSession;
                m_CurrentSession = nullptr;
                m_Results.clear();
            }
        }

       private:
        std::mutex m_Mutex;
        InstrumentationSession* m_CurrentSession;
        std::ofstream m_OutputStream;
        std::vector<ProfileResult> m_Results;
    };

    class InstrumentationTimer {
       public:
        InstrumentationTimer(const char* name)
            : m_Name(name), m_Stopped(false) {
            m_StartTimepoint = std::chrono::steady_clock::now();
        }

        ~InstrumentationTimer() {
            if (!m_Stopped) Stop();
        }

        void Stop() {
            auto endTimepoint = std::chrono::steady_clock::now();
            auto highResStart =
                FloatingPointMicroseconds {m_StartTimepoint.time_since_epoch()};
            auto elapsedTime =
                std::chrono::time_point_cast<std::chrono::microseconds>(
                    endTimepoint)
                    .time_since_epoch() -
                std::chrono::time_point_cast<std::chrono::microseconds>(
                    m_StartTimepoint)
                    .time_since_epoch();

            Instrumentor::Get().WriteProfile({m_Name, highResStart, elapsedTime,
                                              std::this_thread::get_id()});

            m_Stopped = true;
        }

       private:
        const char* m_Name;
        std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
        bool m_Stopped;
    };

    namespace InstrumentorUtils {

        template <size_t N>
        struct ChangeResult {
            char Data[N];
        };

        template <size_t N, size_t K>
        constexpr auto CleanupOutputString(const char (&expr)[N],
                                           const char (&remove)[K]) {
            ChangeResult<N> result = {};

            size_t srcIndex = 0;
            size_t dstIndex = 0;
            while (srcIndex < N) {
                size_t matchIndex = 0;
                while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 &&
                       expr[srcIndex + matchIndex] == remove[matchIndex])
                    matchIndex++;
                if (matchIndex == K - 1) srcIndex += matchIndex;
                result.Data[dstIndex++] =
                    expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
                srcIndex++;
            }
            return result;
        }
    }  // namespace InstrumentorUtils
}  // namespace nldb

#if NLDB_PROFILE
   // Resolve which function signature macro will be used. Note that this only
// is resolved when the (pre)compiler starts, so the syntax highlighting
// could mark the wrong one in your editor!
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || \
    (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define NLDB_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define NLDB_FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
#define NLDB_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || \
    (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define NLDB_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define NLDB_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define NLDB_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define NLDB_FUNC_SIG __func__
#else
#define NLDB_FUNC_SIG "NLDB_FUNC_SIG unknown!"
#endif

#define NLDB_PROFILE_BEGIN_SESSION(name, filepath) \
    ::nldb::Instrumentor::Get().BeginSession(name, filepath)
#define NLDB_PROFILE_END_SESSION() ::nldb::Instrumentor::Get().EndSession()
#define NLDB_PROFILE_SCOPE_LINE2(name, line)                              \
    constexpr auto fixedName##line =                                      \
        ::nldb::InstrumentorUtils::CleanupOutputString(name, "__cdecl "); \
    ::nldb::InstrumentationTimer timer##line(fixedName##line.Data)
#define NLDB_PROFILE_SCOPE_LINE(name, line) NLDB_PROFILE_SCOPE_LINE2(name, line)
#define NLDB_PROFILE_SCOPE(name) NLDB_PROFILE_SCOPE_LINE(name, __LINE__)
#define NLDB_PROFILE_FUNCTION() NLDB_PROFILE_SCOPE(NLDB_FUNC_SIG)
#define NLDB_PROFILE_CALL__(name, line, ...)                              \
    constexpr auto fixedName##line =                                      \
        ::nldb::InstrumentorUtils::CleanupOutputString(name, "__cdecl "); \
    ::nldb::InstrumentationTimer timer##line(fixedName##line.Data);       \
    __VA_ARGS__;                                                          \
    timer##line.Stop();
#define NLDB_PROFILE_CALL(name, ...) \
    NLDB_PROFILE_CALL__(name, __LINE__, __VA_ARGS__);

#else
#define NLDB_PROFILE_BEGIN_SESSION(name, filepath)
#define NLDB_PROFILE_END_SESSION()
#define NLDB_PROFILE_SCOPE(name)
#define NLDB_PROFILE_FUNCTION()
#define NLDB_PROFILE_CALL(...) __VA_ARGS__
#endif