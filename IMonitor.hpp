#pragma once

#include <utility>

#include "IMFrame.hpp"

namespace MWD::Monitor {
    using namespace MWD::Frame;
    using namespace MWD::GM;
    /********************************************************
     监听事件
    *********************************************************/
    enum IM_MONITOR_EVENT {
        /// <summary>
        /// 鼠标左键事件
        /// </summary>
        MOUSE_LEFT_BUTTON_DOWN = 1,
        MOUSE_LEFT_BUTTON_UP = 2,
        MOUSE_LEFT_BUTTON_DOUBLE = 4,

        /// <summary>
        /// 鼠标右键事件
        /// </summary>
        MOUSE_RIGHT_BUTTON_DOWN = 8,
        MOUSE_RIGHT_BUTTON_UP = 16,
        MOUSE_RIGHT_BUTTON_DOUBLE = 32,

        /// <summary>
        /// 鼠标中键事件
        /// </summary>
        MOUSE_MIDDLE_BUTTON_DOWN = 64,
        MOUSE_MIDDLE_BUTTON_UP = 128,

        /// <summary>
        /// 鼠标滚轮事件
        /// </summary>
        MOUSE_WHEEL_FRONT = 256,
        MOUSE_WHEEL_AFTER = 512,

        /// <summary>
        /// 键盘事件
        /// </summary>
        KEYBOARD_KEY_DOWN = 1024,
        KEYBOARD_KEY_UP = 2048,

        /// <summary>
        /// 键盘字母键事件
        /// </summary>
        KEYBOARD_ALPHA_KEY = 4096,

        /// <summary>
        /// 鼠标移动事件
        /// </summary>
        MOUSE_MOVE = 8192,

        /// <summary>
        /// 鼠标进入离开事件
        /// </summary>
        MOUSE_ENTER = 16384,
        MOUSE_LEAVE = 32768,

        /// <summary>
        /// 输入法事件
        /// </summary>
        KEYBOARD_IME_KEY = 65536,
    };

    /********************************************************
     鼠标进入离开事件初始必传参数
    *********************************************************/
    class IMonitorEvent {
    public:
        // frame信息
        std::shared_ptr<IMFrame> frame;

        // 鼠标是否能触发进入事件
        MBoolean isEnter = true;

        HWND hwnd;

        // 扩展
        void* pointer;
    };

    /********************************************************
     监听队列
    *********************************************************/
    class IMonitorQueue {
    public:
        virtual ~IMonitorQueue() = default;

    public:
        static MInt ToInt(IM_MONITOR_EVENT e) {
            return static_cast<MInt>(e);
        }

        static MString ToCompleteName(MString uuid, IM_MONITOR_EVENT e, MInt id) {
            uuid.append(":").append(std::to_string(static_cast<MInt>(e))).append(":").append(std::to_string(id));
            return uuid;
        }

    public:
        virtual void
        AddListenEvent(const MString& uuid, IM_MONITOR_EVENT e,
                       const std::shared_ptr<std::function<void(std::shared_ptr<IMonitorEvent>, void*)>>& func,
                       std::shared_ptr<IMonitorEvent> args, MInt) = 0;

        virtual void DeleteListenEvent(const MString& uuid, IM_MONITOR_EVENT e, MInt) = 0;

        virtual void Exec(const MString& uuid, IM_MONITOR_EVENT e, void*, MInt) = 0;
    };

    /********************************************************
     队列类型
    *********************************************************/
    enum IM_MONITOR_QUEUE_TYPE {
        SYNC_QUEUE = 1,
        ASYNC_QUEUE = 2,
    };

    /********************************************************
     同步队列
    *********************************************************/
    class IMonitorSyncQueue : public IMonitorQueue {
    public:
        IMonitorSyncQueue() {
            m_function = std::make_shared<Map<MString, std::shared_ptr<std::function<void(std::shared_ptr<IMonitorEvent>, void*)>>>>();
            m_args = std::make_shared<Map<MString, std::shared_ptr<IMonitorEvent>>>();
        }

