#include <KFileItem>

class NepomukQueryRecentFolders
{
private:
    QDateTime sinceDate;
    uint itemsLimit;

public:
    NepomukQueryRecentFolders(QDateTime sinceDate, uint itemsLimit);
    KFileItemList getTimeline();
};
