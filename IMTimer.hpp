#pragma once

#include <utility>

#include "IMCommon.hpp"
#include "IMLogger.hpp"
#include "IMConfig.hpp"

namespace MWD::Timer {
    /********************************************************
     任务(单位毫秒)
    *********************************************************/
    class IMTaskHandle {
    public:
        IMTaskHandle(MULLong id, MLong delay, MLong interval, void* args,
                     std::function<void(void*)> func)
                : m_id(id), m_delayTime(delay), m_intervalTime(interval), m_args(args) {
            m_handle = std::move(func);
            m_startTime = std::chrono::high_resolution_clock::now();
            m_number = 0;
        }

    public:
        [[nodiscard]] MULLong GetId() const { return m_id; }

        [[nodiscard]] MLong GetDelayTime() const { return m_delayTime; }

        [[nodiscard]] MLong GetIntervalTime() const { return m_intervalTime; }

        void ExecTask() const { m_handle(m_args); }

        void SetId(MULLong id) {
            m_id = id;
        }

        std::chrono::high_resolution_clock::time_point GetFirstExecTime() {
            if (!m_number.load()) {
                return m_startTime + std::chrono::milliseconds(m_delayTime);
            }
            return m_startTime + std::chrono::milliseconds(m_delayTime + m_intervalTime * m_number);
        }

        void UpdateFirstExecTime() {
            m_number++;
        }

    private:
        MULLong m_id;
        MLong m_delayTime;
        MLong m_intervalTime;
        std::chrono::high_resolution_clock::time_point m_startTime;
        void* m_args;
        std::function<void(void*)> m_handle;
        volatile std::atomic<MULLong> m_number;
    };

    /********************************************************
     定时器任务容器
    *********************************************************/
    class IMTimerContainer {
    public:
        IMTimerContainer() {
            m_map = std::make_shared<Map<MULLong, std::shared_ptr<IMTaskHandle>>>();
            m_vec = std::make_shared<Vector<std::shared_ptr<IMTaskHandle>>>();
        }

        ~IMTimerContainer() {
            m_map->clear();
            m_vec->clear();
        }

    public:
        MString Print() {
            std::stringstream ss;
            for (auto item = m_vec->begin();item != m_vec->end(); ++item) {
                ss << "id-->" << (*item)->GetId() << " | " << "container-->" << std::chrono::duration_cast<std::chrono::seconds>((*item)->GetFirstExecTime().time_since_epoch()).count();
            }

            return ss.str();
        }

        void Sort() {
            std::sort(m_vec->begin(), m_vec->end(), [](const std::shared_ptr<IMTaskHandle>& p, const std::shared_ptr<IMTaskHandle>& n){
                return (p->GetFirstExecTime() - n->GetFirstExecTime()).count() < 0;
            });
        }

        MLong GetSize() {
            return m_vec->size();
        }

        void ClearContainer() {
            m_map->clear();
            m_vec->clear();
        }

        void AddTimerTask(const std::shared_ptr<IMTaskHandle>& task) {
            auto item = m_map->find(task->GetId());
            if (item != m_map->end()) {
                m_map->erase(item);
            }

            m_map->insert(std::make_pair(task->GetId(), task));
            m_vec->push_back(task);
            Sort();
        }

        void DeleteTimerTask(MULLong id) {
            auto item = m_map->find(id);
            if (item != m_map->end()) {
                if (item->second->GetId() == id) {
                    m_map->erase(item);
                }
            }

            for (auto i = m_vec->begin(); i != m_vec->end(); ++i) {
                if ((*i)->GetId() == id) {
                    m_vec->erase(i);
                    break;
                }
            }

            Sort();
        }

        std::shared_ptr<IMTaskHandle> GetExecTask() {
            return *(m_vec->begin());
        }

        std::shared_ptr<IMTaskHandle> GetSecondExecTask() {
            if (m_vec->size() == 1) {
                return *(m_vec->begin());
            }
            auto item = m_vec->begin();
            ++item;
            return *item;
        }

    private:
        std::shared_ptr<Map<MULLong, std::shared_ptr<IMTaskHandle>>> m_map;
        std::shared_ptr<Vector<std::shared_ptr<IMTaskHandle>>> m_vec;
    };

    /********************************************************
     定时器任务执行类
    *********************************************************/
    class IMTimerTask {
    public:
        static std::chrono::high_resolution_clock::time_point CurrentTime() {
            return std::chrono::high_resolution_clock::now();
        }

        virtual void ExecTask(const std::shared_ptr<IMTaskHandle>& taskHandle) = 0;
    };

