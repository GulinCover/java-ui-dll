#pragma once

#include "IMFrame.hpp"

namespace MWD::MainWindow {
    using namespace MWD::Frame;
    using namespace MWD::GM;

    /********************************************************
     全局事件监听
    *********************************************************/
    class IMMainWindowEvent {
    private:
        static IMPoint CalcCaretPos(const std::shared_ptr<IMFrame>& frame, IMPoint pos, MBoolean usePos = false,
                                    MBoolean updateInsert = true) {
            if (frame->font->isSingleRow)
                frame->textLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

            MDouble realX = 0;
            MDouble realY = 0;
            MDouble baseX = frame->innerX + frame->paddingLeft + frame->offX + frame->textOffX;
            MDouble baseY = frame->innerY + frame->paddingTop + frame->offY + frame->textOffY;
            DWRITE_HIT_TEST_METRICS hitTestMetrics{};
            if (usePos) {
                BOOL isTrailingHit;
                BOOL isInside;
                frame->textLayout->HitTestPoint(
                        pos.x - frame->innerX - frame->paddingLeft - frame->offX - frame->textOffX,
                        pos.y - frame->innerY - frame->paddingTop - frame->offY - frame->textOffY,
                        &isTrailingHit,
                        &isInside,
                        &hitTestMetrics
                );

                if (isInside) {
                    if (pos.x > baseX + hitTestMetrics.left + hitTestMetrics.width / 2.0f) {
                        realX = baseX + hitTestMetrics.left + hitTestMetrics.width;
                        realY = baseY + hitTestMetrics.top;

                        if (updateInsert) {
                            if (hitTestMetrics.textPosition + 1 >= frame->font->text->size()) {
                                frame->font->insertStringPos = frame->font->text->size();
                            } else {
                                frame->font->insertStringPos = hitTestMetrics.textPosition + 1;
                            }
                        }
                    } else {
                        realX = baseX + hitTestMetrics.left;
                        realY = baseY + hitTestMetrics.top;

                        if (updateInsert) {
                            if (hitTestMetrics.textPosition <= 0) {
                                frame->font->insertStringPos = 0;
                            } else {
                                frame->font->insertStringPos = hitTestMetrics.textPosition;
                            }
                        }
                    }
                } else {
                    if (pos.x > frame->innerX + frame->paddingLeft + frame->offX) {
                        realX = baseX + hitTestMetrics.left + hitTestMetrics.width;
                        realY = baseY + hitTestMetrics.top;

                        if (updateInsert)
                            frame->font->insertStringPos = frame->font->text->size();
                    } else {
                        realX = baseX + hitTestMetrics.left;
                        realY = baseY + hitTestMetrics.top;

                        if (updateInsert)
                            frame->font->insertStringPos = 0;
                    }
                }
            } else {
                FLOAT x;
                FLOAT y;

                if (frame->font->insertStringPos >= frame->font->text->size()) {
                    frame->textLayout->HitTestTextPosition(
                            frame->font->text->size() - 1,
                            true,
                            &x,
                            &y,
                            &hitTestMetrics
                    );

                    realX = baseX + x;
                    realY = baseY + y;
                } else if (frame->font->insertStringPos <= 0) {
                    frame->textLayout->HitTestTextPosition(
                            0,
                            true,
                            &x,
                            &y,
                            &hitTestMetrics
                    );

                    realX = baseX + x - hitTestMetrics.width;
                    realY = baseY + y;
                } else {
                    frame->textLayout->HitTestTextPosition(
                            frame->font->insertStringPos,
                            true,
                            &x,
                            &y,
                            &hitTestMetrics
                    );

                    realX = baseX + x - hitTestMetrics.width;
                    realY = baseY + y;
                }
            }

            IMPoint p{};
            p.x = realX;
            p.y = realY;

            return p;
        }

