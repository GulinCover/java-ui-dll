#pragma once

#include "IMCommon.hpp"

namespace MWD::Log {
    /******************************************************
    日志事件
    ******************************************************/
    class IMLogEvent {
    public:
        IMLogEvent(const MChar* file, MInt line, MUInt elspase, MUInt threadId, MUInt time, MString content,
                   MString logName)
                : m_file(file), m_line(line), m_elapse(elspase), m_threadId(threadId), m_time(time),
                  m_content(std::move(content)),
                  m_logName(std::move(logName)) {
        }

    public:
        [[nodiscard]] const MChar* GetFile() const { return m_file; }

        [[nodiscard]] MInt GetLine() const { return m_line; }

        [[nodiscard]] MUInt GetElapse() const { return time(nullptr) - GetTime(); }

        [[nodiscard]] MUInt GetThreadId() const { return m_threadId; }

        [[nodiscard]] MUInt GetTime() const { return m_time; }

        [[nodiscard]] MString GetContent() const { return m_content; }

        [[nodiscard]] MString GetLogName() { return m_logName; }

    private:
        const MChar* m_file;
        MInt m_line = 0;
        MUInt m_elapse = 0;
        MUInt m_threadId = 0;
        MUInt m_time;
        MString m_content;
        MString m_logName;
    };

    /******************************************************
    日志级别
    ******************************************************/
    class IMLogLevel {
    public:
        enum Level {
            INFO = 1,
#undef DEBUG
            DEBUG = 2,
            WARN = 3,
#undef ERROR
            ERROR = 4,
            FATAL = 5
        };

    public:
        static const MChar* ToString(IMLogLevel::Level level) {
            switch (level) {
#define XX(NAME)      \
    case NAME:        \
        return #NAME; \
        break;

                XX(INFO);
                XX(DEBUG);
                XX(WARN);
                XX(ERROR);
                XX(FATAL);
#undef XX
                default:
                    return "UNKNOWN";
                    break;
            }
        }
    };

    /********************************************************
     格式执行器
    *********************************************************/
    class IMFormatItem {
    public:
        explicit IMFormatItem(MString symbol) : m_symbol(std::move(symbol)) {
        }

        virtual ~IMFormatItem() = default;

    public:
        virtual MString Format(MString str,
                               IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) = 0;

        virtual MBoolean IsSpecialType() { return false; }

        virtual void ExecSpecialMethod(MString args[]) {}

        virtual MInt Length() {
            return m_symbol.length();
        }

        virtual MBoolean IsRepeat() { return false; }

    protected:
        MString m_symbol;
    };

    /******************************************************
     格式执行器Impl
    ******************************************************/
    // message
    class MMessageFormatItem : public IMFormatItem {
    public:
        explicit MMessageFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString Format(MString str,
                       IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) override {
            return event->GetContent();
        }
    };

    // level
    class MLevelFormatItem : public IMFormatItem {
    public:
        explicit MLevelFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString Format(MString str,
                       IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) override {
            return IMLogLevel::ToString(level);
        }
    };

    // elapse
    class MElapseFormatItem : public IMFormatItem {
    public:
        explicit MElapseFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString Format(MString str,
                       IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) override {
            MChar buf[16];
            sprintf(buf, "%d", event->GetElapse());
            return buf;
        }
    };

    // name
    class MNameFormatItem : public IMFormatItem {
    public:
        explicit MNameFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString Format(MString str,
                       IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) override {
            return event->GetLogName();
        }
    };

    // thread id
    class MThreadFormatItem : public IMFormatItem {
    public:
        explicit MThreadFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString Format(MString str,
                       IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) override {
            MChar buf[16];
            sprintf(buf, "%d", event->GetThreadId());
            return buf;
        }
    };

    // datetime
    class MDatetimeFormatItem : public IMFormatItem {
    public:
        explicit MDatetimeFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {
            m_length = std::strlen(symbol.c_str());
        }

