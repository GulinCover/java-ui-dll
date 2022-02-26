#pragma once

#include "IMFrame.hpp"

namespace MWD::SwapChain {
    using namespace MWD::Frame;

    /********************************************************
     渲染队列类
    *********************************************************/
    class IMSwapQueue {
    public:
        IMSwapQueue() {
            m_frameMap = std::make_shared<Map<MString, std::shared_ptr<IMFrame>>>();
            m_renderSort = std::make_shared<List<MString>>();
        }

    public:
        [[nodiscard]] List<MString>::iterator GetBeginIter() const {
            return m_renderSort->begin();
        }

        [[nodiscard]] List<MString>::iterator GetEndIter() const {
            return m_renderSort->end();
        }

        [[nodiscard]] std::shared_ptr<IMFrame> GetFrame(List<MString>::iterator iter) const {
            if (iter == m_renderSort->end())
                return nullptr;

            auto item = m_frameMap->find(*iter);
            if (item != m_frameMap->end()) {
                return item->second;
            }

            return nullptr;
        }

        void InsertFrame() {

        }

        void AddFrame(const std::shared_ptr<IMFrame>& frame) const {
            m_frameMap->insert(std::make_pair(frame->uuid, frame));
            m_renderSort->push_back(frame->uuid);
        }

        void DeleteFrame(const MString& uuid) const {
            auto item = m_frameMap->find(uuid);
            if (item != m_frameMap->end()) {
                m_frameMap->erase(item);
            }

            for (auto i = m_renderSort->begin(); i != m_renderSort->end(); ++i) {
                if (*i == uuid) {
                    m_renderSort->erase(i);
                    break;
                }
            }
        }

        void UpdateFrame(const std::shared_ptr<IMFrame>& frame) const {
            auto item = m_frameMap->find(frame->uuid);
            if (item != m_frameMap->end()) {
                item->second = frame;
            }
        }

        void ClearFrame() const {
            m_frameMap->clear();
            m_renderSort->clear();
        }

        [[nodiscard]] std::shared_ptr<IMFrame> GetFrame(const MString& uuid) const {
            auto item = m_frameMap->find(uuid);
            if (item != m_frameMap->end()) {
                return item->second;
            }
            return nullptr;
        }

    public:
        std::shared_ptr<Map<MString, std::shared_ptr<IMFrame>>> m_frameMap;
        std::shared_ptr<List<MString>> m_renderSort;
    };
}//MWD::SwapChain