        static void CalcTextWidth(const std::shared_ptr<IMFrame>& frame) {
            if (frame->font->isSingleRow)
                frame->textLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

            DWRITE_TEXT_METRICS textMetrics{};
            frame->textLayout->GetMetrics(&textMetrics);

            frame->font->textWidth = textMetrics.widthIncludingTrailingWhitespace;
            frame->font->textHeight = textMetrics.height;
        }

        static void UpdateTextInfo(const std::shared_ptr<IMFrame>& frame) {
            CComPtr<IDWriteTextLayout> textLayout{};
            frame->writeFactory->CreateTextLayout(
                    frame->font->GetText().c_str(),
                    frame->font->GetText().length(),
                    frame->textFormat,
                    frame->width,
                    frame->height,
                    &textLayout
            );
            frame->textLayout = textLayout;
            CalcTextWidth(frame);
        }

    public:
        virtual void SetRoute(MString route) {
            m_route = std::move(route);
        }

        virtual void ExecMainClickDownEvent(POINT point) {
            auto focusManager = IMFocusManager::GetInstance();
            auto frame = focusManager->GetFocusFrame(m_route);

            if (frame && frame->isInput) {
                IMPoint imPoint{
                        (MDouble) point.x,
                        (MDouble) point.y,
                };

                // 清空选中的文本
                frame->font->textSelectedRect->clear();

                imPoint = CalcCaretPos(frame, imPoint, true);
                frame->font->textSelectedStartPos = frame->font->insertStringPos;
                frame->caretPosX = imPoint.x;
                frame->caretPosY = imPoint.y;

                focusManager->SetFocusFrame(m_route, frame);
                SetCaretPos(frame->caretPosX, frame->caretPosY + frame->font->fontSize / 2);

                frame->font->primaryText.clear();
            }

            if (frame) {
                frame->isClkDown = true;
                frame->clkDownPoint = {
                        (MDouble) point.x,
                        (MDouble) point.y,
                };
            }
        }

        virtual void ExecMainClickUpEvent(POINT point) {
            auto frame = IMFocusManager::GetInstance()->GetFocusFrame(m_route);

            if (frame && frame->isInput) {
                IMPoint imPoint{
                        (MDouble) point.x,
                        (MDouble) point.y,
                };

                imPoint = CalcCaretPos(frame, imPoint, true);
                frame->font->textSelectedEndPos = frame->font->insertStringPos;
                frame->caretPosX = imPoint.x;
                frame->caretPosY = imPoint.y;

                if (!frame->font->textSelectedRect->empty()) {
                    if (frame->font->textSelectedEndPos < frame->font->textSelectedStartPos) {
                        MLong temp = frame->font->textSelectedEndPos;
                        frame->font->textSelectedEndPos = frame->font->textSelectedStartPos;
                        frame->font->textSelectedStartPos = temp;
                    }
                } else {
                    frame->font->textSelectedEndPos = frame->font->insertStringPos;
                }
            }

            if (frame) {
                frame->isClkDown = false;
                frame->clkUpPoint = {
                        (MDouble) point.x,
                        (MDouble) point.y
                };


            }
        }

