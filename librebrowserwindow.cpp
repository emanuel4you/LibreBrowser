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
#include "bookmark.h"
#include "downloadmanagerwidget.h"
#include "tabwidget.h"
#include "webview.h"
#include <QApplication>
#include <QCloseEvent>
#include <QEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QScreen>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWebEngineFindTextResult>
#include <QWebEngineProfile>
#include <QFileInfo>
#include <QDateTime>

LibreBrowserWindow::LibreBrowserWindow(LibreBrowser *browser, QWebEngineProfile *profile, bool forDevTools)
    : m_browser(browser)
    , m_profile(profile)
    , m_tabWidget(new TabWidget(profile, this))
    , m_progressBar(nullptr)
    , m_historyBackAction(nullptr)
    , m_historyForwardAction(nullptr)
    , m_stopAction(nullptr)
    , m_reloadAction(nullptr)
    , m_stopReloadAction(nullptr)
    , m_urlLineEdit(nullptr)
    , m_favAction(nullptr)
    , m_maxBookmarkNr(100)
    , m_maxHistoryNr(100)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setFocusPolicy(Qt::ClickFocus);

    if (!forDevTools) {
        m_progressBar = new QProgressBar(this);

        QToolBar *toolbar = createToolBar();
        addToolBar(toolbar);
        menuBar()->addMenu(createFileMenu(m_tabWidget));
        menuBar()->addMenu(createEditMenu());
        menuBar()->addMenu(createBookmarkMenu());
        menuBar()->addMenu(createHistoryMenu());
        menuBar()->addMenu(createViewMenu(toolbar));
        menuBar()->addMenu(createWindowMenu(m_tabWidget));
        menuBar()->addMenu(createHelpMenu());

        connect(toolbar, &QToolBar::visibilityChanged,
                this, &LibreBrowserWindow::toolBarVisibilityChanged);
    }

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    if (!forDevTools) {
        addToolBarBreak();

        m_progressBar->setMaximumHeight(1);
        m_progressBar->setTextVisible(false);
        m_progressBar->setStyleSheet(QStringLiteral("QProgressBar {border: 0px} QProgressBar::chunk {background-color: #da4453}"));

        layout->addWidget(m_progressBar);
    }

    layout->addWidget(m_tabWidget);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    connect(m_tabWidget, &TabWidget::titleChanged, this, &LibreBrowserWindow::handleWebViewTitleChanged);
    if (!forDevTools) {
        connect(m_tabWidget, &TabWidget::linkHovered, this, [this](const QString& url) {
            statusBar()->showMessage(url);
        });
        connect(m_tabWidget, &TabWidget::loadProgress, this, &LibreBrowserWindow::handleWebViewLoadProgress);
        connect(m_tabWidget, &TabWidget::webActionEnabledChanged, this, &LibreBrowserWindow::handleWebActionEnabledChanged);
        connect(m_tabWidget, &TabWidget::urlChanged, this, [this](const QUrl &url) {
            m_urlLineEdit->setText(url.toDisplayString());
        });
        connect(m_tabWidget, &TabWidget::favIconChanged, m_favAction, &QAction::setIcon);
        connect(m_tabWidget, &TabWidget::devToolsRequested, this, &LibreBrowserWindow::handleDevToolsRequested);
        connect(m_urlLineEdit, &QLineEdit::returnPressed, this, [this]() {
            m_tabWidget->setUrl(QUrl::fromUserInput(m_urlLineEdit->text()));
        });
        connect(m_tabWidget, &TabWidget::findTextFinished, this, &LibreBrowserWindow::handleFindTextFinished);

        QAction *focusUrlLineEditAction = new QAction(this);
        addAction(focusUrlLineEditAction);
        focusUrlLineEditAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
        connect(focusUrlLineEditAction, &QAction::triggered, this, [this] () {
            m_urlLineEdit->setFocus(Qt::ShortcutFocusReason);
        });
    }

    readSettings();
    handleWebViewTitleChanged(QString());
    m_tabWidget->createTab();
}

void LibreBrowserWindow::toolBarVisibilityChanged(bool visible)
{
    writeToolbarSettings(!visible);
}

QSize LibreBrowserWindow::sizeHint() const
{
    QRect desktopRect = QApplication::primaryScreen()->geometry();
    QSize size = desktopRect.size() * qreal(0.9);
    return size;
}

