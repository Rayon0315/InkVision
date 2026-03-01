#include "NavButton.h"

NavButton::NavButton(QWidget *parent)
    : QPushButton(parent)
{
    setMinimumHeight(60);
    setCursor(Qt::PointingHandCursor);

    setStyleSheet(
        "QPushButton {"
        "background-color: #2C313C;"
        "color: #E0E6ED;"
        "border: none;"
        "border-radius: 14px;"
        "padding: 10px 16px;"
        "font-size: 15px;"
        "}"
        "QPushButton:hover {"
        "background-color: #3A3F4B;"
        "}"
        "QPushButton:pressed {"
        "background-color: #242833;"
        "}"
        );
}