    public:
        MString Format(MString str,
                       IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) override {
            std::time_t now = time(nullptr);
            std::tm tm{};
            localtime_s(&tm, &now);
            MChar buf[64];

            MString newSymbol = m_symbol + '{';
            MInt argsStartPos = str.find(newSymbol);
            if (argsStartPos >= 0) {
                MInt argsEndPos = str.find('}', argsStartPos);

                if (argsEndPos > argsStartPos) {
                    m_format = str.substr(argsStartPos + m_symbol.length() + 1, argsEndPos - argsStartPos - m_symbol.length() - 1);

                    MInt pos = m_format.find('$');
                    while (pos >= 0) {
                        m_format = m_format.replace(pos , 1, "%");
                        pos = m_format.find('$');
                    }

                    m_length = argsEndPos - argsStartPos + 1;
                }
            }

            if (m_format.empty()) {
                m_format = "%Y/%m/%d %H:%M:%S";
            }

            std::strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            return buf;
        }

        MInt Length() override { return m_length == 0 ? m_symbol.length() : m_length; }

    private:
        MString m_format;
        MInt m_length;
    };

    // filename
    class MFilenameFormatItem : public IMFormatItem {
    public:
        explicit MFilenameFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString Format(MString str,
                       IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) override {
            return event->GetFile();
        }
    };

    // line number
    class MLineFormatItem : public IMFormatItem {
    public:
        explicit MLineFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString Format(MString str,
                       IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) override {
            MChar buf[64];
            sprintf(buf, "%d", event->GetLine());
            return buf;
        }
    };

    // new line
    class MNewLineFormatItem : public IMFormatItem {
    public:
        explicit MNewLineFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString Format(MString str,
                       IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) override {
            return "\n";
        }
    };

    // tab
    class MTabFormatItem : public IMFormatItem {
    public:
        explicit MTabFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString Format(MString str,
                       IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) override {
            return "\t";
        }

        MBoolean IsRepeat() override {
            return true;
        }
    };

    /******************************************************
    日志格式
    ******************************************************/
    class IMLogFormatter {
    public:
        explicit IMLogFormatter(MString pattern) : m_pattern(std::move(pattern)) {
            m_items = std::make_shared<Map<MString, std::shared_ptr<IMFormatItem>>>();
            m_interceptorItems = std::make_shared<Vector<MString>>();
        }

    public:
        void Init() {
// %m -- message
// %p -- level
// %r -- elapse time
// %c -- log name
// %t -- thread id
// %n -- new line
// %d -- datetime
// %f -- filename
// %l -- line number
// %T -- tab
// {} -- args
#define ADD_KEY_VALUE(KEY, CLASS) \
    m_items->insert(std::pair<MString, std::shared_ptr<IMFormatItem>>(KEY, std::shared_ptr<CLASS>(new CLASS(KEY))));

            if (m_items) {
                ADD_KEY_VALUE("%m", MMessageFormatItem)
                ADD_KEY_VALUE("%p", MLevelFormatItem)
                ADD_KEY_VALUE("%r", MElapseFormatItem)
                ADD_KEY_VALUE("%c", MNameFormatItem)
                ADD_KEY_VALUE("%t", MThreadFormatItem)
                ADD_KEY_VALUE("%n", MNewLineFormatItem)
                ADD_KEY_VALUE("%d", MDatetimeFormatItem)
                ADD_KEY_VALUE("%f", MFilenameFormatItem)
                ADD_KEY_VALUE("%l", MLineFormatItem)
                ADD_KEY_VALUE("%T", MTabFormatItem)
            }

#undef ADD_KEY_VALUE
        }

        MString Format(IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) {
            MString str = m_pattern;
            auto i = m_items->begin();
            for (; i != m_items->end(); ++i) {
                MBoolean flag = true;

                for (auto& j : *m_interceptorItems) {
                    if (i->first == j) {
                        flag = false;
                        break;
                    }
                }

                if (flag) {
                    MString ret = i->second->Format(str, level, event);
                    MInt pos = str.find(i->first);
                    MInt len = i->second->Length();
                    if (pos >= 0 && len > 0) {
                        str = str.replace(pos, len, ret);
                    }

                    while (i->second->IsRepeat()) {
                        MString ret2 = i->second->Format(str, level, event);
                        MInt pos2 = str.find(i->first);
                        MInt len2 = i->second->Length();
                        if (pos2 >= 0 && len2 > 0) {
                            str = str.replace(pos2, len2, ret2);
                        } else {
                            break;
                        }
                    }
                }
            }

            return str;
        }

        void SetFormatItems(const std::shared_ptr<Map<MString, std::shared_ptr<IMFormatItem>>>& map) {
            m_items = map;
        }