        ~IMonitorSyncQueue() override {
            auto item = m_args->begin();
            for (; item != m_args->end(); ++item) {
                if (item->second) {
                    item->second = nullptr;
                }
                m_args->erase(item);
            }
        }

    public:
        void AddListenEvent(
                const MString& uuid, IM_MONITOR_EVENT e,
                const std::shared_ptr<std::function<void(std::shared_ptr<IMonitorEvent>, void*)>>& func,
                std::shared_ptr<IMonitorEvent> args, MInt id) override {
            MString iid = IMonitorQueue::ToCompleteName(uuid, e, id);

            auto item = m_function->find(iid);
            if (item != m_function->end()) {
                m_function->erase(item);
            }

            auto iter = m_args->find(iid);
            if (iter != m_args->end()) {
                m_args->erase(iter);
            }

            m_function->insert(std::make_pair(iid, func));
            m_args->insert(std::make_pair(iid, args));
        }

        void DeleteListenEvent(const MString& uuid, IM_MONITOR_EVENT e, MInt id) override {
            MString iid = IMonitorQueue::ToCompleteName(uuid, e, id);
            auto item = m_function->find(iid);
            if (item != m_function->end()) {
                m_function->erase(item);
            }

            auto iter = m_args->find(iid);
            if (iter != m_args->end()) {
                if (iter->second) {
                    iter->second = nullptr;
                }
                m_args->erase(iter);
            }
        }

        void Exec(const MString& uuid, IM_MONITOR_EVENT e, void* args, MInt id) override {
            if (id == -1) {
                auto item = m_function->begin();
                for (; item != m_function->end(); ++item) {
                    auto splitVec = IMTools::StringSplit(item->first, ':');

                    MString key = splitVec->at(0) + ":" + splitVec->at(1);
                    MString name;
                    name.append(uuid).append(":").append(std::to_string(static_cast<MInt>(e)));
                    if (key == name) {
                        auto iter = m_args->find(item->first);
                        if (iter != m_args->end()) {
                            if (e == MOUSE_ENTER) {
                                std::shared_ptr<IMonitorEvent> p = iter->second;
                                if (p->isEnter) {
                                    (*item->second)(iter->second, args);
                                    p->isEnter = false;
                                }

                            } else if (e == MOUSE_LEAVE) {
                                std::shared_ptr<IMonitorEvent> p = iter->second;
                                if (!p->isEnter) {
                                    (*item->second)(iter->second, args);
                                    p->isEnter = true;
                                }
                            } else {
                                (*item->second)(iter->second, args);
                            }
                        } else {
                            (*item->second)(nullptr, args);
                        }

                    }
                }
                return;
            }

            MString iid = IMonitorQueue::ToCompleteName(uuid, e, id);
            auto item = m_function->find(iid);
            if (item == m_function->end()) return;

            auto iter = m_args->find(iid);
            if (iter != m_args->end()) {
                if (e == MOUSE_ENTER) {
                    std::shared_ptr<IMonitorEvent> p = iter->second;
                    if (p->isEnter) {
                        (*item->second)(iter->second, args);
                        p->isEnter = false;
                    }

                } else if (e == MOUSE_LEAVE) {
                    std::shared_ptr<IMonitorEvent> p = iter->second;
                    if (!p->isEnter) {
                        (*item->second)(iter->second, args);
                        p->isEnter = true;
                    }
                } else {
                    (*item->second)(iter->second, args);
                }
            } else {
                (*item->second)(nullptr, args);
            }
        }

    private:
        std::shared_ptr<Map<MString, std::shared_ptr<std::function<void(std::shared_ptr<IMonitorEvent>,
                                                                        void*)>>>> m_function;
        std::shared_ptr<Map<MString, std::shared_ptr<IMonitorEvent>>> m_args;
    };

    /********************************************************
     异步回调队列
    *********************************************************/
    class IMonitorAsyncQueue : public IMonitorQueue {
    public:
    };

    /********************************************************
     范围监听器
    *********************************************************/
    class IMBlockListener {
    public:
        virtual void Init(MLong, MLong) = 0;