QMenu *LibreBrowserWindow::createFileMenu(TabWidget *tabWidget)
{
    QMenu *fileMenu = new QMenu(tr("&File"));

    QAction *newWinFileAction = new QAction(tr("&New Window"), this);
    newWinFileAction->setIcon(QIcon::fromTheme(QStringLiteral("window-new")));
    connect(newWinFileAction, &QAction::triggered, this, &LibreBrowserWindow::handleNewWindowTriggered);
    fileMenu->addAction(newWinFileAction);

    //fileMenu->addAction(tr("&New Window"), QKeySequence::New, this, &LibreBrowserWindow::handleNewWindowTriggered);

    QAction *incoFileAction = new QAction(tr("New &Incognito Window"), this);
    incoFileAction->setIcon(QIcon::fromTheme(QStringLiteral("im-invisible-user")));
    connect(incoFileAction, &QAction::triggered, this, &LibreBrowserWindow::handleNewIncognitoWindowTriggered);
    //fileMenu->addAction(tr("New &Incognito Window"), this, &LibreBrowserWindow::handleNewIncognitoWindowTriggered);
    fileMenu->addAction(incoFileAction);

    QAction *newTabAction = new QAction(tr("New &Tab"), this);
    newTabAction->setShortcuts(QKeySequence::AddTab);
    newTabAction->setIcon(QIcon::fromTheme(QStringLiteral("tab-new")));

    connect(newTabAction, &QAction::triggered, this, [this]() {
        m_tabWidget->createTab();
        m_urlLineEdit->setFocus();
    });
    fileMenu->addAction(newTabAction);

    QAction *openFileAction = new QAction(tr("&Open File..."), this);
    openFileAction->setShortcuts(QKeySequence::Open);
    openFileAction->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(openFileAction, &QAction::triggered, this, &LibreBrowserWindow::handleFileOpenTriggered);

    //fileMenu->addAction(tr("&Open File..."), QKeySequence::Open, this, &LibreBrowserWindow::handleFileOpenTriggered);
    fileMenu->addAction(openFileAction);

    fileMenu->addSeparator();

    QAction *closeTabAction = new QAction(tr("&Close Tab"), this);
    closeTabAction->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    closeTabAction->setShortcuts(QKeySequence::Close);
    connect(closeTabAction, &QAction::triggered, tabWidget, [tabWidget]() {
        tabWidget->closeTab(tabWidget->currentIndex());
    });
    fileMenu->addAction(closeTabAction);

    QAction *closeAction = new QAction(tr("&Quit"),this);
    closeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    closeAction->setIcon(QIcon::fromTheme(QStringLiteral("application-exit")));
    connect(closeAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(closeAction);

    connect(fileMenu, &QMenu::aboutToShow, this, [this, closeAction]() {
        if (m_browser->windows().count() == 1)
            closeAction->setText(tr("&Quit"));
        else
            closeAction->setText(tr("&Close Window"));
    });
    return fileMenu;
}

QMenu *LibreBrowserWindow::createEditMenu()
{
    QMenu *editMenu = new QMenu(tr("&Edit"));
    QAction *findAction = editMenu->addAction(tr("&Find"));
    findAction->setShortcuts(QKeySequence::Find);
    findAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
    connect(findAction, &QAction::triggered, this, &LibreBrowserWindow::handleFindActionTriggered);

    QAction *findNextAction = editMenu->addAction(tr("Find &Next"));
    findNextAction->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
    findNextAction->setShortcut(QKeySequence::FindNext);
    connect(findNextAction, &QAction::triggered, this, [this]() {
        if (!currentTab() || m_lastSearch.isEmpty())
            return;
        currentTab()->findText(m_lastSearch);
    });

    QAction *findPreviousAction = editMenu->addAction(tr("Find &Previous"));
    findPreviousAction->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
    findPreviousAction->setShortcut(QKeySequence::FindPrevious);
    connect(findPreviousAction, &QAction::triggered, this, [this]() {
        if (!currentTab() || m_lastSearch.isEmpty())
            return;
        currentTab()->findText(m_lastSearch, QWebEnginePage::FindBackward);
    });

    return editMenu;
}

QMenu *LibreBrowserWindow::createBookmarkMenu()
{
    QMenu *bookmarkMenu = new QMenu(tr("&Bookmark"));
    QAction *help = bookmarkMenu->addAction(tr("&LibreCAD Help"));

    bookmarkMenu->addSeparator();

    QAction *wiki = bookmarkMenu->addAction(tr("&Wiki"));
    QAction *user_man = bookmarkMenu->addAction(tr("User's &Manual"));
    QAction *cmd = bookmarkMenu->addAction(tr("&Commands"));
    QAction *sty = bookmarkMenu->addAction(tr("&Style Sheets"));
    QAction *wi = bookmarkMenu->addAction(tr("Wid&gets"));

    bookmarkMenu->addSeparator();

    QAction *devel = bookmarkMenu->addAction(tr("&Developer Help"));

    bookmarkMenu->addSeparator();

    QAction *fo = bookmarkMenu->addAction(tr("&Forum"));
    QAction *ch = bookmarkMenu->addAction(tr("Zulip &Chat"));

    connect(help, &QAction::triggered, this, [=](){
        currentTab()->setUrl(QUrl("qrc:/html/index.html"));
    });
    connect(wiki, &QAction::triggered, this, [=](){
        currentTab()->setUrl(QUrl("https://dokuwiki.librecad.org/"));
    });
    connect(user_man, &QAction::triggered, this, [=](){
        currentTab()->setUrl(QUrl("https://librecad.readthedocs.io/"));
    });
    connect(cmd, &QAction::triggered, this, [=](){
        currentTab()->setUrl(QUrl("https://librecad.readthedocs.io/en/latest/ref/tools.html"));
    });
    connect(sty, &QAction::triggered, this, [=](){
        currentTab()->setUrl(QUrl("https://librecad.readthedocs.io/en/latest/ref/customize.html#style-sheets"));
    });
    connect(wi, &QAction::triggered, this, [=](){
        currentTab()->setUrl(QUrl("https://librecad.readthedocs.io/en/latest/ref/menu.html#widgets"));
    });
    connect(devel, &QAction::triggered, this, [=](){
        currentTab()->setUrl(QUrl("qrc:/html/developer.html"));
    });
    connect(fo, &QAction::triggered, this, [=](){
        currentTab()->setUrl(QUrl("https://forum.librecad.org/"));
    });
    connect(ch, &QAction::triggered, this, [=](){
        currentTab()->setUrl(QUrl("https://librecad.zulipchat.com/"));
    });

    bookmarkMenu->addSeparator();

    QAction *addBookmarkAction = bookmarkMenu->addAction(tr("&add Bookmark"));
    addBookmarkAction->setShortcuts(QKeySequence::AddTab);
    addBookmarkAction->setIcon(QIcon::fromTheme(QStringLiteral("bookmark-new")));
    connect(addBookmarkAction, &QAction::triggered, this, &LibreBrowserWindow::addBookmarkTriggered);

    QAction *bookmarkManAction = new QAction(tr("Bookmark &Manager"), this);
    bookmarkManAction->setIcon(QIcon::fromTheme(QStringLiteral("application-menu")));
    bookmarkManAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_M));
    bookmarkMenu->addAction(bookmarkManAction);
    connect(bookmarkManAction, &QAction::triggered, this, &LibreBrowserWindow::removeBookmarkTriggered);

    bookmarkMenu->addSeparator();

    QAction* recentBookmarkAction = 0;
    for(unsigned int i = 0; i < m_maxBookmarkNr; ++i)
    {
        recentBookmarkAction = new QAction(this);
        recentBookmarkAction->setVisible(false);
        QObject::connect(recentBookmarkAction, &QAction::triggered,
                         this, &LibreBrowserWindow::openBookmark);
        m_recentBookmarkActionList.append(recentBookmarkAction);
        bookmarkMenu->addAction(recentBookmarkAction);
    }

    updateBookmarkActionList();

    return bookmarkMenu;
}

