#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>

#include "aboutTab.h"

AboutTab::AboutTab(QWidget *parent) : QWidget(parent) {
    auto mainLayout = new QVBoxLayout();

    auto group = new QGroupBox("License");
    auto layout = new QFormLayout();
    group->setLayout(layout);

    auto license = new QLabel(R"(MIT License

Copyright (c) 2020 winderica

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.)");
    QFont font("Consolas");
    font.setStyleHint(QFont::Monospace);
    font.setPixelSize(16);
    license->setFont(font);
    layout->addRow(license);

    mainLayout->addWidget(group);
    mainLayout->addStretch(1);

    setLayout(mainLayout);
}

AboutTab::~AboutTab() = default;
