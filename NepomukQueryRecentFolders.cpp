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

#include "NepomukQueryRecentFolders.h"

NepomukQueryRecentFolders::NepomukQueryRecentFolders(QDateTime sinceDate, uint itemsLimit)
{
    this->sinceDate = sinceDate;
    this->itemsLimit = itemsLimit;
}

KFileItemList NepomukQueryRecentFolders::getTimeline()
{
    if(Nepomuk::ResourceManager::instance()->init() != 0)
    {
        kError() << "Please make sure that your Nepomuk Service is available." << endl;
        return KFileItemList();
    }

    Nepomuk::Query::ComparisonTerm mtime = Nepomuk::Vocabulary::NIE::lastModified() > Nepomuk::Query::LiteralTerm(sinceDate);
    Nepomuk::Query::Query query(mtime);
    
    Nepomuk::Query::FileQuery fileQuery = query.toFileQuery();
    fileQuery.setFileMode(Nepomuk::Query::FileQuery::QueryFolders);
    fileQuery.setLimit(itemsLimit);

    KDirLister dirLister;
    dirLister.openUrl(fileQuery.toSearchUrl());
    QEventLoop loop;
    QObject::connect(&dirLister, SIGNAL(completed()), &loop, SLOT(quit()));
    loop.exec();

    return dirLister.items();
}
