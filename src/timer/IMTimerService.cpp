#include "IMTimerService.h"

namespace MWD::Service::Timer {
    using namespace MWD::Service;
    //////////////////////////////////////////////////
    // 任务(单位毫秒)
    [[nodiscard]] MULLong IMTaskHandle::GetId() const { return m_id; }

    [[nodiscard]] MLong IMTaskHandle::GetDelayTime() const { return m_delayTime; }

    [[nodiscard]] MLong IMTaskHandle::GetIntervalTime() const { return m_intervalTime; }

    void IMTaskHandle::ExecTask() const { m_handle(m_args); }

    void IMTaskHandle::SetId(MULLong id) {
        m_id = id;
    }

    std::chrono::high_resolution_clock::time_point IMTaskHandle::GetFirstExecTime() {
        if (!m_number.load()) {
            return m_startTime + std::chrono::milliseconds(m_delayTime);
        }
        return m_startTime + std::chrono::milliseconds(m_delayTime + m_intervalTime * m_number);
    }

    void IMTaskHandle::UpdateFirstExecTime() {
        m_number++;
    }

    //////////////////////////////////////////////////
    // 定时器任务容器
    MString IMTimerContainer::Print() {
        std::stringstream ss;
        for (auto item = m_vec->begin(); item != m_vec->end(); ++item) {
            ss << "id-->" << (*item)->GetId() << " | " << "container-->"
               << std::chrono::duration_cast<std::chrono::seconds>(
                       (*item)->GetFirstExecTime().time_since_epoch()).count();
        }

        return ss.str();
    }

    void IMTimerContainer::Sort() {
        std::sort(m_vec->begin(), m_vec->end(),
                  [](const IMTaskHandle::ptr& p, const IMTaskHandle::ptr& n) {
                      return (p->GetFirstExecTime() - n->GetFirstExecTime()).count() < 0;
                  });
    }

    MLong IMTimerContainer::GetSize() {
        return m_vec->size();
    }

    void IMTimerContainer::ClearContainer() {
        m_map->clear();
        m_vec->clear();
    }

    void IMTimerContainer::AddTimerTask(const IMTaskHandle::ptr& task) {
        auto item = m_map->find(task->GetId());
        if (item != m_map->end()) {
            m_map->erase(item);
        }

        m_map->insert(std::make_pair(task->GetId(), task));
        m_vec->push_back(task);
        Sort();
    }

    void IMTimerContainer::DeleteTimerTask(MULLong id) {
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

    IMTaskHandle::ptr IMTimerContainer::GetExecTask() {
        return *(m_vec->begin());
    }

    IMTaskHandle::ptr IMTimerContainer::GetSecondExecTask() {
        if (m_vec->size() == 1) {
            return *(m_vec->begin());
        }
        auto item = m_vec->begin();
        ++item;
        return *item;
    }

    //////////////////////////////////////////////////
    // 定时器任务执行类Impl
    void STDMETHODCALLTYPE MTimerTaskHandle::ExecTask(const IMTaskHandle::ptr& taskHandle) {
        taskHandle->ExecTask();
        taskHandle->UpdateFirstExecTime();
    }

    //////////////////////////////////////////////////
    // 定时器服务实现
    SIID SIID_IM_TIMER_SERVICE {
            {0,0,0,0,0,0,0,1},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3}
    };
    /********************************************************
     IMService方法实现
    *********************************************************/
    MBoolean STDMETHODCALLTYPE MTimerService::QueryInterface(
            /* [in] */SIID siid
    ) {
        return SIID_IM_TIMER_SERVICE == siid;
    }

    /// <summary>
    /// 获取iid
    /// </summary>
    SIID STDMETHODCALLTYPE MTimerService::GetSIID(
            /* [in] */char Data2[16]
    ) {
        for (int i = 0; i < sizeof(SIID_IM_TIMER_SERVICE.Data2); ++i) {
            if (Data2[i] != SIID_IM_TIMER_SERVICE.Data2[i]) {
                return SIID_SERVICE_INVALID;
            }
        }

        return SIID_IM_TIMER_SERVICE;
    }

    /********************************************************
     IMTimer方法实现
    *********************************************************/
    /// <summary>
    /// 更改当前日志配置
    /// </summary>
    void STDMETHODCALLTYPE MTimerService::UpdateConfig(
            /* [in][要更改的配置] */const MString& key,
            /* [in][要更改的配置] */const MString& value
    ) {

    }

    void STDMETHODCALLTYPE MTimerService::Init() {
        m_timerContainer = MakeShared(IMTimerContainer);
        m_timerTask = MakeShared(MTimerTaskHandle);
        StartupTimerQueue();
    }

    void STDMETHODCALLTYPE MTimerService::ClearTimer() {
        m_timerContainer->ClearContainer();
        m_signal.notify_all();
    }

    void STDMETHODCALLTYPE MTimerService::DeleteTimerTask(const MULLong& id) {
        m_timerContainer->DeleteTimerTask(id);
        m_signal.notify_all();
    }

    MULLong STDMETHODCALLTYPE MTimerService::AddTimerTask(const IMTaskHandle::ptr& task, MULLong& id) {
        m_id++;
        id = m_id.load();
        if (!task->GetId()) {
            task->SetId(id);
        }
        m_timerContainer->AddTimerTask(task);

        m_signal.notify_all();

        return id;
    }

    [[noreturn]] void STDMETHODCALLTYPE MTimerService::StartupTimerQueue() {
        while (true) {
            m_timerContainer->Sort();

            std::unique_lock<std::mutex> lock(m_mtx);
            MBoolean flag = true;

            while (flag) {
                if (m_timerContainer->GetSize() == 0) {
                    m_signal.wait(lock);
                } else {
                    IMTaskHandle::ptr h = m_timerContainer->GetExecTask();
                    std::chrono::high_resolution_clock::time_point tp = h->GetFirstExecTime();

                    m_signal.wait_until(lock, tp);

                    auto ct = IMTimerTask::CurrentTime();
                    std::chrono::duration<MDouble, std::milli> ms = ct - tp;

                    if (ms.count() >= 0) {
                        flag = false;
                    }
                }
            }

            IMTaskHandle::ptr h = m_timerContainer->GetExecTask();
            m_timerTask->ExecTask(h);

            //MString ret = IM_CONFIG_GET_ARG("mwd.timer.container.consoleOutput");
            //std::transform<MString::iterator, MString::iterator, int(int)>(ret.begin(), ret.end(), ret.begin(),
            //                                                               std::toupper);
            //if (ret == "TRUE") {
            //    IM_LOG_DEBUG(m_timerContainer->Print(), "IMTimer");
            //}
        }
    }
} // MWD::Service::Timer

/// <summary>
/// 定时器服务siid
/// </summary>
MWD::Service::SIID SIID_IM_TIMER_SERVICE{
        {0,0,0,0,0,0,0,1},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3}
};

/// <summary>
/// dll函数定义
/// </summary>
GM_DEFINE_REGISTER_FUNC_AND_DESTROY(MWD::Service::Timer::MTimerService, SIID_IM_TIMER_SERVICE)