        virtual void ExecMainMoveEvent(POINT point) {
            auto frame = IMFocusManager::GetInstance()->GetFocusFrame(m_route);

            if (frame && frame->isInput && frame->isClkDown) {
                IMPoint imPoint{
                        (MDouble) point.x,
                        (MDouble) point.y
                };

                IMPoint realPoint = CalcCaretPos(frame, imPoint, true);

                // 单、多行框选
                if (frame->font->isSingleRow) {
                    if (frame->font->textSelectedRect->empty()) {
                        D2D1_RECT_F rect;
                        if (frame->clkDownPoint.x > realPoint.x) {
                            rect.left = realPoint.x;
                            rect.top = realPoint.y;
                            rect.bottom = frame->font->fontSize * 1.35 + realPoint.y;
                            rect.right = frame->caretPosX;
                        } else {
                            rect.left = frame->caretPosX;
                            rect.top = frame->caretPosY;
                            rect.bottom = frame->font->fontSize * 1.35 + realPoint.y;
                            rect.right = realPoint.x;
                        }

                        frame->font->textSelectedRect->push_back(rect);
                    } else {
                        while (imPoint.x < frame->innerX + frame->offX + frame->paddingLeft && frame->textOffX < 0) {
                            frame->textOffX += frame->font->fontSize / 12;
                            realPoint = CalcCaretPos(frame, imPoint, true);
                        }

                        while (imPoint.x > frame->innerX + frame->offX + frame->paddingLeft + frame->width &&
                               frame->font->textWidth > frame->width - frame->textOffX) {
                            frame->textOffX -= frame->font->fontSize / 12;
                            realPoint = CalcCaretPos(frame, imPoint, true);
                        }

                        D2D1_RECT_F rect = frame->font->textSelectedRect->front();
                        if (frame->clkDownPoint.x > realPoint.x) {
                            rect.left = realPoint.x;
                            rect.top = realPoint.y;
                            rect.bottom = frame->font->fontSize * 1.35 + realPoint.y;
                            rect.right = frame->caretPosX;
                        } else {
                            rect.left = frame->caretPosX;
                            rect.top = frame->caretPosY;
                            rect.bottom = frame->font->fontSize * 1.35 + realPoint.y;
                            rect.right = realPoint.x;
                        }

                        frame->font->textSelectedRect->clear();
                        frame->font->textSelectedRect->push_back(rect);
                    }
                } else {

                }
            }
        }

        virtual void ExecMainIMEEvent(MUInt* ch) {
            auto frame = IMFocusManager::GetInstance()->GetFocusFrame(m_route);

            if (frame && !frame->isClkDown) {
                auto c = (MUInt)*ch;
                Vector<MUChar> vec;
                vec.push_back((MUChar) (c >> 8));
                vec.push_back((MUChar) c);
                vec.push_back('\0');
                auto* ss = (MChar*) &(vec[0]);

                MWString strUtf18 = IMTools::GBKToUTF16(ss);

                frame->font->text->emplace_back(strUtf18[0]);
                std::cout << ss << std::endl;
                ++(frame->font->insertStringPos);

                UpdateTextInfo(frame);

                if (frame->font->isSingleRow) {
                    IMPoint p{};
                    p = CalcCaretPos(frame, p, false);

                    while (p.x > frame->innerX + frame->paddingLeft + frame->paddingRight + frame->width + frame->offX -
                                 frame->font->fontSize / 4) {
                        frame->textOffX -= frame->font->fontSize / 12;

                        p = CalcCaretPos(frame, p, true, false);
                    }

                    SetCaretPos(p.x, p.y + frame->font->fontSize / 2);
                    frame->caretPosX = p.x;
                    frame->caretPosY = p.y;
                } else {

                }
            }
        }