QMenu *LibreBrowserWindow::createHistoryMenu()
{
    QMenu *historyMenu = new QMenu(tr("&History"));
#if 0
    QAction *historyAction = new QAction(tr("History &Manager"), this);
    historyAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_M));
    historyMenu->addAction(historyAction);
    connect(historyAction, &QAction::triggered, this, &LibreBrowserWindow::historyTriggered);
    historyMenu->addSeparator();
#endif
    QAction* recentHistoryAction = 0;
    for(unsigned int i = 0; i < m_maxHistoryNr; ++i)
    {
        recentHistoryAction = new QAction(this);
        recentHistoryAction->setVisible(false);
        QObject::connect(recentHistoryAction, &QAction::triggered,
                         this, &LibreBrowserWindow::openBookmark);
        m_recentHistoryActionList.append(recentHistoryAction);
        historyMenu->addAction(recentHistoryAction);
    }

    updateHistoryActionList();

    return historyMenu;
}

QMenu *LibreBrowserWindow::createViewMenu(QToolBar *toolbar)
{
    QMenu *viewMenu = new QMenu(tr("&View"));

    m_stopAction = viewMenu->addAction(tr("&Stop"));
    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Period));
    shortcuts.append(Qt::Key_Escape);
    m_stopAction->setShortcuts(shortcuts);
    m_stopAction->setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));
    connect(m_stopAction, &QAction::triggered, this, [this]() {
        m_tabWidget->triggerWebPageAction(QWebEnginePage::Stop);
    });

    m_reloadAction = viewMenu->addAction(tr("&Reload Page"));
    m_reloadAction->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    m_reloadAction->setShortcuts(QKeySequence::Refresh);
    connect(m_reloadAction, &QAction::triggered, this, [this]() {
        m_tabWidget->triggerWebPageAction(QWebEnginePage::Reload);
    });

    QAction *zoomIn = viewMenu->addAction(tr("Zoom &In"));
    zoomIn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
    zoomIn->setIcon(QIcon::fromTheme(QStringLiteral("zoom-in")));
    connect(zoomIn, &QAction::triggered, this, [this]() {
        if (currentTab())
        {
            writeZoomFactorSettings(currentTab()->zoomFactor() + 0.1);
            currentTab()->setZoomFactor(currentTab()->zoomFactor() + 0.1);
        }
    });

    QAction *zoomOut = viewMenu->addAction(tr("Zoom &Out"));
    zoomOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
    zoomOut->setIcon(QIcon::fromTheme(QStringLiteral("zoom-out")));
    connect(zoomOut, &QAction::triggered, this, [this]() {
        if (currentTab())
        {
            writeZoomFactorSettings(currentTab()->zoomFactor() - 0.1);
            currentTab()->setZoomFactor(currentTab()->zoomFactor() - 0.1);
        }
    });

    QAction *resetZoom = viewMenu->addAction(tr("Reset &Zoom"));
    resetZoom->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    resetZoom->setIcon(QIcon::fromTheme(QStringLiteral("zoom-original")));
    connect(resetZoom, &QAction::triggered, this, [this]() {
        if (currentTab())
        {
            writeZoomFactorSettings(0.1);
            currentTab()->setZoomFactor(1.0);
        }
    });

    viewMenu->addSeparator();

    m_toggleFullscreen = new QAction(this);
    m_toggleFullscreen->setShortcut(QKeySequence(Qt::Key_F11));
    m_toggleFullscreen->setCheckable(true);
    m_toggleFullscreen->setChecked(false);
    m_toggleFullscreen->setText(tr("&Full Screen"));
    viewMenu->addAction(m_toggleFullscreen);

    connect(m_toggleFullscreen, &QAction::triggered, this, &LibreBrowserWindow::toggleFullscreen);

    viewMenu->addSeparator();

    QAction *viewToolbarAction = new QAction(tr("Hide Toolbar"),this);
    viewToolbarAction->setShortcut(tr("Ctrl+|"));
    viewToolbarAction->setText(tr("Toolbar"));
    viewToolbarAction->setCheckable(true);

    if(toolbarIsHidden())
    {
        toolbar->hide();
        viewToolbarAction->setChecked(false);
    }
    else
    {
        viewToolbarAction->setChecked(true);
    }

    connect(viewToolbarAction, &QAction::triggered, toolbar, [toolbar,viewToolbarAction]()
    {
        if (toolbar->isVisible())
        {
            viewToolbarAction->setChecked(false);
            toolbar->close();
        } else
        {
            viewToolbarAction->setChecked(true);
            toolbar->show();
        }

    });
    viewMenu->addAction(viewToolbarAction);

    QAction *viewStatusbarAction = new QAction(tr("Hide Status Bar"), this);
    viewStatusbarAction->setShortcut(tr("Ctrl+/"));
    viewStatusbarAction->setCheckable(true);
    viewStatusbarAction->setText(tr("Status Bar"));

    if(statusbarIsHidden())
    {
        statusBar()->hide();
        viewStatusbarAction->setChecked(false);
    }
    else
    {
        viewStatusbarAction->setChecked(true);
    }

    connect(viewStatusbarAction, &QAction::triggered, this, [this, viewStatusbarAction]()
    {
        if (statusBar()->isVisible()) {
            viewStatusbarAction->setChecked(false);
            statusBar()->close();
        } else {
            viewStatusbarAction->setChecked(true);
            statusBar()->show();
        }
    });
    viewMenu->addAction(viewStatusbarAction);
    return viewMenu;
}

