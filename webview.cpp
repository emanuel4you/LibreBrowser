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
#include "webpage.h"
#include "webpopupwindow.h"
#include "webview.h"
#include "ui_certificateerrordialog.h"
#include "ui_passworddialog.h"
#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QAuthenticator>
#include <QWebEngineSettings>
#include <QTimer>
#include <QStyle>
#include <QWidget>
#include <QTabWidget>

WebView::WebView(QWidget *parent)
    : QWebEngineView(parent)
    , m_loadProgress(100)
    , m_parent(parent)
{
    connect(this, &QWebEngineView::loadStarted, [this]() {
        m_loadProgress = 0;
        emit favIconChanged(favIcon());
    });
    connect(this, &QWebEngineView::loadProgress, [this](int progress) {
        m_loadProgress = progress;
    });
    connect(this, &QWebEngineView::loadFinished, [this](bool success) {
        m_loadProgress = success ? 100 : -1;
        emit favIconChanged(favIcon());
    });
    connect(this, &QWebEngineView::iconChanged, [this](const QIcon &) {
        emit favIconChanged(favIcon());
    });

    connect(this, &QWebEngineView::renderProcessTerminated,
            [this](QWebEnginePage::RenderProcessTerminationStatus termStatus, int statusCode) {
        QString status;
        switch (termStatus) {
        case QWebEnginePage::NormalTerminationStatus:
            status = tr("Render process normal exit");
            break;
        case QWebEnginePage::AbnormalTerminationStatus:
            status = tr("Render process abnormal exit");
            break;
        case QWebEnginePage::CrashedTerminationStatus:
            status = tr("Render process crashed");
            break;
        case QWebEnginePage::KilledTerminationStatus:
            status = tr("Render process killed");
            break;
        }
        QMessageBox::StandardButton btn = QMessageBox::question(window(), status,
                                                   tr("Render process exited with code: %1\n"
                                                      "Do you want to reload the page ?").arg(statusCode));
        if (btn == QMessageBox::Yes)
            QTimer::singleShot(0, this, [this] { reload(); });
    });
}

inline QString questionForFeature(QWebEnginePage::Feature feature)
{
    switch (feature) {
    case QWebEnginePage::Geolocation:
        return QObject::tr("Allow %1 to access your location information?");
    case QWebEnginePage::MediaAudioCapture:
        return QObject::tr("Allow %1 to access your microphone?");
    case QWebEnginePage::MediaVideoCapture:
        return QObject::tr("Allow %1 to access your webcam?");
    case QWebEnginePage::MediaAudioVideoCapture:
        return QObject::tr("Allow %1 to access your microphone and webcam?");
    case QWebEnginePage::MouseLock:
        return QObject::tr("Allow %1 to lock your mouse cursor?");
    case QWebEnginePage::DesktopVideoCapture:
        return QObject::tr("Allow %1 to capture video of your desktop?");
    case QWebEnginePage::DesktopAudioVideoCapture:
        return QObject::tr("Allow %1 to capture audio and video of your desktop?");
    case QWebEnginePage::Notifications:
        return QObject::tr("Allow %1 to show notification on your desktop?");
    }
    return QString();
}

