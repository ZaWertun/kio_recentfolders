#ifndef KIO_RECENT_FOLDERS_H
#define KIO_RECENT_FOLDERS_H

#include <KUser>
#include <kfileitem.h>
#include <kio/slavebase.h>

class RecentFolders : public KIO::SlaveBase
{
public:
    RecentFolders(const QByteArray &pool, const QByteArray &app);
    ~RecentFolders() override;

    void listDir(const QUrl&) override;
    void mimetype(const QUrl&) override;
    void stat(const QUrl&) override;

private:
    uint backDays;

    void loadConfig();
    QString toPretty(const QString& path);
    KIO::UDSEntry getUdsEntry(const QString& path);
    QStringList queryDate(const QString& root, const QDate& date);
    QStringList compactDirs(const uint level, const QStringList& dirs);
};

#endif // KIO_RECENT_FOLDERS_H
