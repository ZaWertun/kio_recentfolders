#include "kio_recentfolders.h"

#include <QDir>
#include <QPair>
#include <QDebug>
#include <QFileInfo>
#include <QCoreApplication>

#include <KConfig>
#include <KFileItem>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>

#include <baloo/indexerconfig.h>
#include <baloo/query.h>

#define COMPACT_LEVEL   2

#define CONFIG_BACK_DAYS           QStringLiteral("BackDays")
#define CONFIG_BACK_DAYS_DEFAULT   7

static const QString HomeDir = KUser().homeDir();
static const int HomeDirLength = HomeDir.length();

RecentFolders::RecentFolders(const QByteArray &pool, const QByteArray &app)
    : SlaveBase("recentfolders", pool, app), backDays(CONFIG_BACK_DAYS_DEFAULT)
{
    loadConfig();
}

RecentFolders::~RecentFolders()
{
}

void RecentFolders::loadConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("kio_recentfolders"));
    KConfigGroup group = config->group(QStringLiteral("General"));

    if (group.hasKey(CONFIG_BACK_DAYS)) {
        backDays = group.readEntry<uint>(CONFIG_BACK_DAYS, backDays);
    } else {
        group.writeEntry(CONFIG_BACK_DAYS, backDays);
    }
}

QString RecentFolders::toPretty(const QString& path)
{
    if (path.startsWith(HomeDir)) {
        return QStringLiteral("~") + path.mid(HomeDirLength);
    }
    return path;
}

KIO::UDSEntry RecentFolders::getUdsEntry(const QString& path)
{
    KIO::UDSEntry uds;
    uds.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    uds.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
    uds.fastInsert(KIO::UDSEntry::UDS_NAME, path);
    uds.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, toPretty(path));
    uds.fastInsert(KIO::UDSEntry::UDS_URL, QUrl::fromLocalFile(path).toString());
    return uds;
}

QStringList RecentFolders::compactDirs(const uint level, const QStringList& dirs)
{
    QMap<QString, uint> map;
    foreach (QString dir, dirs) {
        map.insert(dir, 1);
    }

    foreach (QString dir, dirs) {
        QFileInfo fileInfo(dir);
        for (uint i = 0; i < level; ++i) {
            QString parent = fileInfo.dir().path();
            if (map.contains(parent)) {
                if (map.contains(dir) && map[dir] == 1) {
                    map[dir] = 0;
                }
                map[parent] += 1;
            }
        }
    }

    QStringList result;
    QMapIterator<QString, uint> it(map);
    while (it.hasNext()) {
        it.next();
        if (it.value() > 0) {
            result << it.key();
        }
    }

    return result;
}

QStringList RecentFolders::queryDate(const QString& root, const QDate& date)
{
    QStringList result;
    Baloo::Query query = Baloo::Query::fromSearchUrl(root);
    query.setType(QStringLiteral("Folder"));
    query.setDateFilter(date.year(), date.month(), date.day());
    query.setSortingOption(Baloo::Query::SortNone);

    Baloo::ResultIterator resultIterator = query.exec();
    while (resultIterator.next()) {
        QString dir = resultIterator.filePath();
        if (dir != HomeDir && QFileInfo(dir).exists()) {
            result << dir;
        }
    }

    return result;
}

void RecentFolders::listDir(const QUrl& url)
{
    if (url.toString() != "recentfolders:/") {
        error(KIO::ERR_DOES_NOT_EXIST, url.toString());
        return;
    }

    Baloo::IndexerConfig indexerConfig;
    if (!indexerConfig.fileIndexingEnabled()) {
        error(KIO::ERR_NO_CONTENT, i18n("File indexing disabled, nothing to do :("));
        return ;
    }

    KIO::UDSEntry dot;
    dot.fastInsert(KIO::UDSEntry::UDS_NAME, QStringLiteral("."));
    listEntry(dot);

    QSet<QString> added;
    QDate date = QDate::currentDate();
    QDate minDate = date.addDays(-1 * (qint64)backDays);
    while (date > minDate) {
        QStringList dirs = queryDate(HomeDir, date);
        QStringList compacted = compactDirs(COMPACT_LEVEL, dirs);
        foreach (QString dir, compacted) {
            if (added.contains(dir))
                continue;

            listEntry(getUdsEntry(dir));
            added << dir;
        }
        date = date.addDays(-1);
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
    uds.fastInsert(KIO::UDSEntry::UDS_NAME, i18n("Recent Places"));
    uds.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, i18n("Recent Places"));
    uds.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    uds.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
    uds.fastInsert(KIO::UDSEntry::UDS_MODIFICATION_TIME, DateTime.toTime_t());
    uds.fastInsert(KIO::UDSEntry::UDS_CREATION_TIME, DateTime.toTime_t());
    uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, 0500);
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
