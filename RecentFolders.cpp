#include <stdlib.h>
#include <iostream>
#include <KIO/ForwardingSlaveBase>
#include <KGlobal>
#include <KIO/Job>
#include <KIO/NetAccess>
#include <KDebug>
#include <KUrl>
#include <KComponentData>
#include <KUser>
#include <KConfig>
#include <KConfigGroup>
#include <QCoreApplication>
#include <QStringList>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QSet>
#include <QString>

#include "BalooQueryRecentFolders.h"

#define CONFIG_MAX_ITEMS                "MaxItems"
#define CONFIG_TIMELINE_BACKWARD_DAYS   "TimelineBackwardDays"


struct Configuration
{
    uint maxItems;
    uint timelineBackwardDays;

    Configuration()
    {
        maxItems = 15;
        timelineBackwardDays = 7;
    }
};

class RecentFolders: public KIO::ForwardingSlaveBase
{
private:
    Configuration config;
    void loadConfig();

protected:
    void listDir(const KUrl& url);
    void mimetype(const KUrl& url);
    void stat(const KUrl& url);
    virtual bool rewriteUrl(const KUrl& url, KUrl& newUrl);
    void del (const KUrl &url, bool isfile);
    void prepareUDSEntry(KIO::UDSEntry& entry, bool listing=false) const;

public:
    RecentFolders(const QByteArray& pool, const QByteArray &app);
};

inline KUrl getFileTrueUrlfromUrl(const KUrl& url)
{
    QString urlString = url.prettyUrl();
    QString encodedUrl = urlString.section('/', 1);
    if (encodedUrl == "")
    {
        return KUrl();
    }

    return KUrl(encodedUrl);
}

void RecentFolders::loadConfig()
{
    KConfig kconfig;
    KConfigGroup generalGroup(&kconfig, "General");

    if (generalGroup.hasKey(CONFIG_MAX_ITEMS))
    {
        config.maxItems = generalGroup.readEntry<uint>(CONFIG_MAX_ITEMS, config.maxItems);
    }
    else
    {
        generalGroup.writeEntry(CONFIG_MAX_ITEMS, config.maxItems);
    }

    if (generalGroup.hasKey(CONFIG_TIMELINE_BACKWARD_DAYS))
    {
        config.timelineBackwardDays = generalGroup.readEntry<uint>(CONFIG_TIMELINE_BACKWARD_DAYS, config.timelineBackwardDays);
    }
    else
    {
        generalGroup.writeEntry(CONFIG_TIMELINE_BACKWARD_DAYS, config.timelineBackwardDays);
    }
}

static KUser user = KUser();

static QString getShortPath(QString path)
{
    QString homeDir = user.homeDir();

    if (path.startsWith(homeDir))
    {
        return "~" + path.mid(homeDir.length());
    }

    return path;
}

bool isRootUrl(const KUrl& url)
{
    const QString path = url.path(KUrl::RemoveTrailingSlash);
    return (!url.hasQuery() && (path.isEmpty() || path == "/"))
           || url.prettyUrl() == "recentfolders:/";
}

void RecentFolders::prepareUDSEntry(KIO::UDSEntry& entry, bool listing) const
{
    ForwardingSlaveBase::prepareUDSEntry(entry, listing);
}

void RecentFolders::del (const KUrl &url, bool isFile)
{
    ForwardingSlaveBase::del(getFileTrueUrlfromUrl(url), isFile);
}

void RecentFolders::listDir(const KUrl& url)
{
    if (!isRootUrl(url))
    {   
        error(KIO::ERR_DOES_NOT_EXIST, url.prettyUrl());
        return;
    }
     
    //flush
    listEntry(KIO::UDSEntry(), true);
    KIO::UDSEntryList udslist;

    BalooQueryRecentFolders queryRecentFolders(QDateTime::currentDateTime().addDays(-1 * config.timelineBackwardDays), config.maxItems);
    KFileItemList list = queryRecentFolders.execute();
    for (KFileItemList::iterator it = list.begin(); it < list.end(); ++it)
    {
        KFileItem file = *it;        
        KIO::UDSEntry entry;
       
        entry.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, getShortPath(file.localPath()));
        
        KUrl fileUrl(file.localPath());
        entry.insert(KIO::UDSEntry::UDS_NAME, fileUrl.path());
        entry.insert(KIO::UDSEntry::UDS_URL, fileUrl.path());
        
        entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        entry.insert(KIO::UDSEntry::UDS_MIME_TYPE, "inode/directory");
        
        udslist << entry;
    }

    listEntries(udslist);
    listEntry(KIO::UDSEntry(), true);
    finished();
}

void RecentFolders::mimetype(const KUrl & url)
{
    if (isRootUrl(url))
    {
        mimeType(QString::fromLatin1("inode/directory"));
        finished();
    }
    else
    {
        KUrl newUrl;
        this->rewriteUrl(url, newUrl);
        ForwardingSlaveBase::mimetype(newUrl);
    }
}

void RecentFolders::stat(const KUrl& url)
{
    if (isRootUrl(url))
    {
        KIO::UDSEntry uds;
        uds.insert(KIO::UDSEntry::UDS_NAME, i18n("Recent Document"));
        uds.insert(KIO::UDSEntry::UDS_DISPLAY_NAME, i18n("Recent Document"));
        uds.insert(KIO::UDSEntry::UDS_DISPLAY_TYPE, i18n("Recent Document"));
        uds.insert(KIO::UDSEntry::UDS_ICON_NAME, QString::fromLatin1("document-open-recent"));
        uds.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
        uds.insert(KIO::UDSEntry::UDS_MIME_TYPE, QString::fromLatin1("inode/directory"));
        statEntry(uds);
        finished();
    }
    else
    {
        //get the true Url and forward the Stat
        KUrl newUrl;
        this->rewriteUrl(url, newUrl);
        ForwardingSlaveBase::stat(newUrl);
    }
}

RecentFolders::RecentFolders(const QByteArray& pool, const QByteArray &app):KIO::ForwardingSlaveBase("recentfolders", pool, app)
{
    loadConfig();
}

bool RecentFolders::rewriteUrl(const KUrl& url, KUrl & newUrl)
{
    if (isRootUrl(url))
    {
        return false;
    }

    newUrl = getFileTrueUrlfromUrl(url);
    return !newUrl.isEmpty();
}

extern "C" int KDE_EXPORT kdemain(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    KComponentData instance("kio_recentfolders", "kio_recentfolders");
    KGlobal::locale();

    if (argc != 4)
    {
        fprintf( stderr, "Usage: kio_recentfolders protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    RecentFolders slave(argv[2], argv[3]);
    slave.dispatchLoop();
    return 0;
}