    /********************************************************
     定时器任务执行类Impl
    *********************************************************/
    class IMTimerTaskHandle : public IMTimerTask {
    public:
        void ExecTask(const std::shared_ptr<IMTaskHandle>& taskHandle) override {
            taskHandle->ExecTask();
            taskHandle->UpdateFirstExecTime();
        }
    };

    /********************************************************
     定时器类
    *********************************************************/
    class IMTimer {
    public:
        explicit IMTimer() {
            m_timerTask = std::make_shared<IMTimerTaskHandle>();
            m_timerContainer = std::make_shared<IMTimerContainer>();
        }

        ~IMTimer() = default;

    public:
        void SetIMTimerContainer(std::shared_ptr<IMTimerContainer> container) {
            m_timerContainer = std::move(container);
        }

        void ClearTimer() {
            m_timerContainer->ClearContainer();

            m_signal.notify_all();
        }

        void DeleteTimerTask(MULLong& id) {
            m_timerContainer->DeleteTimerTask(id);

            m_signal.notify_all();

        }

        MULLong AddTimerTask(const std::shared_ptr<IMTaskHandle>& task, MULLong& id) {
            m_id++;
            id = m_id.load();
            if (!task->GetId()) {
                task->SetId(id);
            }
            m_timerContainer->AddTimerTask(task);

            m_signal.notify_all();

            return id;
        }

        void SetTimerTaskHandle(std::shared_ptr<IMTimerTask> taskHandle) {
            m_timerTask = std::move(taskHandle);
        }

        [[noreturn]] void StartupTimerQueue() {
            while (true) {
                m_timerContainer->Sort();

                std::unique_lock<std::mutex> lock(m_mtx);
                MBoolean flag = true;

                while (flag) {
                    if (m_timerContainer->GetSize() == 0) {
                        m_signal.wait(lock);
                    } else {
                        std::shared_ptr<IMTaskHandle> h = m_timerContainer->GetExecTask();
                        std::chrono::high_resolution_clock::time_point tp = h->GetFirstExecTime();

                        m_signal.wait_until(lock, tp);

                        auto ct = IMTimerTask::CurrentTime();
                        std::chrono::duration<MDouble, std::milli> ms = ct - tp;

                        if (ms.count() >= 0) {
                            flag = false;
                        }
                    }
                }

                std::shared_ptr<IMTaskHandle> h = m_timerContainer->GetExecTask();
                m_timerTask->ExecTask(h);

                MString ret = IM_CONFIG_GET_ARG("mwd.timer.container.consoleOutput");
                std::transform<MString::iterator, MString::iterator, int(int)>(ret.begin(), ret.end(), ret.begin(), std::toupper);
                if (ret == "TRUE") {
                    IM_LOG_DEBUG(m_timerContainer->Print(), "IMTimer");
                }
            }
        }

    private:
        std::shared_ptr<IMTimerTask> m_timerTask;
        std::shared_ptr<IMTimerContainer> m_timerContainer;
        volatile std::atomic<MULLong> m_id = 0;

        std::mutex m_mtx;
        std::condition_variable m_signal;
    };

    /********************************************************
     定时器资源管理器
    *********************************************************/
    class IMTimerResourcesManager {
    public:
        static std::shared_ptr<IMTimer> GetTimerCore() {
            return MWD::GM::IMSingletonPtr<IMTimer>::GetInstance();
        }

        static void SetTimerTaskHandle(std::shared_ptr<IMTimerTask> taskHandle) {
            auto i = GetTimerCore();
            i->SetTimerTaskHandle(std::move(taskHandle));
        }

        static MULLong AddTimerTask(const std::shared_ptr<IMTaskHandle>& task, MULLong& id) {
            auto i = GetTimerCore();
            return i->AddTimerTask(task, id);
        }

        static void StartupTimer() {
            auto i = GetTimerCore();
            i->StartupTimerQueue();
        }
    };

#define IM_TIMER_SET_TASK_HANDLE(TASK_HANDLE) \
    MWD::Timer::IMTimerResourcesManager::SetTimerTaskHandle(TASK_HANDLE);

#define IM_TIMER_SET_DEFAULT_HANDLE \
    IM_TIMER_SET_TASK_HANDLE(std::make_shared<IMTimerTaskHandle>());

#define IM_TIMER_ADD_TASK(HANDLE, ID) \
        MWD::Timer::IMTimerResourcesManager::AddTimerTask(HANDLE, ID);

#define IM_TIMER_SET_TIMER(ID, DELAY, INTERVAL, ARGS, FUNC) \
    IM_TIMER_ADD_TASK(std::make_shared<MWD::Timer::IMTaskHandle>(ID, DELAY, INTERVAL, ARGS, FUNC), ID);

#define IM_TIMER_STARTUP \
    MWD::Timer::IMTimerResourcesManager::StartupTimer();

}//MWD::Timer
