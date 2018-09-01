/***************************************************************************
 *   Copyright (C) 2016 by Arnav Dhamija <arnav.dhamija@gmail.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "kio_recentfolders.h"

#include <QDebug>
#include <QFileInfo>
#include <QCoreApplication>

#include <KUser>
#include <KFileItem>
#include <KLocalizedString>

#include <baloo/indexerconfig.h>
#include <baloo/query.h>

static const QString HomeDir = KUser().homeDir();
static const int HomeDirLength = HomeDir.length();

RecentFolders::RecentFolders(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("recentfolders", pool, app)
{
}

RecentFolders::~RecentFolders()
{
}

QString RecentFolders::toPretty(const QString& path)
{
    if (path.startsWith(HomeDir))
    {
        return QStringLiteral("~") + path.mid(HomeDirLength);
    }
    return path;
}

const qint64 BackDays = 7;  // TODO: read from config
const uint MaxResults = 25; // TODO: read from config

void RecentFolders::listDir(const QUrl& url)
{
    if (url.toString() != "recentfolders:/")
    {
        error(KIO::ERR_DOES_NOT_EXIST, url.toString());
        return;
    }

    Baloo::IndexerConfig indexerConfig;
    if (!indexerConfig.fileIndexingEnabled())
    {
        error(KIO::ERR_NO_CONTENT, i18n("File indexing disabled, nothing to do :("));
        return ;
    }

    Baloo::Query query = Baloo::Query::fromSearchUrl(HomeDir);
    query.setType(QStringLiteral("Folder"));

    uint count = 0;
    QDateTime since = QDateTime::currentDateTime().addDays(-1 * BackDays);
    Baloo::ResultIterator it = query.exec();
    while (it.next() && count < MaxResults)
    {
        QString filePath = it.filePath();
        if (filePath == HomeDir)
        {
            continue;
        }

        QFileInfo fileInfo(filePath);
        if (fileInfo.lastModified() > since)
        {
            KIO::UDSEntry uds;
            uds.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
            uds.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
            uds.fastInsert(KIO::UDSEntry::UDS_NAME, filePath);
            uds.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, toPretty(filePath));
            uds.fastInsert(KIO::UDSEntry::UDS_URL, QUrl::fromLocalFile(filePath).toString());
            listEntry(uds);
            count++;
        }
    }

    finished();
}

void RecentFolders::mimetype(const QUrl&)
{
    mimeType(QStringLiteral("inode/directory"));
    finished();
}

static const QDateTime DateTime = QDateTime::currentDateTime();

void RecentFolders::stat(const QUrl&)
{
    KIO::UDSEntry uds;
    uds.fastInsert(KIO::UDSEntry::UDS_NAME, i18n("Recent Folders"));
    uds.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, i18n("Recent Folders"));
    uds.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    uds.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
    uds.fastInsert(KIO::UDSEntry::UDS_MODIFICATION_TIME, DateTime.toTime_t());
    uds.fastInsert(KIO::UDSEntry::UDS_CREATION_TIME, DateTime.toTime_t());
    uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, 0700);
    uds.fastInsert(KIO::UDSEntry::UDS_USER, KUser().loginName());

    statEntry(uds);
    finished();
}

extern "C"
{
    int Q_DECL_EXPORT kdemain(int argc, char **argv)
    {
        QCoreApplication app(argc, argv);
        app.setApplicationName(QStringLiteral("kio_recentfolders"));
        RecentFolders slave(argv[2], argv[3]);
        slave.dispatchLoop();
        return 0;
    }
}