        void SetInterceptorFormatItems(const std::shared_ptr<Vector<MString>>& vec) {
            m_interceptorItems = vec;
        }

        void SetPattern(MString pattern) {
            m_pattern = std::move(pattern);
        }

    private:
        std::shared_ptr<Map<MString, std::shared_ptr<IMFormatItem>>> m_items;
        std::shared_ptr<Vector<MString>> m_interceptorItems;
        MString m_pattern;
    };

    /******************************************************
     日志输出器
    ******************************************************/
    class IMLogAppender {
    public:
        IMLogAppender() = default;

        virtual ~IMLogAppender() = default;

    public:
        virtual void Log(IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) = 0;

    public:
        void SetFormatter(const std::shared_ptr<IMLogFormatter>& formatter) {
            m_formatter = formatter;
        }

        [[nodiscard]] const std::shared_ptr<IMLogFormatter>& GetFormatter() const {
            return m_formatter;
        }

        virtual void SetLevel(IMLogLevel::Level level) { m_level = level; }

    protected:
        IMLogLevel::Level m_level;
        std::shared_ptr<IMLogFormatter> m_formatter;
    };

    /******************************************************
    日志器
    ******************************************************/
    class IMLogger {
    public:
// %m -- message
// %p -- level
// %r -- elapse time
// %c -- log name
// %t -- thread id
// %n -- new line
// %d -- datetime
// %f -- filename
// %l -- line number
// %T -- tab
// {} -- args
        IMLogger()
                : m_level(IMLogLevel::DEBUG) {
            m_formatter = std::make_shared<IMLogFormatter>("%d [%p] %f %l [%c] %m %n");
            m_appenders = std::make_shared<Vector<std::shared_ptr<IMLogAppender>>>();
        }

        ~IMLogger() = default;

    public:
        void Log(IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) {
            const std::shared_ptr<IMLogEvent>& event2 = event;

            if (level >= m_level) {
                Vector<std::shared_ptr<IMLogAppender>>::iterator i;
                for (i = m_appenders->begin();
                     i != m_appenders->end(); ++i) {
                    (*i)->SetLevel(m_level);
                    (*i)->SetFormatter(m_formatter);
                    (*i)->Log(level, event);
                }
            }
        }

        void SetPattern(MString pattern) {
            m_formatter->SetPattern(std::move(pattern));
        }

        void Debug(IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) {
            Log(IMLogLevel::DEBUG, event);
        }

        void Info(IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) {
            Log(IMLogLevel::INFO, event);
        }

        void Warn(IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) {
            Log(IMLogLevel::WARN, event);
        }

        void Error(IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) {
            Log(IMLogLevel::ERROR, event);
        }

        void Fatal(IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) {
            Log(IMLogLevel::FATAL, event);
        }

        void AddAppender(const std::shared_ptr<IMLogAppender>& appender) {

            if (appender->GetFormatter()) {
                appender->SetFormatter(m_formatter);
            }
            m_appenders->push_back(appender);
        }

        void DelAppender(const std::shared_ptr<IMLogAppender>& appender) {
            for (auto iter = m_appenders->begin(); iter != m_appenders->end(); ++iter) {
                if (*iter == appender) {
                    m_appenders->erase(iter);
                    break;
                }
            }
        }

        [[nodiscard]] IMLogLevel::Level GetLevel() const {
            return m_level;
        }

        void SetLevel(IMLogLevel::Level level) {
            m_level = level;
        }

    private:
        IMLogLevel::Level m_level;
        std::shared_ptr<Vector<std::shared_ptr<IMLogAppender>>> m_appenders{};
        std::shared_ptr<IMLogFormatter> m_formatter{};
    };


    /******************************************************
    输出到标准控制台
    ******************************************************/
    class MStdOutLogAppender : public IMLogAppender {
    public:
        MStdOutLogAppender() {}

        ~MStdOutLogAppender() override = default;

    public:
        void Log(IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) override {
            const std::shared_ptr<IMLogEvent>& event2 = event;

            if (level >= m_level) {
                m_formatter->Init();
                std::cout << "" << m_formatter->Format(level, event) << std::flush;
            }
        }
    };

    /******************************************************
    输出到文件
    ******************************************************/
    class MFileLogAppender : public IMLogAppender {
    public:
        MFileLogAppender(MString  filename, std::ofstream& ofs)
                : m_filename(std::move(filename)), m_fileStream(ofs) {
            Reopen();
        }