QMenu *LibreBrowserWindow::createWindowMenu(TabWidget *tabWidget)
{
    QMenu *menu = new QMenu(tr("&Window"));

    QAction *nextTabAction = new QAction(tr("Show Next Tab"), this);
    nextTabAction->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BraceRight));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_PageDown));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Less));
    nextTabAction->setShortcuts(shortcuts);
    connect(nextTabAction, &QAction::triggered, tabWidget, &TabWidget::nextTab);

    QAction *previousTabAction = new QAction(tr("Show Previous Tab"), this);
    previousTabAction->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
    shortcuts.clear();
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BraceLeft));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_PageUp));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Greater));
    previousTabAction->setShortcuts(shortcuts);
    connect(previousTabAction, &QAction::triggered, tabWidget, &TabWidget::previousTab);

    connect(menu, &QMenu::aboutToShow, this, [this, menu, nextTabAction, previousTabAction]() {
        menu->clear();
        menu->addAction(nextTabAction);
        menu->addAction(previousTabAction);
        menu->addSeparator();

        QList<LibreBrowserWindow*> windows = m_browser->windows();
        int index(-1);
        for (auto window : windows) {
            QAction *action = menu->addAction(window->windowTitle(), this, &LibreBrowserWindow::handleShowWindowTriggered);
            action->setData(++index);
            action->setCheckable(true);
            if (window == this)
                action->setChecked(true);
        }
    });
    return menu;
}