        virtual void AddListen(const std::shared_ptr<IMFrame>&, MInt) = 0;

        virtual void DeleteListen(const std::shared_ptr<IMFrame>&) = 0;

        virtual std::shared_ptr<IMFrame> ExecListen(MLong, MLong) = 0;

        virtual std::shared_ptr<List<MString>> ExecListenNon(MLong, MLong) = 0;
    };

    /********************************************************
     范围监听器impl
    *********************************************************/
    class IMDefaultBlockListener : public IMBlockListener {
    public:
        IMDefaultBlockListener() {
            m_block = std::make_shared<Map<MString, std::shared_ptr<Vector<std::shared_ptr<IMFrame>>>>>();
        }

    public:
        void AddListen(const std::shared_ptr<IMFrame>& frame, MInt id) override {
            MLong min_x = std::floor(frame->innerX / 100.0f);
            MLong min_y = std::floor(frame->innerY / 100.0f);
            MLong max_x = std::ceil((frame->innerX + frame->width) / 100.0f);
            MLong max_y = std::ceil((frame->innerY + frame->height) / 100.0f);

            MBoolean has;

            for (int i = min_x; i <= max_x; ++i) {
                for (int j = min_y; j <= max_y; ++j) {
                    MString key;
                    key.append(std::to_string(i)).append(":").append(std::to_string(j));
                    auto item = m_block->find(key);
                    if (item != m_block->end()) {
                        has = false;
                        for (const auto& k : *item->second) {
                            if (k->uuid == frame->uuid) {
                                has = true;
                                break;
                            }
                        }

                        if (!has) {
                            item->second->push_back(frame);
                        }
                    }
                }
            }

            has ? ++(frame->ref) : frame->ref = 1;
        }

        void DeleteListen(const std::shared_ptr<IMFrame>& frame) override {
            auto item = m_block->begin();
            MBoolean isRemover = false;
            for (; item != m_block->end(); ++item) {
                auto iter = item->second->begin();
                for (; iter != item->second->end(); ++iter) {
                    if ((*iter)->uuid == frame->uuid) {
                        if ((*iter)->ref == 1) {
                            item->second->erase(iter);
                            isRemover = true;
                        } else {
                            isRemover = false;
                        }
                        break;
                    }
                }
            }

            isRemover ? frame->ref = 0 : (--(frame->ref) >= 0 ? 0 : frame->ref = 0);
        }

        void Init(MLong w, MLong h) override {
            MLong xn = std::ceil(w / 100.0f);
            MLong yn = std::ceil(h / 100.0f);

            for (int i = 0; i < xn; ++i) {
                for (int j = 0; j < yn; ++j) {
                    MString key;
                    key.append(std::to_string(i)).append(":").append(std::to_string(j));
                    std::shared_ptr<Vector<std::shared_ptr<IMFrame>>> vec = std::make_shared<Vector<std::shared_ptr<IMFrame>>>();
                    m_block->insert(std::make_pair(key, vec));
                }
            }
        }

        std::shared_ptr<IMFrame> ExecListen(MLong x, MLong y) override {
            MLong rx = std::floor(x / 100.0f);
            MLong ry = std::floor(y / 100.0f);
            MString key;
            key.append(std::to_string(rx)).append(":").append(std::to_string(ry));

            auto item = m_block->find(key);
            if (item != m_block->end()) {
                auto iter = item->second->begin();
                std::shared_ptr<IMFrame> maxIndex = std::make_shared<IMFrame>();
                for (; iter != item->second->end(); ++iter) {
                    MLong ix = (*iter)->innerX;
                    MLong iy = (*iter)->innerY;
                    MLong ox = (*iter)->offX;
                    MLong oy = (*iter)->offY;
                    MLong w = (*iter)->width;
                    MLong h = (*iter)->height;
                    MLong pt = (*iter)->paddingTop;
                    MLong pr = (*iter)->paddingRight;
                    MLong pb = (*iter)->paddingBottom;
                    MLong pl = (*iter)->paddingLeft;

                    if ((x >= ix + ox && x <= ix + ox + w + pl + pr) && (y >= iy + oy && y <= iy + oy + h + pt + pb)) {
                        if ((*iter)->zIndex >= maxIndex->zIndex) {
                            maxIndex = *iter;
                        }
                    }
                }

                return maxIndex;
            }

            return nullptr;
        }

