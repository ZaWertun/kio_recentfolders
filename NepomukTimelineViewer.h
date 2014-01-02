#include <KFileItem>

const uint DEFAULT_TIMELINE_DAYS = 10;

class NepomukTimelineViewer
{
private:
    uint days;//look backward how many days

public:
    NepomukTimelineViewer(uint days);
    NepomukTimelineViewer();
    KFileItemList getTimeline();
    QList<QString> getTimelineFilesURL();
};
