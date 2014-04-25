#ifndef _BALOO_SEARCH_H
#define _BALOO_SEARCH_H

#include <KFileItem>

class BalooQueryRecentFolders : public QObject
{
public:
    explicit BalooQueryRecentFolders(QDateTime, uint);
    virtual ~BalooQueryRecentFolders();
    
    KFileItemList execute();

private:
    KDateTime sinceDate;
    uint itemsLimit;
};

#endif // _BALOO_SEARCH_H
