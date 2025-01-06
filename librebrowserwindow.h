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

#ifndef LIBREBROWSERWINDOW_H
#define LIBREBROWSERWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QWebEnginePage>
#include <QSettings>
#include <QStringList>
#include <QList>
#include <QAction>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QProgressBar;
QT_END_NAMESPACE

class LibreBrowser;
class TabWidget;
class WebView;

class LibreBrowserWindow : public QMainWindow
{
    Q_OBJECT

public:
    LibreBrowserWindow(LibreBrowser *browser, QWebEngineProfile *profile, bool forDevTools = false);
    QSize sizeHint() const override;
    TabWidget *tabWidget() const;
    WebView *currentTab() const;
    LibreBrowser *browser() { return m_browser; }
    void writeToolbarSettings(bool isHidden);
    void writeStatusbarSettings(bool isHidden);
    void writeZoomFactorSettings(double factor);
    bool toolbarIsHidden();
    bool statusbarIsHidden();
    double zoomFactor();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void handleNewWindowTriggered();
    void handleNewIncognitoWindowTriggered();
    void handleFileOpenTriggered();
    void handleFindActionTriggered();
    void addBookmarkTriggered();
    void removeBookmarkTriggered();
    void toggleFullscreen();
#if 0
    void historyTriggered();
#endif
    void handleShowWindowTriggered();
    void handleWebViewLoadProgress(int);
    void handleWebViewTitleChanged(const QString &title);
    void handleWebActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);
    void handleDevToolsRequested(QWebEnginePage *source);
    void handleFindTextFinished(const QWebEngineFindTextResult &result);
    void toolBarVisibilityChanged(bool visible);
    void openBookmark(bool checked = false);

private:
    QMenu *createFileMenu(TabWidget *tabWidget);
    QMenu *createEditMenu();
    QMenu *createBookmarkMenu();
    QMenu *createViewMenu(QToolBar *toolBar);
    QMenu *createWindowMenu(TabWidget *tabWidget);
    QMenu *createHistoryMenu();
    QMenu *createHelpMenu();
    QToolBar *createToolBar();
    void writeSettings();
    void readSettings();
    void updateBookmarkActionList();
    void adjustForCurrentBookmark(const QString &history);
    void updateHistoryActionList();
    void adjustForCurrentHistory(const QString &history);

private:
    LibreBrowser *m_browser;
    QWebEngineProfile *m_profile;
    TabWidget *m_tabWidget;
    QProgressBar *m_progressBar;
    QAction *m_historyBackAction;
    QAction *m_historyForwardAction;
    QAction *m_stopAction;
    QAction *m_reloadAction;
    QAction *m_stopReloadAction;
    QLineEdit *m_urlLineEdit;
    QAction *m_favAction;
    QString m_lastSearch;
    uint m_maxBookmarkNr;
    uint m_maxHistoryNr;
    QList<QAction*> m_recentBookmarkActionList;
    QList<QAction*> m_recentHistoryActionList;
    QString m_currentUrlTitle;
    QAction *m_toggleFullscreen = 0;
};

#endif // LIBREBROWSERWINDOW_H
