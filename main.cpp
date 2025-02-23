/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "librebrowser.h"
#include "librebrowserwindow.h"
#include "tabwidget.h"
#include <QApplication>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QFile>

QUrl commandLineUrlArgument()
{
    const QStringList args = QCoreApplication::arguments();
    for (const QString &arg : args.mid(1)) {
        if (!arg.startsWith(QLatin1Char('-')))
        {
            if (arg == "/" || arg == "/=" || !arg.contains("/"))
            {
                QString url = arg;

                if (arg == "+")
                {
                    url = "addition_operator";
                }
                else if(arg == "-")
                {
                    url = "subtraction_operator";
                }
                else if(arg == "*")
                {
                    url = "multiplication_operator";
                }
                else if(arg == "/")
                {
                    url = "division_operator";
                }
                else if(arg == "=")
                {
                    url = "equal_to_operator";
                }
                else if (arg == "/=")
                {
                    url = "not_equal_to_operator";
                }
                else if(arg == "<")
                {
                    url = "less_than_operator";
                }
                else if(arg == "<=")
                {
                    url = "less_than_or_equal_to_operator";
                }
                else if(arg == ">")
                {
                    url = "greater_than_operator";
                }
                else if(arg == ">=")
                {
                    url = "greater_than_or_equal_to_operator";
                }
                else if(arg == "~")
                {
                    url = "1s_compliment";
                }
                else if(arg == "%")
                {
                    url = "modulo_operator";
                }
                else {}

                if (arg.endsWith('*') || arg.endsWith('!'))
                {
                    url.chop(1);
                }

                if (arg.endsWith('?'))
                {
                    url.chop(1);
                    url += "_q";
                }

                if (QFile::exists(":/html/" + url + ".html"))
                {
                    return QUrl::fromUserInput("qrc:/html/" + url + ".html");
                }
                else if(QFile::exists(":/html/dcl-tiles/" + url + ".html"))
                {
                    return QUrl::fromUserInput("qrc:/html/dcl-tiles/" + url + ".html");
                }
                else if(QFile::exists(":/html/lisp/" + url + ".html"))
                {
                    return QUrl::fromUserInput("qrc:/html/lisp/" + url + ".html");
                }
                else if(QFile::exists(":/html/predefined-attributes/" + url + ".html"))
                {
                    return QUrl::fromUserInput("qrc:/html/predefined-attributes/" + url + ".html");
                }
                else if(QFile::exists(":/html/python/" + url + ".html"))
                {
                    return QUrl::fromUserInput("qrc:/html/python/" + url + ".html");
                }
                else
                {
                    return QUrl(QStringLiteral("qrc:/html/index.html"));
                }
            }

            return QUrl::fromUserInput(arg);
        }
    }
    return QUrl(QStringLiteral("qrc:/html/index.html"));
}

int main(int argc, char **argv)
{
    QCoreApplication::setOrganizationName("LibreCAD");

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":AppLogoColor.png")));

    QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    QWebEngineProfile::defaultProfile()->settings()->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);

    QUrl url = commandLineUrlArgument();

    LibreBrowser browser;
    LibreBrowserWindow *window = browser.createWindow();
    window->tabWidget()->setUrl(url);

    return app.exec();
}