void WebView::setPage(WebPage *page)
{
    if (auto oldPage = qobject_cast<WebPage *>(QWebEngineView::page())) {
        disconnect(oldPage, &WebPage::createCertificateErrorDialog, this,
                   &WebView::handleCertificateError);
        disconnect(oldPage, &QWebEnginePage::authenticationRequired, this,
                   &WebView::handleAuthenticationRequired);
        disconnect(oldPage, &QWebEnginePage::featurePermissionRequested, this,
                   &WebView::handleFeaturePermissionRequested);
        disconnect(oldPage, &QWebEnginePage::proxyAuthenticationRequired, this,
                   &WebView::handleProxyAuthenticationRequired);
        disconnect(oldPage, &QWebEnginePage::registerProtocolHandlerRequested, this,
                   &WebView::handleRegisterProtocolHandlerRequested);
    }
    createWebActionTrigger(page,QWebEnginePage::Forward);
    createWebActionTrigger(page,QWebEnginePage::Back);
    createWebActionTrigger(page,QWebEnginePage::Reload);
    createWebActionTrigger(page,QWebEnginePage::Stop);
    QWebEngineView::setPage(page);
    connect(page, &WebPage::createCertificateErrorDialog, this, &WebView::handleCertificateError);
    connect(page, &QWebEnginePage::authenticationRequired, this,
            &WebView::handleAuthenticationRequired);
    connect(page, &QWebEnginePage::featurePermissionRequested, this,
            &WebView::handleFeaturePermissionRequested);
    connect(page, &QWebEnginePage::proxyAuthenticationRequired, this,
            &WebView::handleProxyAuthenticationRequired);
    connect(page, &QWebEnginePage::registerProtocolHandlerRequested, this,
            &WebView::handleRegisterProtocolHandlerRequested);

    auto set = settings();
    set->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    connect(page, &QWebEnginePage::fullScreenRequested, this, &WebView::acceptFullScreenRequest);
}

void WebView::acceptFullScreenRequest(QWebEngineFullScreenRequest request)
{
    if(request.toggleOn())
    {
        QTabWidget * tabWidget = qobject_cast<QTabWidget*>(m_parent);

        m_currentTabText = tabWidget->tabText(tabWidget->currentIndex());
        request.accept();
        QWidget* window = (QWidget*) parent();
        setLayout(window->layout());
        layout()->removeWidget(this);
        setParent(0);
        showFullScreen();
    }
    else
    {
        request.accept();
        QTabWidget * tabWidget = qobject_cast<QTabWidget*>(m_parent);
        tabWidget->addTab(this, m_currentTabText);
    }
}

int WebView::loadProgress() const
{
    return m_loadProgress;
}

void WebView::createWebActionTrigger(QWebEnginePage *page, QWebEnginePage::WebAction webAction)
{
    QAction *action = page->action(webAction);
    connect(action, &QAction::changed, this, [this, action, webAction]{
        emit webActionEnabledChanged(webAction, action->isEnabled());
    });
}

bool WebView::isWebActionEnabled(QWebEnginePage::WebAction webAction) const
{
    return page()->action(webAction)->isEnabled();
}

QIcon WebView::favIcon() const
{
    QIcon favIcon = icon();
    if (!favIcon.isNull())
        return favIcon;

    if (m_loadProgress < 0) {
        static QIcon errorIcon(QIcon::fromTheme(QStringLiteral("dialog-error")));
        return errorIcon;
    } else if (m_loadProgress < 100) {
        static QIcon loadingIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
        return loadingIcon;
    } else {
        static QIcon defaultIcon(QIcon::fromTheme(QStringLiteral("text-html")));
        return defaultIcon;
    }
}

QWebEngineView *WebView::createWindow(QWebEnginePage::WebWindowType type)
{
    LibreBrowserWindow *mainWindow = qobject_cast<LibreBrowserWindow*>(window());
    if (!mainWindow)
        return nullptr;

    switch (type) {
    case QWebEnginePage::WebBrowserTab: {
        return mainWindow->tabWidget()->createTab();
    }
    case QWebEnginePage::WebBrowserBackgroundTab: {
        return mainWindow->tabWidget()->createBackgroundTab();
    }
    case QWebEnginePage::WebBrowserWindow: {
        return mainWindow->browser()->createWindow()->currentTab();
    }
    case QWebEnginePage::WebDialog: {
        WebPopupWindow *popup = new WebPopupWindow(page()->profile());
        connect(popup->view(), &WebView::devToolsRequested, this, &WebView::devToolsRequested);
        return popup->view();
    }
    }
    return nullptr;
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    const QList<QAction *> actions = menu->actions();
    auto inspectElement = std::find(actions.cbegin(), actions.cend(), page()->action(QWebEnginePage::InspectElement));
    if (inspectElement == actions.cend()) {
        auto viewSource = std::find(actions.cbegin(), actions.cend(), page()->action(QWebEnginePage::ViewSource));
        if (viewSource == actions.cend())
            menu->addSeparator();

        QAction *action = new QAction(menu);
        action->setText("Open inspector in new window");
        connect(action, &QAction::triggered, this, [this]() { emit devToolsRequested(page()); });

        QAction *before(inspectElement == actions.cend() ? nullptr : *inspectElement);
        menu->insertAction(before, action);
    } else {
        (*inspectElement)->setText(tr("Inspect element"));
    }
    menu->popup(event->globalPos());
}

