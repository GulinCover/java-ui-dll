#include "IMService.h"

namespace MWD::Service::Log {
    using namespace MWD::Service;
    //////////////////////////////////////////////////
    // 日志事件
    abstract IMLogEvent {
    public:
        using ptr = std::shared_ptr<IMLogEvent>;
    public:
        IMLogEvent(const MChar* file, MInt line, MUInt elspase, MUInt threadId, MUInt time, MString content,
                   MString logName)
                : m_file(file), m_line(line), m_elapse(elspase), m_threadId(threadId), m_time(time),
                  m_content(std::move(content)),
                  m_logName(std::move(logName)) {
        }

        virtual ~IMLogEvent() = default;

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


    //////////////////////////////////////////////////
    // 日志级别
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
            }
        }
    };

    //////////////////////////////////////////////////
    // 格式执行器
    abstract IMFormatItem {
    public:
        using ptr = std::shared_ptr<IMFormatItem>;
    public:
        explicit IMFormatItem(MString symbol) : m_symbol(std::move(symbol)) {
        }

        virtual ~IMFormatItem() = default;

    public:
        virtual MString STDMETHODCALLTYPE Format(
                /* [in] */MString str,
                /* [in] */IMLogLevel::Level level,
                /* [in] */const IMLogEvent::ptr& event) PURE;

        virtual MBoolean STDMETHODCALLTYPE IsSpecialType() { return false; }

        virtual void STDMETHODCALLTYPE ExecSpecialMethod(MString args[]) {}

        virtual MInt STDMETHODCALLTYPE Length() {
            return m_symbol.length();
        }

        virtual MBoolean STDMETHODCALLTYPE IsRepeat() { return false; }

    protected:
        MString m_symbol;
    };

    /******************************************************
     格式执行器Impl
    ******************************************************/
    //////////////////////////////////////////////////
    // message
    class MMessageFormatItem : public IMFormatItem {
    public:
        using ptr = std::shared_ptr<MMessageFormatItem>;
    public:
        explicit MMessageFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString STDMETHODCALLTYPE Format(
                MString str,
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event) override;
    };

    //////////////////////////////////////////////////
    // level
    class MLevelFormatItem : public IMFormatItem {
    public:
        using ptr = std::shared_ptr<MLevelFormatItem>;
    public:
        explicit MLevelFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString STDMETHODCALLTYPE Format(
                MString str,
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event) override;
    };

    //////////////////////////////////////////////////
    // elapse
    class MElapseFormatItem : public IMFormatItem {
    public:
        using ptr = std::shared_ptr<MElapseFormatItem>;
    public:
        explicit MElapseFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString STDMETHODCALLTYPE Format(
                MString str,
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event) override;
    };

    //////////////////////////////////////////////////
    // name
    class MNameFormatItem : public IMFormatItem {
    public:
        using ptr = std::shared_ptr<MNameFormatItem>;
    public:
        explicit MNameFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString STDMETHODCALLTYPE Format(
                MString str,
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event) override;
    };

    //////////////////////////////////////////////////
    // thread id
    class MThreadFormatItem : public IMFormatItem {
    public:
        using ptr = std::shared_ptr<MThreadFormatItem>;
    public:
        explicit MThreadFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString STDMETHODCALLTYPE Format(
                MString str,
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event) override;
    };

    //////////////////////////////////////////////////
    // datetime
    class MDatetimeFormatItem : public IMFormatItem {
    public:
        using ptr = std::shared_ptr<MDatetimeFormatItem>;
    public:
        explicit MDatetimeFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {
            m_length = std::strlen(symbol.c_str());
        }

    public:
        MString STDMETHODCALLTYPE Format(
                MString str,
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event) override;

        MInt STDMETHODCALLTYPE Length() override;
    private:
        MString m_format;
        MInt m_length;
    };

    //////////////////////////////////////////////////
    // filename
    class MFilenameFormatItem : public IMFormatItem {
    public:
        using ptr = std::shared_ptr<MFilenameFormatItem>;
    public:
        explicit MFilenameFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString STDMETHODCALLTYPE Format(
                MString str,
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event) override;
    };

    //////////////////////////////////////////////////
    // line number
    class MLineFormatItem : public IMFormatItem {
    public:
        using ptr = std::shared_ptr<MLineFormatItem>;
    public:
        explicit MLineFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString STDMETHODCALLTYPE Format(
                MString str,
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event) override;
    };

    //////////////////////////////////////////////////
    // new line
    class MNewLineFormatItem : public IMFormatItem {
    public:
        using ptr = std::shared_ptr<MNewLineFormatItem>;
    public:
        explicit MNewLineFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString STDMETHODCALLTYPE Format(
                MString str,
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event) override;
    };

    //////////////////////////////////////////////////
    // tab
    class MTabFormatItem : public IMFormatItem {
    public:
        using ptr = std::shared_ptr<MTabFormatItem>;
    public:
        explicit MTabFormatItem(MString symbol) : IMFormatItem(std::move(symbol)) {}

    public:
        MString STDMETHODCALLTYPE Format(
                MString str,
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event) override;

        MBoolean STDMETHODCALLTYPE IsRepeat() override;
    };


    //////////////////////////////////////////////////
    // 日志格式
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
    class IMLogFormatter {
    public:
        using ptr = std::shared_ptr<IMLogFormatter>;
    public:
        explicit IMLogFormatter(MString pattern) : m_pattern(std::move(pattern)) {
            m_items = MakeMap(MString, IMFormatItem::ptr);
            m_interceptorItems = MakeShared(Vector<MString>);
        }

    public:
        void Init();

        MString Format(
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event);

        void SetFormatItems(const MapPtr<MString, IMFormatItem::ptr>& map);

        void SetInterceptorFormatItems(const VectorPtr<MString>& vec);

        void SetPattern(MString pattern);

    private:
        MapPtr<MString, IMFormatItem::ptr> m_items;
        VectorPtr<MString> m_interceptorItems;
        MString m_pattern;
    };


    //////////////////////////////////////////////////
    // 日志输出器
    abstract IMLogAppender {
    public:
        using ptr = std::shared_ptr<IMLogAppender>;
    public:
        IMLogAppender() = default;

        virtual ~IMLogAppender() = default;

    public:
        /// <summary>
        /// 日志输出
        /// </summary>
        virtual void STDMETHODCALLTYPE Log(
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event) PURE;

        virtual void SetLevel(IMLogLevel::Level level);
    public:
        void SetFormatter(const IMLogFormatter::ptr& formatter);

        [[nodiscard]] const IMLogFormatter::ptr& GetFormatter() const;

    protected:
        IMLogLevel::Level m_level;
        IMLogFormatter::ptr m_formatter;
    };

    //////////////////////////////////////////////////
    // 日志器基类
    abstract IMLogger {
    public:
        using ptr = std::shared_ptr<IMLogger>;
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
            m_appenders = std::make_shared<Vector<IMLogAppender::ptr>>();
        }

        ~IMLogger() = default;

    public:
        virtual void STDMETHODCALLTYPE Log(
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event);

        virtual void STDMETHODCALLTYPE SetPattern(
                MString pattern);

        virtual void STDMETHODCALLTYPE Debug(
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event);

        virtual void STDMETHODCALLTYPE Info(
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event);

        virtual void STDMETHODCALLTYPE Warn(
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event);

        virtual void STDMETHODCALLTYPE Error(
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event);

        virtual void STDMETHODCALLTYPE Fatal(
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event);

        virtual void STDMETHODCALLTYPE AddAppender(
                const IMLogAppender::ptr& appender);

        virtual void STDMETHODCALLTYPE DelAppender(
                const IMLogAppender::ptr& appender);

        [[nodiscard]] virtual IMLogLevel::Level STDMETHODCALLTYPE GetLevel() const;

        virtual void STDMETHODCALLTYPE SetLevel(IMLogLevel::Level level);

    private:
        IMLogLevel::Level m_level;
        VectorPtr<IMLogAppender::ptr> m_appenders;
        IMLogFormatter::ptr m_formatter;
    };

    //////////////////////////////////////////////////
    // 输出到标准控制台
    class MStdOutLogAppender : public IMLogAppender {
    public:
        using ptr = std::shared_ptr<MStdOutLogAppender>;
    public:
        MStdOutLogAppender() = default;

        ~MStdOutLogAppender() override = default;

    public:
        void STDMETHODCALLTYPE Log(
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event) override;
    };

    //////////////////////////////////////////////////
    // 输出到文件
    class MFileLogAppender : public IMLogAppender {
    public:
        using ptr = std::shared_ptr<MFileLogAppender>;
    public:
        MFileLogAppender(MString  filename, std::ofstream& ofs)
                : m_filename(std::move(filename)), m_fileStream(ofs) {
            Reopen();
        }

        ~MFileLogAppender() override {
            m_fileStream.close();
        }

    public:
        void STDMETHODCALLTYPE Log(
                IMLogLevel::Level level,
                const IMLogEvent::ptr& event) override;

        MBoolean Reopen();

    private:
        MString m_filename;
        std::ofstream& m_fileStream;
    };

    //////////////////////////////////////////////////
    // 日志服务
    class MLoggerService : public IMService {
    public:
        using ptr = std::shared_ptr<MLoggerService>;
    public:
        MLoggerService() {
            m_logger = MakeShared(IMLogger);
            m_isInit = true;
        }
    public:
        /********************************************************
         IMService方法实现
        *********************************************************/
        /// <summary>
        /// 测试是否可转
        /// </summary>
        MBoolean STDMETHODCALLTYPE QueryInterface(
                /* [in] */SIID siid
        ) override;

        /// <summary>
        /// 获取siid
        /// </summary>
        SIID STDMETHODCALLTYPE GetSIID(
                /* [in] */char Data2[16]
        ) override;
    public:
        /// <summary>
        /// 更改当前日志配置
        /// </summary>
        virtual void STDMETHODCALLTYPE UpdateConfig(
                /* [in][要更改的配置] */const MString& key,
                /* [in][要更改的配置] */const MString& value
                );

        virtual void STDMETHODCALLTYPE Init();

        virtual void STDMETHODCALLTYPE Log(
                const IMLogLevel::Level& level,
                const IMLogEvent::ptr& event
                );
    private:
        IMLogger::ptr m_logger;
        MBoolean m_isInit;
    };

#define IM_SIMPLE_LOGGER_EVENT(CONTENT, CLASS_NAME) \
    std::make_shared<MWD::Service::Log::IMLogEvent>(__FILE__, __LINE__, 0, GMGetCurrentProcessId(), time(0), CONTENT, CLASS_NAME)


}// MWD::Service::Log