        std::shared_ptr<List<MString>> ExecListenNon(MLong x, MLong y) override {
            std::shared_ptr<List<MString>> lst = std::make_shared<List<MString>>();
            MLong rx = std::floor(x / 100.0f);
            MLong ry = std::floor(y / 100.0f);
            MString key;
            key.append(std::to_string(rx)).append(":").append(std::to_string(ry));

            auto item = m_block->find(key);
            if (item != m_block->end()) {
                auto iter = item->second->begin();

                for (; iter != item->second->end(); ++iter) {
                    MLong ix = (*iter)->innerX;
                    MLong iy = (*iter)->innerY;
                    MLong ox = (*iter)->offX;
                    MLong oy = (*iter)->offY;
                    MLong w = (*iter)->width;
                    MLong h = (*iter)->height;

                    if ((x < ix + ox || x > ix + ox + w) || (y < iy + oy || y > iy + oy + h)) {
                        lst->push_back((*iter)->uuid);
                    }
                }

            }

            return lst;
        }

    private:
        std::shared_ptr<Map<MString, std::shared_ptr<Vector<std::shared_ptr<IMFrame>>>>> m_block;
    };

    /********************************************************
     事件监听器
    *********************************************************/
    class IMonitor {
    public:
        IMonitor() {
            m_queue = std::make_shared<Map<IM_MONITOR_QUEUE_TYPE, std::shared_ptr<IMonitorQueue>>>();
            m_isInit = true;
            m_listener = std::make_shared<IMDefaultBlockListener>();
        }

    public:
        void AddTask(const std::shared_ptr<IMFrame>& frame, IM_MONITOR_EVENT e, IM_MONITOR_QUEUE_TYPE t,
                     const std::shared_ptr<std::function<void(std::shared_ptr<IMonitorEvent>, void*)>>& f,
                     std::shared_ptr<IMonitorEvent> args, MInt id) {
            auto item = m_queue->find(t);
            if (item != m_queue->end()) {
                item->second->AddListenEvent(frame->uuid, e, f, std::move(args), id);
            }

            frame->event |= e;
            m_listener->AddListen(frame, id);
        }

        void Init(HWND hwnd) {
            RECT rect;
            GetWindowRect(hwnd, &rect);

            m_listener->Init(rect.right - rect.left, rect.bottom - rect.top);

            std::shared_ptr<IMonitorSyncQueue> syncQueue = std::make_shared<IMonitorSyncQueue>();
            m_queue->insert(std::make_pair(SYNC_QUEUE, syncQueue));

            m_isInit = false;
        }

        [[nodiscard]] MBoolean IsInit() const {
            return m_isInit;
        }

        void LoopExec(HWND hwnd, IM_MONITOR_EVENT e, void* args) {
            RECT rect;
            GetWindowRect(hwnd, &rect);
            POINT point;
            GetCursorPos(&point);

            MLong x = point.x - rect.left;
            MLong y = point.y - rect.top;

            if (e == MOUSE_LEAVE) {
                auto lst = m_listener->ExecListenNon(x, y);

                for (const MString& uuid : *lst) {
                    auto item = m_queue->begin();
                    for (; item != m_queue->end(); ++item) {
                        item->second->Exec(uuid, e, args, -1);
                    }
                }
                return;
            }

            std::shared_ptr<IMFrame> f = m_listener->ExecListen(x, y);

            auto item = m_queue->begin();
            for (; item != m_queue->end(); ++item) {
                if (f && f->event & e) {
                    item->second->Exec(f->uuid, e, args, -1);
                }
            }
        }

    private:
        std::shared_ptr<Map<IM_MONITOR_QUEUE_TYPE, std::shared_ptr<IMonitorQueue>>> m_queue;
        std::shared_ptr<IMBlockListener> m_listener;
        MBoolean m_isInit;
    };
}