void WebView::handleCertificateError(QWebEngineCertificateError error)
{
    QDialog dialog(window());
    dialog.setModal(true);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    Ui::CertificateErrorDialog certificateDialog;
    certificateDialog.setupUi(&dialog);
    certificateDialog.m_iconLabel->setText(QString());
    QIcon icon(window()->style()->standardIcon(QStyle::SP_MessageBoxWarning, 0, window()));
    certificateDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));
    certificateDialog.m_errorLabel->setText(error.description());
    dialog.setWindowTitle(tr("Certificate Error"));

    if (dialog.exec() == QDialog::Accepted)
        error.acceptCertificate();
    else
        error.rejectCertificate();
}

void WebView::handleAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth)
{
    QDialog dialog(window());
    dialog.setModal(true);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.m_iconLabel->setText(QString());
    QIcon icon(window()->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, window()));
    passwordDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));

    QString introMessage(tr("Enter username and password for \"%1\" at %2")
                            .arg(auth->realm(),requestUrl.toString().toHtmlEscaped()));
    passwordDialog.m_infoLabel->setText(introMessage);
    passwordDialog.m_infoLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(passwordDialog.m_userNameLineEdit->text());
        auth->setPassword(passwordDialog.m_passwordLineEdit->text());
    } else {
        // Set authenticator null if dialog is cancelled
        *auth = QAuthenticator();
    }
}

void WebView::handleFeaturePermissionRequested(const QUrl &securityOrigin,
                                               QWebEnginePage::Feature feature)
{
    QString title = tr("Permission Request");
    QString question = questionForFeature(feature).arg(securityOrigin.host());
    if (!question.isEmpty() && QMessageBox::question(window(), title, question) == QMessageBox::Yes)
        page()->setFeaturePermission(securityOrigin, feature,
                                     QWebEnginePage::PermissionGrantedByUser);
    else
        page()->setFeaturePermission(securityOrigin, feature,
                                     QWebEnginePage::PermissionDeniedByUser);
}

void WebView::handleProxyAuthenticationRequired(const QUrl &, QAuthenticator *auth,
                                                const QString &proxyHost)
{
    QDialog dialog(window());
    dialog.setModal(true);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.m_iconLabel->setText(QString());
    QIcon icon(window()->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, window()));
    passwordDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));

    QString introMessage = tr("Connect to proxy \"%1\" using:");
    introMessage = introMessage.arg(proxyHost.toHtmlEscaped());
    passwordDialog.m_infoLabel->setText(introMessage);
    passwordDialog.m_infoLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(passwordDialog.m_userNameLineEdit->text());
        auth->setPassword(passwordDialog.m_passwordLineEdit->text());
    } else {
        // Set authenticator null if dialog is cancelled
        *auth = QAuthenticator();
    }
}

//! [registerProtocolHandlerRequested]
void WebView::handleRegisterProtocolHandlerRequested(
        QWebEngineRegisterProtocolHandlerRequest request)
{
    auto answer = QMessageBox::question(window(), tr("Permission Request"),
                                        tr("Allow %1 to open all %2 links?")
                                            .arg(request.origin().host(),request.scheme()));
    if (answer == QMessageBox::Yes)
        request.accept();
    else
        request.reject();
}
//! [registerProtocolHandlerRequested]
