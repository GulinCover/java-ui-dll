#pragma once

#include "IMCommon.h"
#include "IMService.h"

namespace MWD::Service::Timer {
    using namespace MWD::Service;

    //////////////////////////////////////////////////
    // 任务(单位毫秒)
    class IMTaskHandle {
    public:
        using ptr = std::shared_ptr<IMTaskHandle>;
    public:
        IMTaskHandle(MULLong id, MLong delay, MLong interval, void* args,
                     std::function<void(void*)> func)
                : m_id(id), m_delayTime(delay), m_intervalTime(interval), m_args(args) {
            m_handle = std::move(func);
            m_startTime = std::chrono::high_resolution_clock::now();
            m_number = 0;
        }

    public:
        [[nodiscard]] MULLong GetId() const;

        [[nodiscard]] MLong GetDelayTime() const;

        [[nodiscard]] MLong GetIntervalTime() const;

        void ExecTask() const;

        void SetId(MULLong id);

        std::chrono::high_resolution_clock::time_point GetFirstExecTime();

        void UpdateFirstExecTime();

    private:
        MULLong m_id;
        MLong m_delayTime;
        MLong m_intervalTime;
        std::chrono::high_resolution_clock::time_point m_startTime;
        void* m_args;
        std::function<void(void*)> m_handle;
        volatile std::atomic<MULLong> m_number;
    };

    //////////////////////////////////////////////////
    // 定时器任务容器
    class IMTimerContainer {
    public:
        using ptr = std::shared_ptr<IMTimerContainer>;
    public:
        IMTimerContainer() {
            m_map = std::make_shared<Map<MULLong, IMTaskHandle::ptr>>();
            m_vec = std::make_shared<Vector<IMTaskHandle::ptr>>();
        }

        ~IMTimerContainer() {
            m_map->clear();
            m_vec->clear();
        }

    public:
        MString Print();

        void Sort();

        MLong GetSize();

        void ClearContainer();

        void AddTimerTask(const IMTaskHandle::ptr& task);

        void DeleteTimerTask(MULLong id);

        IMTaskHandle::ptr GetExecTask();

        IMTaskHandle::ptr GetSecondExecTask();

    private:
        MapPtr<MULLong, IMTaskHandle::ptr> m_map;
        VectorPtr<IMTaskHandle::ptr> m_vec;
    };

    //////////////////////////////////////////////////
    // 定时器任务执行类
    abstract IMTimerTask {
    public:
        using ptr = std::shared_ptr<IMTimerTask>;
    public:
        static std::chrono::high_resolution_clock::time_point CurrentTime() {
            return std::chrono::high_resolution_clock::now();
        }

        virtual void STDMETHODCALLTYPE ExecTask(
                const IMTaskHandle::ptr& taskHandle
        ) PURE;
    };

    //////////////////////////////////////////////////
    // 定时器任务执行类Impl
    class MTimerTaskHandle : public IMTimerTask {
    public:
        using ptr = std::shared_ptr<MTimerTaskHandle>;
    public:
        void STDMETHODCALLTYPE ExecTask(
                const IMTaskHandle::ptr& taskHandle
        ) override;
    };

    //////////////////////////////////////////////////
    // 定时器基类
    interface IMTimer : public IMService {
    public:
        using ptr = std::shared_ptr<IMTimer>;
    public:
        /// <summary>
        /// 更改当前日志配置
        /// </summary>
        virtual void STDMETHODCALLTYPE UpdateConfig(
                /* [in][要更改的配置] */const MString& key,
                /* [in][要更改的配置] */const MString& value
        )PURE;

        virtual void STDMETHODCALLTYPE Init()PURE;

        virtual void STDMETHODCALLTYPE ClearTimer()PURE;

        virtual void STDMETHODCALLTYPE DeleteTimerTask(const MULLong& id)PURE;

        virtual MULLong STDMETHODCALLTYPE AddTimerTask(const IMTaskHandle::ptr& task, MULLong& id)PURE;

        virtual void STDMETHODCALLTYPE StartupTimerQueue() PURE;
    };

    //////////////////////////////////////////////////
    // 定时器服务实现
    class MTimerService : public IMTimer {
    public:
        explicit MTimerService() {
            m_timerTask = MakeShared(MTimerTaskHandle);
            m_timerContainer = MakeShared(IMTimerContainer);
        }

        ~MTimerService() override = default;

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
        /// 获取iid
        /// </summary>
        SIID STDMETHODCALLTYPE GetSIID(
                /* [in] */char Data2[16]
        ) override;

    public:
        /********************************************************
         IMTimer方法实现
        *********************************************************/
        /// <summary>
        /// 更改当前日志配置
        /// </summary>
        void STDMETHODCALLTYPE UpdateConfig(
                /* [in][要更改的配置] */const MString& key,
                /* [in][要更改的配置] */const MString& value
        ) override;

        void STDMETHODCALLTYPE Init() override;

        void STDMETHODCALLTYPE ClearTimer() override;

        void STDMETHODCALLTYPE DeleteTimerTask(const MULLong& id) override;

        MULLong STDMETHODCALLTYPE AddTimerTask(const IMTaskHandle::ptr& task, MULLong& id) override;

        [[noreturn]] void STDMETHODCALLTYPE StartupTimerQueue() override;

    private:
        IMTimerTask::ptr m_timerTask;
        IMTimerContainer::ptr m_timerContainer;
        volatile std::atomic<MULLong> m_id = 0;

        std::mutex m_mtx;
        std::condition_variable m_signal;
    };

#define IM_TIMER_SAMPLE_SET_TIMER(ID, DELAY, INTERVAL, ARGS, FUNC) \
    std::make_shared<MWD::Timer::IMTaskHandle>(ID, DELAY, INTERVAL, ARGS, FUNC)

} // MWD::Service::Timer
