#include "IMLoggerService.h"

namespace MWD::Service::Log {
    /******************************************************
     格式执行器Impl
    ******************************************************/
    //////////////////////////////////////////////////
    // message
    MString MMessageFormatItem::Format(
            MString str,
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        return event->GetContent();
    }

    //////////////////////////////////////////////////
    // level
    MString STDMETHODCALLTYPE MLevelFormatItem::Format(
            MString str,
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        return IMLogLevel::ToString(level);
    }

    //////////////////////////////////////////////////
    // elapse
    MString STDMETHODCALLTYPE MElapseFormatItem::Format(
            MString str,
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        MChar buf[16];
        sprintf(buf, "%d", event->GetElapse());
        return buf;
    }

    //////////////////////////////////////////////////
    // name
    MString STDMETHODCALLTYPE MNameFormatItem::Format(
            MString str,
            IMLogLevel::Level level,
            const std::shared_ptr<IMLogEvent>& event) {
        return event->GetLogName();
    }

    //////////////////////////////////////////////////
    // thread id
    MString STDMETHODCALLTYPE MThreadFormatItem::Format(
            MString str,

            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        MChar buf[16];
        sprintf(buf, "%d", event->GetThreadId());
        return buf;
    }

    //////////////////////////////////////////////////
    // datetime
    MString STDMETHODCALLTYPE MDatetimeFormatItem::Format(
            MString str,
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        std::time_t now = time(nullptr);
        std::tm tm{};
        localtime_s(&tm, &now);
        MChar buf[64];

        MString newSymbol = m_symbol + '{';
        MInt argsStartPos = str.find(newSymbol);
        if (argsStartPos >= 0) {
            MInt argsEndPos = str.find('}', argsStartPos);

            if (argsEndPos > argsStartPos) {
                m_format = str.substr(argsStartPos + m_symbol.length() + 1,
                                      argsEndPos - argsStartPos - m_symbol.length() - 1);

                MInt pos = m_format.find('$');
                while (pos >= 0) {
                    m_format = m_format.replace(pos, 1, "%");
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

    MInt STDMETHODCALLTYPE MDatetimeFormatItem::Length() {
        return m_length == 0 ? m_symbol.length() : m_length;
    }

    //////////////////////////////////////////////////
    // filename
    MString STDMETHODCALLTYPE MFilenameFormatItem::Format(
            MString str,
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        return event->GetFile();
    }

    //////////////////////////////////////////////////
    // line number
    MString STDMETHODCALLTYPE MLineFormatItem::Format(
            MString str,
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        MChar buf[64];
        sprintf(buf, "%d", event->GetLine());
        return buf;
    }

    //////////////////////////////////////////////////
    // new line
    MString STDMETHODCALLTYPE MNewLineFormatItem::Format(
            MString str,
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        return "\n";
    }

    //////////////////////////////////////////////////
    // tab
    MString STDMETHODCALLTYPE MTabFormatItem::Format(
            MString str,
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        return "\t";
    }

    MBoolean STDMETHODCALLTYPE MTabFormatItem::IsRepeat() {
        return true;
    }

    //////////////////////////////////////////////////
    // 日志格式
    void IMLogFormatter::Init() {
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
    m_items->insert(std::pair<MString, IMFormatItem::ptr>(KEY, std::shared_ptr<CLASS>(new CLASS(KEY))));

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

    MString IMLogFormatter::Format(
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
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

    void IMLogFormatter::SetFormatItems(
            const MapPtr<MString, IMFormatItem::ptr>& map) {
        m_items = map;
    }

    void IMLogFormatter::SetInterceptorFormatItems(
            const VectorPtr<MString>& vec) {
        m_interceptorItems = vec;
    }

    void IMLogFormatter::SetPattern(
            MString pattern) {
        m_pattern = std::move(pattern);
    }

    //////////////////////////////////////////////////
    // 日志输出器
    void IMLogAppender::SetLevel(IMLogLevel::Level level) {
        m_level = level;
    }

    void IMLogAppender::SetFormatter(const IMLogFormatter::ptr& formatter) {
        m_formatter = formatter;
    }

    [[nodiscard]] const IMLogFormatter::ptr& IMLogAppender::GetFormatter() const {
        return m_formatter;
    }

    //////////////////////////////////////////////////
    // 日志器
    void STDMETHODCALLTYPE IMLogger::Log(
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        const IMLogEvent::ptr& event2 = event;

        if (level >= m_level) {
            auto i = m_appenders->begin();
            for (; i != m_appenders->end(); ++i) {
                (*i)->SetLevel(m_level);
                (*i)->SetFormatter(m_formatter);
                (*i)->Log(level, event);
            }
        }
    }

    void STDMETHODCALLTYPE IMLogger::SetPattern(MString pattern) {
        m_formatter->SetPattern(std::move(pattern));
    }

    void STDMETHODCALLTYPE IMLogger::Debug(
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        Log(IMLogLevel::DEBUG, event);
    }

    void STDMETHODCALLTYPE IMLogger::Info(
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        Log(IMLogLevel::INFO, event);
    }

    void STDMETHODCALLTYPE IMLogger::Warn(
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        Log(IMLogLevel::WARN, event);
    }

    void STDMETHODCALLTYPE IMLogger::Error(
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        Log(IMLogLevel::ERROR, event);
    }

    void STDMETHODCALLTYPE IMLogger::Fatal(
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        Log(IMLogLevel::FATAL, event);
    }

    void STDMETHODCALLTYPE IMLogger::AddAppender(
            const IMLogAppender::ptr& appender) {

        if (appender->GetFormatter()) {
            appender->SetFormatter(m_formatter);
        }
        m_appenders->push_back(appender);
    }

    void STDMETHODCALLTYPE IMLogger::DelAppender(
            const IMLogAppender::ptr& appender) {
        auto iter = m_appenders->begin();
        for (; iter != m_appenders->end(); ++iter) {
            if (*iter == appender) {
                m_appenders->erase(iter);
                break;
            }
        }
    }

    [[nodiscard]] IMLogLevel::Level STDMETHODCALLTYPE IMLogger::GetLevel() const {
        return m_level;
    }

    void STDMETHODCALLTYPE IMLogger::SetLevel(IMLogLevel::Level level) {
        m_level = level;
    }

    //////////////////////////////////////////////////
    // 输出到标准控制台
    void STDMETHODCALLTYPE MStdOutLogAppender::Log(
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        const IMLogEvent::ptr& event2 = event;

        if (level >= m_level) {
            m_formatter->Init();
            std::cout << "" << m_formatter->Format(level, event) << std::flush;
        }
    }

    //////////////////////////////////////////////////
    // 输出到文件
    void STDMETHODCALLTYPE MFileLogAppender::Log(
            IMLogLevel::Level level,
            const IMLogEvent::ptr& event) {
        if (level >= m_level) {
            m_fileStream << m_formatter->Format(level, event);
        }
    }

    MBoolean MFileLogAppender::Reopen() {
        if (m_fileStream.is_open()) {
            m_fileStream.close();
        }

        m_fileStream.open(m_filename);

        return !!m_fileStream;
    };

    //////////////////////////////////////////////////
    // 日志服务
    SIID SIID_IM_SERVICE_LOGGER{
            {0,0,0,0,0,0,0,1},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2}
    };
    /********************************************************
     IMService方法实现
    *********************************************************/
    /// <summary>
    /// 测试是否可转
    /// </summary>
    MBoolean STDMETHODCALLTYPE MLoggerService::QueryInterface(
            /* [in] */SIID siid
    ) {
        return siid == SIID_IM_SERVICE_LOGGER;
    }

    /// <summary>
    /// 获取iid
    /// </summary>
    SIID STDMETHODCALLTYPE MLoggerService::GetSIID(
            /* [in] */char Data2[16]
    ) {
        for (int i = 0; i < sizeof(SIID_IM_SERVICE_LOGGER.Data2); ++i) {
            if (Data2[i] != SIID_IM_SERVICE_LOGGER.Data2[i]) {
                return SIID_SERVICE_INVALID;
            }
        }

        return SIID_IM_SERVICE_LOGGER;
    }

    /********************************************************
     自身方法
    *********************************************************/
    void STDMETHODCALLTYPE MLoggerService::UpdateConfig(
            /* [in][要更改的配置] */const MString& key,
            /* [in][要更改的配置] */const MString& value
            ) {

    }

    void STDMETHODCALLTYPE MLoggerService::Init() {
        if (m_isInit) {
            // 默认添加标准控制台输出
            IMLogAppender::ptr appender = MakeShared(MStdOutLogAppender);
            m_logger->AddAppender(appender);

            m_isInit = false;
        }
    }

    void STDMETHODCALLTYPE MLoggerService::Log(
            const IMLogLevel::Level& level,
            const IMLogEvent::ptr& event
            ) {
        m_logger->Log(level, event);
    }
}// MWD::Service::Log

/// <summary>
/// 日志服务siid
/// </summary>
MWD::Service::SIID SIID_IM_SERVICE_LOGGER{
        {0,0,0,0,0,0,0,1},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2}
};

/// <summary>
/// dll函数定义
/// </summary>
GM_DEFINE_REGISTER_FUNC_AND_DESTROY(MWD::Service::Log::MLoggerService, SIID_IM_SERVICE_LOGGER)