        ~MFileLogAppender() override {
            m_fileStream.close();
        }

    public:
        void Log(IMLogLevel::Level level, const std::shared_ptr<IMLogEvent>& event) override {
            if (level >= m_level) {
                m_fileStream << m_formatter->Format(level, event);
            }
        }

        MBoolean Reopen() {
            if (m_fileStream.is_open()) {
                m_fileStream.close();
            }

            m_fileStream.open(m_filename);

            return !!m_fileStream;
        };

    private:
        MString m_filename;
        std::ofstream& m_fileStream;
    };


/******************************************************
简易定义
******************************************************/
#define IM_LOG_LEVEL(CONTENT, CLASS, LEVEL)  \
    MWD::Log::GMLogResourcesManager::GetLogger()->Log(LEVEL, std::shared_ptr<MWD::Log::IMLogEvent>(new MWD::Log::IMLogEvent(__FILE__, __LINE__, 0, MWD::GM::GMGetCurrentThreadId(), time(0), CONTENT, CLASS)));

#define IM_LOG_INFO(CONTENT, CLASS) IM_LOG_LEVEL(CONTENT, CLASS, MWD::Log::IMLogLevel::INFO)
#define IM_LOG_DEBUG(CONTENT, CLASS) IM_LOG_LEVEL(CONTENT, CLASS, MWD::Log::IMLogLevel::DEBUG)
#define IM_LOG_WARN(CONTENT, CLASS) IM_LOG_LEVEL(CONTENT, CLASS, MWD::Log::IMLogLevel::WARN)
#define IM_LOG_ERROR(CONTENT, CLASS) IM_LOG_LEVEL(CONTENT, CLASS, MWD::Log::IMLogLevel::ERROR)
#define IM_LOG_FATAL(CONTENT, CLASS) IM_LOG_LEVEL(CONTENT, CLASS, MWD::Log::IMLogLevel::FATAL)

#define IM_LOG_SET_DEFAULT_LEVEL(LEVEL) \
MWD::Log::GMLogResourcesManager::SetLoggerLevel(LEVEL);

#define IM_LOG_SET_DEFAULT_PATTERN(PATTERN) \
MWD::Log::GMLogResourcesManager::SetLoggerPattern(PATTERN);

#define IM_LOG_ENABLE_CONSOLE_OUTPUT \
MWD::Log::GMLogResourcesManager::EnableConsoleOutput();

#define IM_LOG_ENABLE_FILE_OUTPUT(FILENAME)\
MWD::Log::GMLogResourcesManager::EnableFileOutput(FILENAME);

    /******************************************************
    日志资源管理器
    ******************************************************/
    class GMLogResourcesManager {
    public:
        static std::shared_ptr<Map<MString, std::shared_ptr<IMFormatItem>>> GetFormatItems() {
            return MWD::GM::IMSingletonPtr<Map<MString, std::shared_ptr<IMFormatItem>>>::GetInstance();
        }

        static std::shared_ptr<Vector<MString>> GetInterceptorFormatItems() {
            return MWD::GM::IMSingletonPtr<Vector<MString>>::GetInstance();
        }

        static void AddFormatItems(const MString& key, const std::shared_ptr<IMFormatItem>& item) {
            GMLogResourcesManager::GetFormatItems()->insert(
                    std::pair<MString, std::shared_ptr<IMFormatItem>>(key, item));
        }

        static void AddInterceptorFormatItems(const MString& item) {
            GMLogResourcesManager::GetInterceptorFormatItems()->push_back(item);
        }

        static std::shared_ptr<IMLogger> GetLogger() {
            return MWD::GM::IMSingletonPtr<IMLogger>::GetInstance();
        }

        static void SetLoggerPattern(MString pattern) {
            GetLogger()->SetPattern(std::move(pattern));
        }

        static void SetLoggerLevel(IMLogLevel::Level level) {
            GetLogger()->SetLevel(level);
        }

        static void EnableConsoleOutput() {
            GetLogger()->AddAppender(std::make_shared<MStdOutLogAppender>());
        }

        static void EnableFileOutput(const MString& fileName) {
            static std::ofstream loggerFileOutputStream;
            GetLogger()->AddAppender(std::make_shared<MFileLogAppender>(fileName, loggerFileOutputStream));
        }
    };


} // namespace WMD::Log