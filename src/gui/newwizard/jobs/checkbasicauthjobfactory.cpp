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

#include "checkbasicauthjobfactory.h"

#include "accessmanager.h"
#include "common/utility.h"
#include "creds/httpcredentials.h"
#include "gui/tlserrordialog.h"
#include "gui/updateurldialog.h"
#include "theme.h"

#include <QNetworkReply>

namespace OCC::Wizard::Jobs {

CoreJob *CheckBasicAuthJobFactory::startJob(const QUrl &url)
{
    auto *job = new CoreJob;

    QNetworkRequest req(Utility::concatUrlPath(url, Theme::instance()->webDavPath()));

    req.setAttribute(QNetworkRequest::AuthenticationReuseAttribute, QNetworkRequest::Manual);

    QString authorizationHeader = QStringLiteral("Basic %1").arg(QString::fromLocal8Bit(QStringLiteral("%1:%2").arg(_username, _password).toLocal8Bit().toBase64()));
    req.setRawHeader("Authorization", authorizationHeader.toLocal8Bit());

    auto *reply = nam()->sendCustomRequest(req, "PROPFIND");

    connect(reply, &QNetworkReply::finished, job, [reply, job]() {
        switch (reply->error()) {
        case QNetworkReply::NoError:
            setJobResult(job, true);
            return;
        case QNetworkReply::AuthenticationRequiredError:
            if (OC_ENSURE(reply->rawHeader(QByteArrayLiteral("WWW-Authenticate")).toLower().contains("basic "))) {
                setJobResult(job, false);
                return;
            }
            Q_FALLTHROUGH();
        default:
            setJobError(job, tr("Invalid reply received from server"), reply);
            return;
        }
    });

    makeRequest();

    return job;
}

CheckBasicAuthJobFactory::CheckBasicAuthJobFactory(QNetworkAccessManager *nam, const QString &username, const QString &password, QObject *parent)
    : AbstractCoreJobFactory(nam, parent)
    , _username(username)
    , _password(password)
{
}

}