QMenu *LibreBrowserWindow::createHelpMenu()
{
    QMenu *helpMenu = new QMenu(tr("&Help"));

    QAction *aboutAction = new QAction(tr("About &LibreBrowser"), this);
    aboutAction->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));
    connect(aboutAction, &QAction::triggered, qApp, QApplication::aboutQt);
    helpMenu->addAction(aboutAction);

    //helpMenu->addAction(tr("About &Qt"), qApp, QApplication::aboutQt);
    return helpMenu;
}

QToolBar *LibreBrowserWindow::createToolBar()
{
    QToolBar *navigationBar = new QToolBar(tr("Navigation"));
    navigationBar->setMovable(false);
    navigationBar->toggleViewAction()->setEnabled(false);

    m_historyBackAction = new QAction(this);
    QList<QKeySequence> backShortcuts = QKeySequence::keyBindings(QKeySequence::Back);
    for (auto it = backShortcuts.begin(); it != backShortcuts.end();) {
        // Chromium already handles navigate on backspace when appropriate.
        if ((*it)[0].key() == Qt::Key_Backspace)
        {
            it = backShortcuts.erase(it);
        }
        else
            ++it;
    }
    // For some reason Qt doesn't bind the dedicated Back key to Back.
    backShortcuts.append(QKeySequence(Qt::Key_Back));
    m_historyBackAction->setShortcuts(backShortcuts);
    m_historyBackAction->setIconVisibleInMenu(false);
    m_historyBackAction->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
    m_historyBackAction->setToolTip(tr("Go back in history"));
    connect(m_historyBackAction, &QAction::triggered, this, [this]() {
        m_tabWidget->triggerWebPageAction(QWebEnginePage::Back);
    });
    navigationBar->addAction(m_historyBackAction);

    m_historyForwardAction = new QAction(this);
    QList<QKeySequence> fwdShortcuts = QKeySequence::keyBindings(QKeySequence::Forward);
    for (auto it = fwdShortcuts.begin(); it != fwdShortcuts.end();) {
        if (((*it)[0].key() & Qt::Key_unknown) == Qt::Key_Backspace)
            it = fwdShortcuts.erase(it);
        else
            ++it;
    }
    fwdShortcuts.append(QKeySequence(Qt::Key_Forward));
    m_historyForwardAction->setShortcuts(fwdShortcuts);
    m_historyForwardAction->setIconVisibleInMenu(false);
    m_historyForwardAction->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
    m_historyForwardAction->setToolTip(tr("Go forward in history"));
    connect(m_historyForwardAction, &QAction::triggered, this, [this]() {
        m_tabWidget->triggerWebPageAction(QWebEnginePage::Forward);
    });
    navigationBar->addAction(m_historyForwardAction);

    m_stopReloadAction = new QAction(this);
    connect(m_stopReloadAction, &QAction::triggered, this, [this]() {
        m_tabWidget->triggerWebPageAction(QWebEnginePage::WebAction(m_stopReloadAction->data().toInt()));
    });
    navigationBar->addAction(m_stopReloadAction);

    m_urlLineEdit = new QLineEdit(this);
    m_favAction = new QAction(this);
    m_urlLineEdit->addAction(m_favAction, QLineEdit::LeadingPosition);
    m_urlLineEdit->setClearButtonEnabled(true);
    navigationBar->addWidget(m_urlLineEdit);

    auto downloadsAction = new QAction(this);
    downloadsAction->setIcon(QIcon::fromTheme(QStringLiteral("go-bottom")));
    downloadsAction->setToolTip(tr("Show downloads"));
    navigationBar->addAction(downloadsAction);
    connect(downloadsAction, &QAction::triggered, this, [this]() {
        m_browser->downloadManagerWidget().show();
    });

    auto favoriteAction = new QAction(this);
    favoriteAction->setIcon(QIcon::fromTheme(QStringLiteral("emblem-favorite")));
    favoriteAction->setToolTip(tr("Add Bookmark"));
    navigationBar->addAction(favoriteAction);
    connect(favoriteAction, &QAction::triggered, this, &LibreBrowserWindow::addBookmarkTriggered);

    return navigationBar;
}

