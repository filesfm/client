/*
 * Copyright (C) Fabian Müller <fmueller@owncloud.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "webfingersetupwizardstate.h"
#include "creds/webfinger.h"
#include "determineauthtypejobfactory.h"
#include "pages/webfingersetupwizardpage.h"

namespace OCC::Wizard {

WebFingerSetupWizardState::WebFingerSetupWizardState(SetupWizardContext *context)
    : AbstractSetupWizardState(context)
{
    _page = new WebFingerSetupWizardPage(_context->accountBuilder().webFingerServerUrl());
}

void WebFingerSetupWizardState::evaluatePage()
{
    auto *webFingerSetupWizardPage = qobject_cast<WebFingerSetupWizardPage *>(_page);
    OC_ASSERT(webFingerSetupWizardPage != nullptr);

    const QString resource = QStringLiteral("acct:%1").arg(webFingerSetupWizardPage->username());

    auto webFinger = new WebFinger(_context->accessManager(), this);

    connect(webFinger, &WebFinger::finished, this, [this, webFingerSetupWizardPage, webFinger]() {
        if (webFinger->error().error != QJsonParseError::NoError) {
            Q_EMIT evaluationFailed(tr("Failed to parse WebFinger response: %1").arg(webFinger->error().errorString()));
            return;
        }

        if (webFinger->href().isEmpty()) {
            Q_EMIT evaluationFailed(tr("WebFinger endpoint did not send href attribute"));
            return;
        }

        qDebug() << "WebFinger server sent href" << webFinger->href();

        // next, we need to find out which kind of authentication page we have to present to the user
        auto authTypeJob = DetermineAuthTypeJobFactory(_context->accessManager(), this).startJob(webFinger->href());

        connect(authTypeJob, &CoreJob::finished, authTypeJob, [this, authTypeJob, webFingerSetupWizardPage, webFinger]() {
            authTypeJob->deleteLater();

            if (authTypeJob->result().isNull()) {
                Q_EMIT evaluationFailed(authTypeJob->errorMessage());
                return;
            }

            _context->accountBuilder().setWebFingerUsername(webFingerSetupWizardPage->username());
            _context->accountBuilder().setServerUrl(webFinger->href(), qvariant_cast<DetermineAuthTypeJob::AuthType>(authTypeJob->result()));
            Q_EMIT evaluationSuccessful();
        });
    });

    webFinger->start(_context->accountBuilder().webFingerServerUrl(), resource);
}

SetupWizardState WebFingerSetupWizardState::state() const
{
    return SetupWizardState::WebFingerState;
}

} // OCC::Wizard
