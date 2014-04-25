#include <baloo/indexerconfig.h>
#include <baloo/query.h>
#include <QDebug>
#include <KDebug>
#include <QDir>
#include <sys/stat.h>

#include "BalooQueryRecentFolders.h"

typedef QList<Baloo::SearchStore*> SearchStoreList;

BalooQueryRecentFolders::BalooQueryRecentFolders(QDateTime sinceDate, uint itemsLimit)
{
    this->sinceDate = KDateTime(sinceDate);
    this->itemsLimit = itemsLimit;
}

BalooQueryRecentFolders::~BalooQueryRecentFolders()
{
}

KFileItemList BalooQueryRecentFolders::execute()
{
    Baloo::IndexerConfig indexerConfig;
    if (!indexerConfig.balooEnabled())
    {
        kError() << "Please make sure that Baloo Service is available." << endl;
        return KFileItemList();
    }
    
    KFileItemList result;
    
    Baloo::Query query = Baloo::Query::fromSearchUrl(QDir::homePath());   
    query.addType("Folder");
    
    uint i = 0;
    Baloo::ResultIterator it = query.exec();
    KUrl home = KUrl(QDir::homePath());
    while (it.next() && i < itemsLimit)
    {
        if (it.url() == home)
        {
            continue;
        }
     
        KFileItem fileItem(it.url(), "inode/directory", S_IFDIR);
        if (fileItem.time(KFileItem::ModificationTime) > sinceDate)
        {
            result << fileItem;
            i++;
        }
    }
       
    return result;
}