void LibreBrowserWindow::toggleFullscreen()
{
    if (windowState() == Qt::WindowFullScreen)
    {
        m_toggleFullscreen->setChecked(false);
        setWindowState(Qt::WindowActive);
    }
    else
    {
        m_toggleFullscreen->setChecked(true);
        setWindowState(Qt::WindowFullScreen);
    }
}

void LibreBrowserWindow::handleWebActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled)
{
    switch (action) {
    case QWebEnginePage::Back:
        m_historyBackAction->setEnabled(enabled);
        break;
    case QWebEnginePage::Forward:
        m_historyForwardAction->setEnabled(enabled);
        break;
    case QWebEnginePage::Reload:
        m_reloadAction->setEnabled(enabled);
        break;
    case QWebEnginePage::Stop:
        m_stopAction->setEnabled(enabled);
        break;
    default:
        qWarning("Unhandled webActionChanged signal");
    }
}

void LibreBrowserWindow::handleWebViewTitleChanged(const QString &title)
{
    m_currentUrlTitle = title;
    QDateTime date = QDateTime::currentDateTime();

    QString suffix = m_profile->isOffTheRecord()
        ? tr("LibreBrowser (Incognito)")
        : tr("LibreBrowser");

    if (title.startsWith('q'))
    {
        QFileInfo fi(title);
        QString fileName = fi.fileName();
        fileName.chop(5);
        m_currentUrlTitle = fileName;
        if(currentTab())
        {
            const QString history = date.toString("dd.MM. hh:mm") + " | " + m_currentUrlTitle + "\t" + currentTab()->url().toEncoded();
            adjustForCurrentHistory(history);
        }
        setWindowTitle(fileName + " - " + tr("LibreCAD Help"));
        return;
    }

    if(currentTab() && !title.isEmpty())
    {
        const QString history = date.toString("dd.MM. hh:mm") + " | " + m_currentUrlTitle + "\t" + currentTab()->url().toEncoded();
        adjustForCurrentHistory(history);
    }

    if (title.isEmpty())
        setWindowTitle(suffix);
    else
        setWindowTitle(title + " - " + suffix);

    if (currentTab())
        currentTab()->setZoomFactor(zoomFactor());
}

void LibreBrowserWindow::handleNewWindowTriggered()
{
    LibreBrowserWindow *window = m_browser->createWindow();
    window->m_urlLineEdit->setFocus();
}

void LibreBrowserWindow::handleNewIncognitoWindowTriggered()
{
    LibreBrowserWindow *window = m_browser->createWindow(/* offTheRecord: */ true);
    window->m_urlLineEdit->setFocus();
}

void LibreBrowserWindow::handleFileOpenTriggered()
{
    QUrl url = QFileDialog::getOpenFileUrl(this, tr("Open Web Resource"), QString(),
                                                tr("Web Resources (*.html *.htm *.svg *.png *.gif *.svgz);;All files (*.*)"));
    if (url.isEmpty())
        return;
    currentTab()->setUrl(url);
}

void LibreBrowserWindow::handleFindActionTriggered()
{
    if (!currentTab())
        return;
    bool ok = false;
    QString search = QInputDialog::getText(this, tr("Find"),
                                           tr("Find:"), QLineEdit::Normal,
                                           m_lastSearch, &ok);
    if (ok && !search.isEmpty()) {
        m_lastSearch = search;
        currentTab()->findText(m_lastSearch);
    }
}

void LibreBrowserWindow::addBookmarkTriggered()
{
    if (!currentTab())
    {
        return;
    }
    const QString bookmark = m_currentUrlTitle + "\t" + currentTab()->url().toEncoded();
    adjustForCurrentBookmark(bookmark);
}

