#include <Nepomuk/ResourceManager>
#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/FileQuery>
#include <Nepomuk/Query/ComparisonTerm>
#include <Nepomuk/Query/LiteralTerm>
#include <Nepomuk/Vocabulary/NIE>
#include <KDebug>
#include <KDirModel>
#include <KDirLister>
#include <KFileItem>
#include <KUrl>

#include "NepomukTimelineViewer.h"

NepomukTimelineViewer::NepomukTimelineViewer(uint days)
{
    this->days = days;
}

NepomukTimelineViewer::NepomukTimelineViewer()
{
    this->days = DEFAULT_TIMELINE_DAYS;
}

KFileItemList NepomukTimelineViewer::getTimeline()
{
    if(Nepomuk::ResourceManager::instance()->init() != 0)
    {
        kError() << "Please make sure that your Nepomuk Service is available." << endl;
        return KFileItemList();
    }

    Nepomuk::Query::ComparisonTerm mtime = Nepomuk::Vocabulary::NIE::lastModified() > Nepomuk::Query::LiteralTerm(QDateTime::currentDateTime().addDays(-1 * this->days));
    Nepomuk::Query::Query query(mtime);
    Nepomuk::Query::FileQuery fileQuery = query.toFileQuery();

    KDirLister dirLister;
    dirLister.openUrl(fileQuery.toSearchUrl());
    QEventLoop loop;
    QObject::connect(&dirLister, SIGNAL(completed()), &loop, SLOT(quit()));
    loop.exec();

    return dirLister.items();
}

QList<QString> NepomukTimelineViewer::getTimelineFilesURL()
{
    QList<QString> result;
    KFileItemList timeline = getTimeline();
    foreach(KFileItem file, timeline)
    {
        result << KUrl(file.entry().stringValue(KIO::UDSEntry::UDS_URL)).prettyUrl();
    }

    return result;
}