        virtual void ExecMainKeyDownEvent(MChar c) {
            auto frame = IMFocusManager::GetInstance()->GetFocusFrame(m_route);

            if (frame && !frame->isClkDown) {
                MUInt vk = (MUInt) (MInt) c;

                switch (vk) {
                    case VK_LEFT: {
                        frame->font->textSelectedStartPos = 0;
                        frame->font->textSelectedEndPos = 0;
                        frame->font->textSelectedRect->clear();

                        --(frame->font->insertStringPos);
                        if (frame->font->insertStringPos < 0)
                            frame->font->insertStringPos = 0;


                        if (frame->font->isSingleRow) {
                            IMPoint p{};
                            p = CalcCaretPos(frame, p, false);

                            while (p.x < frame->innerX + frame->paddingLeft + frame->offX) {
                                frame->textOffX += frame->font->fontSize / 12;

                                p = CalcCaretPos(frame, p, true, false);
                            }

                            SetCaretPos(p.x, p.y + frame->font->fontSize / 2);
                            frame->caretPosX = p.x;
                            frame->caretPosY = p.y;
                        } else {

                        }

                    }
                        break;
                    case VK_RIGHT: {
                        frame->font->textSelectedStartPos = 0;
                        frame->font->textSelectedEndPos = 0;
                        frame->font->textSelectedRect->clear();

                        frame->font->insertStringPos = max(
                                min(frame->font->insertStringPos + 1,
                                    frame->font->text->size()), 0);

                        if (frame->font->isSingleRow) {
                            IMPoint p{};
                            p = CalcCaretPos(frame, p, false);

                            while (p.x >
                                   frame->innerX + frame->paddingLeft + frame->paddingRight + frame->width +
                                   frame->offX -
                                   frame->font->fontSize / 4) {
                                frame->textOffX -= frame->font->fontSize / 12;

                                p = CalcCaretPos(frame, p, true, false);
                            }

                            SetCaretPos(p.x, p.y + frame->font->fontSize / 2);
                            frame->caretPosX = p.x;
                            frame->caretPosY = p.y;
                        } else {

                        }
                    }
                        break;
                    case VK_DELETE: {
                        if (frame->font->textSelectedRect->empty()) {
                            if (frame->font->insertStringPos <= 0 &&
                                !frame->font->text->empty()) {
                                frame->font->text->erase(frame->font->text->begin());
                            } else if (frame->font->insertStringPos > 0 &&
                                       frame->font->insertStringPos <
                                       frame->font->text->size()) {
                                auto b = frame->font->text->begin();
                                auto e = frame->font->text->end();

                                MInt i = 0;
                                for (; b != e; ++b, ++i) {
                                    if (i == frame->font->insertStringPos) {
                                        frame->font->text->erase(b);
                                        break;
                                    }
                                }
                            }

                        } else if (frame->font->textSelectedStartPos !=
                                   frame->font->textSelectedEndPos) {
                            auto b = frame->font->text->begin();
                            auto e = frame->font->text->end();

                            MInt i = 0;
                            Vector<MWChar>::iterator rb;
                            Vector<MWChar>::iterator re;
                            for (; b != e; ++b, ++i) {
                                if (i == frame->font->textSelectedStartPos) {
                                    rb = b;
                                } else if (i == frame->font->textSelectedEndPos - 1) {
                                    re = b;
                                    break;
                                }
                            }

                            frame->font->text->erase(rb, ++re);
                            IMPoint point{};

                            frame->font->insertStringPos = frame->font->textSelectedStartPos;
                            point = CalcCaretPos(frame, point, false);

                            frame->caretPosX = point.x;
                            frame->caretPosY = point.y;

                            frame->font->textSelectedStartPos = 0;
                            frame->font->textSelectedEndPos = 0;

                            frame->font->textSelectedRect->clear();
                        }

                        UpdateTextInfo(frame);
                    }
                        break;
                    default:
                        break;
                }
            }
        }