void LibreBrowserWindow::removeBookmarkTriggered()
{
    Bookmark *dlg = new Bookmark(this);

    if (dlg->exec() == QDialog::Accepted)
    {
        updateBookmarkActionList();
    }
}
#if 0
void LibreBrowserWindow::historyTriggered()
{

}
#endif
void LibreBrowserWindow::closeEvent(QCloseEvent *event)
{
    if (m_tabWidget->count() > 1) {
        int ret = QMessageBox::warning(this, tr("Confirm close"),
                                       tr("Are you sure you want to close the window ?\n"
                                          "There are %1 tabs open.").arg(m_tabWidget->count()),
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (ret == QMessageBox::No) {
            event->ignore();
            return;
        }
    }
    writeSettings();
    event->accept();
    deleteLater();
}

TabWidget *LibreBrowserWindow::tabWidget() const
{
    return m_tabWidget;
}

WebView *LibreBrowserWindow::currentTab() const
{
    return m_tabWidget->currentWebView();
}

void LibreBrowserWindow::handleWebViewLoadProgress(int progress)
{
    static QIcon stopIcon(QIcon::fromTheme(QStringLiteral("critical")));
    static QIcon reloadIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));

    if (0 < progress && progress < 100) {
        m_stopReloadAction->setData(QWebEnginePage::Stop);
        m_stopReloadAction->setIcon(stopIcon);
        m_stopReloadAction->setToolTip(tr("Stop loading the current page"));
        m_progressBar->setValue(progress);
    } else {
        m_stopReloadAction->setData(QWebEnginePage::Reload);
        m_stopReloadAction->setIcon(reloadIcon);
        m_stopReloadAction->setToolTip(tr("Reload the current page"));
        m_progressBar->setValue(0);
    }
}

void LibreBrowserWindow::handleShowWindowTriggered()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int offset = action->data().toInt();
        QList<LibreBrowserWindow*> windows = m_browser->windows();
        windows.at(offset)->activateWindow();
        windows.at(offset)->currentTab()->setFocus();
    }
}

void LibreBrowserWindow::handleDevToolsRequested(QWebEnginePage *source)
{
    source->setDevToolsPage(m_browser->createDevToolsWindow()->currentTab()->page());
    source->triggerAction(QWebEnginePage::InspectElement);
}

void LibreBrowserWindow::handleFindTextFinished(const QWebEngineFindTextResult &result)
{
    if (result.numberOfMatches() == 0) {
        statusBar()->showMessage(tr("\"%1\" not found.").arg(m_lastSearch));
    } else {
        statusBar()->showMessage(tr("\"%1\" found: %2/%3").arg(m_lastSearch,
                                                               QString::number(result.activeMatch()),
                                                               QString::number(result.numberOfMatches())));
    }
}

void LibreBrowserWindow::openBookmark(bool checked)
{
    Q_UNUSED(checked)
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        currentTab()->setUrl(QUrl(action->data().toString()));
    }
}

void LibreBrowserWindow::writeSettings()
{
    QSettings settings("LibreBrowser", "LibreBrowser");
    settings.beginGroup("BrowserWindow");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void LibreBrowserWindow::writeToolbarSettings(bool isHidden)
{
    QSettings settings("LibreBrowser", "LibreBrowser");
    settings.beginGroup("BrowserWindow");
    settings.setValue("toolbar", isHidden);
    settings.endGroup();
}

void LibreBrowserWindow::writeStatusbarSettings(bool isHidden)
{
    QSettings settings("LibreBrowser", "LibreBrowser");
    settings.beginGroup("BrowserWindow");
    settings.setValue("statusbar", isHidden);
    settings.endGroup();
}

void LibreBrowserWindow::writeZoomFactorSettings(double factor)
{
    QSettings settings("LibreBrowser", "LibreBrowser");
    settings.beginGroup("BrowserWindow");
    settings.setValue("zoomfactor", factor);
    settings.endGroup();
}

bool LibreBrowserWindow::toolbarIsHidden()
{
    QSettings settings("LibreBrowser", "LibreBrowser");
    settings.beginGroup("BrowserWindow");
    const auto isHidden = settings.value("toolbar").toBool();
    if (settings.contains("toolbar"))
    {
        return isHidden;
    }
    settings.endGroup();
    return false;
}

bool LibreBrowserWindow::statusbarIsHidden()
{
    QSettings settings("LibreBrowser", "LibreBrowser");
    settings.beginGroup("BrowserWindow");
    const auto isHidden = settings.value("statusbar").toBool();
    if (settings.contains("statusbar"))
    {
        return isHidden;
    }
    settings.endGroup();
    return false;
}

double LibreBrowserWindow::zoomFactor()
{
    QSettings settings("LibreBrowser", "LibreBrowser");
    settings.beginGroup("BrowserWindow");
    const auto zoomfactor = settings.value("zoomfactor").toDouble();
    if (settings.contains("zoomfactor"))
    {
        return zoomfactor;
    }
    settings.endGroup();
    return false;
}

void LibreBrowserWindow::readSettings()
{
    QSettings settings("LibreBrowser", "LibreBrowser");
    settings.beginGroup("BrowserWindow");
    const auto geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty())
        setGeometry(320, 280, 1280, 720);
    else
        restoreGeometry(geometry);
    settings.endGroup();
}

