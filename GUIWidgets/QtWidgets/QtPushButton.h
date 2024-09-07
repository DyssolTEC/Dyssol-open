/* Copyright (c) 2024, DyssolTEC GmbH.
 * All rights reserved. This file is part of Dyssol. See LICENSE file for license information. */

#pragma once

#include <QPushButton>

/**
 * \brief Extends the standard QPushButton.
 * \details If an icon is set, it will be aligned left, while the text will stay center aligned.
 */
class CQtPushButton : public QPushButton
{
public:
    explicit CQtPushButton(QWidget* _parent = nullptr);

    /**
     * \brief Sets new icon.
     * \details Hides the parent function, so should be used with care from own code.
     * But if the QPushButton is promoted to CQtPushButton in Qt Designer, this version will be properly called.
     * \param _icon Icon.
     */
    void setIcon(const QIcon& _icon);
    /**
     * \brief Sets new icon size.
     * \details Hides the parent function, so should be used with care from own code.
     * But if the QPushButton is promoted to CQtPushButton in Qt Designer, this version will be properly called.
     * \param _size
     */
    void setIconSize(const QSize& _size);

    [[nodiscard]] QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* _event) override;

private:
    static constexpr int c_margin{ 5 };   /// Margin for icons.

    QPixmap m_pixmap{}; /// Stores a scaled pixmap of the icon to be drawn.
    QIcon m_icon{};     /// Current icon; it has to be stored for the case of the setIconSize event, to be able to rescale the pixmap without loosing picture quality.
};