        virtual void ExecMainCharDownEvent(MChar c) {
            auto frame = IMFocusManager::GetInstance()->GetFocusFrame(m_route);

            if (frame && frame->isInput && !frame->isClkDown) {
                MUInt vk = (MUInt)(MInt)c;

                switch (vk) {
                    case VK_BACK: {
                        if (frame->font->textSelectedRect->empty()) {
                            if (frame->font->text->size() == frame->font->insertStringPos &&
                                !frame->font->text->empty()) {
                                --(frame->font->insertStringPos);
                                frame->font->text->erase(--(frame->font->text->end()));

                                if (frame->font->isSingleRow) {
                                    IMPoint p{};
                                    p = CalcCaretPos(frame, p, false);

                                    while (p.x < frame->innerX + frame->paddingLeft + frame->offX) {
                                        frame->textOffX += frame->font->fontSize / 12;
                                        p = CalcCaretPos(frame, p, true, false);
                                    }

                                    SetCaretPos(p.x, p.y + frame->font->fontSize / 2);
                                    frame->caretPosX = p.x;
                                    frame->caretPosY = p.y;
                                } else {

                                }
                            } else if (frame->font->insertStringPos > 0 &&
                                       frame->font->insertStringPos <
                                       frame->font->text->size() &&
                                       !frame->font->text->empty()) {
                                auto b = frame->font->text->begin();
                                auto e = frame->font->text->end();

                                MInt i = 0;
                                for (; b != e; ++b, ++i) {
                                    if (i == frame->font->insertStringPos - 1) {
                                        frame->font->text->erase(b);
                                        break;
                                    }
                                }

                                --(frame->font->insertStringPos);

                                if (frame->font->isSingleRow) {
                                    IMPoint p{};
                                    p = CalcCaretPos(frame, p, false);

                                    while (p.x < frame->innerX + frame->paddingLeft + frame->offX) {
                                        frame->textOffX += frame->font->fontSize / 12;

                                        p = CalcCaretPos(frame, p, true, false);
                                    }

                                    SetCaretPos(p.x, p.y + frame->font->fontSize / 2);
                                    frame->caretPosX = p.x;
                                    frame->caretPosY = p.y;
                                } else {

                                }
                            }
                        } else if (frame->font->textSelectedStartPos !=
                                   frame->font->textSelectedEndPos) {
                            auto b = frame->font->text->begin();
                            auto e = frame->font->text->end();

                            MInt i = 0;
                            Vector<MWChar>::iterator rb;
                            Vector<MWChar>::iterator re;
                            for (; b != e; ++b, ++i) {
                                if (i == frame->font->textSelectedStartPos) {
                                    rb = b;
                                } else if (i == frame->font->textSelectedEndPos - 1) {
                                    re = b;
                                    break;
                                }
                            }

                            frame->font->text->erase(rb, ++re);
                            IMPoint point{};

                            frame->font->insertStringPos = frame->font->textSelectedStartPos;
                            point = CalcCaretPos(frame, point, false);

                            frame->caretPosX = point.x;
                            frame->caretPosY = point.y;
                            SetCaretPos(point.x, point.y + frame->font->fontSize / 2);

                            frame->font->textSelectedStartPos = 0;
                            frame->font->textSelectedEndPos = 0;

                            frame->font->textSelectedRect->clear();
                        }

                        UpdateTextInfo(frame);
                    }
                        break;
                    default:
                        break;
                }

                // 接收字母
                if (vk >= 32 && vk <= 126) {
                    MWChar w = (MWChar)c;

                    auto b = frame->font->text->begin();
                    auto e = frame->font->text->end();
                    int i = 0;

                    if (frame->font->insertStringPos == frame->font->text->size()) {
                        frame->font->text->emplace_back(w);
                    } else {
                        for (; b != e; ++b, ++i) {
                            if (i == frame->font->insertStringPos) {
                                frame->font->text->insert(b, w);
                                break;
                            }
                        }
                    }

                    ++(frame->font->insertStringPos);

                    UpdateTextInfo(frame);

                    if (frame->font->isSingleRow) {
                        IMPoint p{};
                        p = CalcCaretPos(frame, p, false);

                        while (p.x >
                               frame->innerX + frame->paddingLeft + frame->paddingRight + frame->width + frame->offX -
                               frame->font->fontSize / 4) {
                            frame->textOffX -= frame->font->fontSize / 12;

                            p = CalcCaretPos(frame, p, true, false);
                        }

                        SetCaretPos(p.x, p.y + frame->font->fontSize / 2);
                        frame->caretPosX = p.x;
                        frame->caretPosY = p.y;
                    } else {

                    }
                }
            }
        }

    private:
        MString m_route;
    };
} //MWD::MainWindow