void LibreBrowserWindow::updateBookmarkActionList()
{
    QSettings settings("LibreBrowser", "LibreBrowser");
    settings.beginGroup("Bookmark");
    QStringList recentBookmark = settings.value("Bookmarks").toStringList();
    settings.endGroup();

    auto itEnd = 0u;
    if(recentBookmark.size() <= m_maxBookmarkNr)
        itEnd = recentBookmark.size();
    else
        itEnd = m_maxBookmarkNr;

    for (auto i = 0u; i < itEnd; ++i) {
        QStringList list = recentBookmark.at(i).split( "\t" );
        QString strippedName = list.value(0);
        const QString url = list.value(1);

        if(strippedName == "")
        {
            strippedName = QUrl(url).host();
        }
        m_recentBookmarkActionList.at(i)->setText(strippedName);
        m_recentBookmarkActionList.at(i)->setData(url);
        m_recentBookmarkActionList.at(i)->setIcon(QIcon::fromTheme(QStringLiteral("text-html")));
        m_recentBookmarkActionList.at(i)->setVisible(true);
    }

    for (auto i = itEnd; i < m_maxBookmarkNr; ++i)
        m_recentBookmarkActionList.at(i)->setVisible(false);
}

void LibreBrowserWindow::adjustForCurrentBookmark(const QString &bookmark)
{
    QSettings settings("LibreBrowser", "LibreBrowser");
    settings.beginGroup("Bookmark");
    QStringList recentBookmark = settings.value("Bookmarks").toStringList();
    recentBookmark.removeAll(bookmark);
    recentBookmark.prepend(bookmark);
    while (recentBookmark.size() > m_maxBookmarkNr)
        recentBookmark.removeLast();
    settings.setValue("Bookmarks", recentBookmark);
    settings.endGroup();

   updateBookmarkActionList();
}

void LibreBrowserWindow::updateHistoryActionList()
{
    QSettings settings("LibreBrowser", "LibreBrowser");
    settings.beginGroup("History");
    QStringList recentHistory = settings.value("urls").toStringList();
    settings.endGroup();

    auto itEnd = 0u;
    if(recentHistory.size() <= m_maxHistoryNr)
        itEnd = recentHistory.size();
    else
        itEnd = m_maxHistoryNr;

    for (auto i = 0u; i < itEnd; ++i) {
        QStringList list = recentHistory.at(i).split( "\t" );
        QString strippedName = list.value(0);
        const QString url = list.value(1);

        if(strippedName == "")
        {
            strippedName = QUrl(url).host();
        }
        m_recentHistoryActionList.at(i)->setText(strippedName);
        m_recentHistoryActionList.at(i)->setData(url);
        m_recentHistoryActionList.at(i)->setIcon(QIcon::fromTheme(QStringLiteral("text-html")));
        m_recentHistoryActionList.at(i)->setVisible(true);
    }

    for (auto i = itEnd; i < m_maxHistoryNr; ++i)
        m_recentHistoryActionList.at(i)->setVisible(false);
}

void LibreBrowserWindow::adjustForCurrentHistory(const QString &history)
{
    if ((history.indexOf("about:blank") > -1) ||
        (history.indexOf("error") > -1))
    {
        return;
    }

    QSettings settings("LibreBrowser", "LibreBrowser");
    settings.beginGroup("History");
    QStringList recentHistory = settings.value("urls").toStringList();

    QStringList list = history.split( "\t" );
    const QString url = list.value(1);

    for (auto i = 0u; i < recentHistory.size(); ++i)
    {
        if (recentHistory.at(i).split( "\t" ).value(1) == url)
        {
            recentHistory.remove(i);
        }
    }

    recentHistory.prepend(history);

    while (recentHistory.size() > m_maxHistoryNr)
        recentHistory.removeLast();

    settings.setValue("urls", recentHistory);
    settings.endGroup();

    updateHistoryActionList();
}
